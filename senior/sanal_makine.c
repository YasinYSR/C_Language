/* Yığın tabanlı küçük bir sanal makine (bytecode yorumlayıcı).
   Komutlar tek bayt, bazılarının ardından operand gelir. Makine kendini korur: yığın taşması/eksilmesi,
   sıfıra bölme, geçersiz komut, kod dışına atlama ve sonsuz döngü (adım sınırı) hata koduyla durdurulur.
   Tamsayılar 32 bit işaretli; taşma tanımsız davranış yerine sarmalanır.
   Derle: gcc sanal_makine.c -o vm */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum Komut {
    K_DURDUR,  /* HALT */
    K_YUKLE,   /* PUSH i32: sabit değeri yığına koy */
    K_AT,      /* POP */
    K_KOPYA,   /* DUP */
    K_TOPLA, K_CIKAR, K_CARP, K_BOL, K_MOD,
    K_KUCUK,   /* LT: a < b ise 1, değilse 0 */
    K_ESIT,    /* EQ */
    K_ATLA,    /* JMP u32: koşulsuz atla */
    K_SIFIRSA, /* JZ u32: tepedeki 0 ise atla (değeri tüketir) */
    K_OKU,     /* LOAD u8: yerel değişken -> yığın */
    K_YAZ,     /* STORE u8: yığın -> yerel değişken */
    K_YAZDIR   /* PRINT: tepedeki değeri çıktıya ekle, yığından çıkar */
};

enum Sonuc {
    S_TAMAM, S_YIGIN_TASMA, S_YIGIN_BOS, S_SIFIRA_BOLME, S_GECERSIZ_KOMUT,
    S_GECERSIZ_ATLAMA, S_KOD_BITTI, S_ADIM_SINIRI, S_GECERSIZ_DEGISKEN, S_CIKTI_DOLU
};

#define YIGIN_BOYUT 64
#define DEGISKEN_SAYISI 16
#define CIKTI_BOYUT 64
#define ADIM_SINIRI 1000000

typedef struct {
    int32_t yigin[YIGIN_BOYUT];
    int32_t degisken[DEGISKEN_SAYISI];
    int32_t cikti[CIKTI_BOYUT];
    size_t cikti_sayisi;
    long adim;
} VM;

static int32_t sarmala(uint32_t x)
{
    return (int32_t)x; /* uygulama tanımlı ama pratikte iki'nin tümleyeni; işaretli taşmadan (UB) iyi */
}

static int okuyabilir(size_t pc, size_t n, size_t gerekli)
{
    return pc + gerekli <= n;
}

enum Sonuc vm_calistir(VM *vm, const uint8_t *kod, size_t n)
{
    size_t pc = 0, sp = 0;
    memset(vm, 0, sizeof *vm);

#define IT(x) do { if (sp >= YIGIN_BOYUT) return S_YIGIN_TASMA; vm->yigin[sp++] = (x); } while (0)
#define CEK(x) do { if (sp == 0) return S_YIGIN_BOS; (x) = vm->yigin[--sp]; } while (0)

    for (;;) {
        if (pc >= n)
            return S_KOD_BITTI; /* DURDUR olmadan kod bitti */
        if (++vm->adim > ADIM_SINIRI)
            return S_ADIM_SINIRI;

        int32_t a, b;
        switch (kod[pc++]) {
        case K_DURDUR:
            return S_TAMAM;
        case K_YUKLE: {
            if (!okuyabilir(pc, n, 4))
                return S_KOD_BITTI;
            uint32_t u = (uint32_t)kod[pc] | (uint32_t)kod[pc + 1] << 8 | (uint32_t)kod[pc + 2] << 16 | (uint32_t)kod[pc + 3] << 24;
            pc += 4;
            IT(sarmala(u));
            break;
        }
        case K_AT:
            CEK(a);
            break;
        case K_KOPYA:
            CEK(a);
            IT(a);
            IT(a);
            break;
        case K_TOPLA: CEK(b); CEK(a); IT(sarmala((uint32_t)a + (uint32_t)b)); break;
        case K_CIKAR: CEK(b); CEK(a); IT(sarmala((uint32_t)a - (uint32_t)b)); break;
        case K_CARP:  CEK(b); CEK(a); IT(sarmala((uint32_t)a * (uint32_t)b)); break;
        case K_BOL:
            CEK(b);
            CEK(a);
            if (b == 0)
                return S_SIFIRA_BOLME;
            IT(b == -1 ? sarmala(0u - (uint32_t)a) : a / b); /* INT_MIN / -1 taşar */
            break;
        case K_MOD:
            CEK(b);
            CEK(a);
            if (b == 0)
                return S_SIFIRA_BOLME;
            IT(b == -1 ? 0 : a % b);
            break;
        case K_KUCUK: CEK(b); CEK(a); IT(a < b); break;
        case K_ESIT:  CEK(b); CEK(a); IT(a == b); break;
        case K_ATLA:
        case K_SIFIRSA: {
            int kosullu = kod[pc - 1] == K_SIFIRSA;
            if (!okuyabilir(pc, n, 4))
                return S_KOD_BITTI;
            uint32_t hedef = (uint32_t)kod[pc] | (uint32_t)kod[pc + 1] << 8 | (uint32_t)kod[pc + 2] << 16 | (uint32_t)kod[pc + 3] << 24;
            pc += 4;
            if (kosullu)
                CEK(a);
            if (!kosullu || a == 0) {
                if (hedef >= n)
                    return S_GECERSIZ_ATLAMA;
                pc = hedef;
            }
            break;
        }
        case K_OKU:
        case K_YAZ: {
            int oku = kod[pc - 1] == K_OKU;
            if (!okuyabilir(pc, n, 1))
                return S_KOD_BITTI;
            uint8_t i = kod[pc++];
            if (i >= DEGISKEN_SAYISI)
                return S_GECERSIZ_DEGISKEN;
            if (oku)
                IT(vm->degisken[i]);
            else
                CEK(vm->degisken[i]);
            break;
        }
        case K_YAZDIR:
            CEK(a);
            if (vm->cikti_sayisi >= CIKTI_BOYUT)
                return S_CIKTI_DOLU;
            vm->cikti[vm->cikti_sayisi++] = a;
            break;
        default:
            return S_GECERSIZ_KOMUT;
        }
    }
#undef IT
#undef CEK
}

const char *sonuc_metni(enum Sonuc s)
{
    static const char *const metin[] = {
        "tamam", "yığın taştı", "yığın boş", "sıfıra bölme", "geçersiz komut",
        "geçersiz atlama", "kod beklenmedik şekilde bitti", "adım sınırı aşıldı (sonsuz döngü?)",
        "geçersiz değişken numarası", "çıktı dolu"};
    return metin[s];
}

/* --- program kurmak için küçük yardımcılar (derleyici/assembler yerine) --- */
typedef struct {
    uint8_t kod[256];
    size_t n;
} Program;

static size_t ekle_komut(Program *p, enum Komut k)
{
    p->kod[p->n++] = (uint8_t)k;
    return p->n;
}

static void ekle_u32(Program *p, uint32_t u)
{
    for (int i = 0; i < 4; i++)
        p->kod[p->n++] = (uint8_t)(u >> (8 * i));
}

static void yukle(Program *p, int32_t v)
{
    ekle_komut(p, K_YUKLE);
    ekle_u32(p, (uint32_t)v);
}

static size_t atla(Program *p, enum Komut k, uint32_t hedef) /* yama için operand konumunu döndürür */
{
    ekle_komut(p, k);
    size_t konum = p->n;
    ekle_u32(p, hedef);
    return konum;
}

static void yama(Program *p, size_t konum, uint32_t hedef)
{
    for (int i = 0; i < 4; i++)
        p->kod[konum + (size_t)i] = (uint8_t)(hedef >> (8 * i));
}

static void degisken(Program *p, enum Komut k, uint8_t no)
{
    ekle_komut(p, k);
    p->kod[p->n++] = no;
}

/* n! hesaplayan program: while (n != 0) { acc *= n; n--; } print acc */
static void faktoriyel_programi(Program *p, int32_t n)
{
    p->n = 0;
    yukle(p, n);
    degisken(p, K_YAZ, 0); /* değişken 0: n */
    yukle(p, 1);
    degisken(p, K_YAZ, 1); /* değişken 1: acc */
    size_t dongu = p->n;
    degisken(p, K_OKU, 0);
    size_t cikis = atla(p, K_SIFIRSA, 0);
    degisken(p, K_OKU, 1);
    degisken(p, K_OKU, 0);
    ekle_komut(p, K_CARP);
    degisken(p, K_YAZ, 1);
    degisken(p, K_OKU, 0);
    yukle(p, 1);
    ekle_komut(p, K_CIKAR);
    degisken(p, K_YAZ, 0);
    atla(p, K_ATLA, (uint32_t)dongu);
    yama(p, cikis, (uint32_t)p->n);
    degisken(p, K_OKU, 1);
    ekle_komut(p, K_YAZDIR);
    ekle_komut(p, K_DURDUR);
}

#ifndef TEST
int main(void)
{
    Program p;
    VM vm;
    faktoriyel_programi(&p, 10);
    enum Sonuc s = vm_calistir(&vm, p.kod, p.n);
    printf("program %zu bayt, sonuç: %s, %ld adımda\n", p.n, sonuc_metni(s), vm.adim);
    for (size_t i = 0; i < vm.cikti_sayisi; i++)
        printf("10! = %d\n", vm.cikti[i]);
    return 0;
}
#endif
