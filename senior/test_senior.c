#define TEST
#include "bellek_havuzu.c"
#include "hash_tablosu.c"
#include "ifade_ayristirici.c"
#include "sanal_makine.c"

static int hata_sayisi;
#define KONTROL(k) do { if (!(k)) { printf("BASARISIZ %s:%d  %s\n", __FILE__, __LINE__, #k); hata_sayisi++; } } while (0)

static double hesap(const char *s)
{
    double v = NAN;
    ifade_hesapla(s, &v, NULL);
    return v;
}

static int hatali(const char *s)
{
    double v;
    const char *h = NULL;
    return ifade_hesapla(s, &v, &h) == -1 && h != NULL;
}

static enum Sonuc kos(VM *vm, Program *p)
{
    return vm_calistir(vm, p->kod, p->n);
}

int main(void)
{
    /* --- hash tablosu --- */
    Tablo *t = tablo_yeni();
    char anahtar[32];
    int v;
    for (int i = 0; i < 20000; i++) { /* birçok kez büyütmeyi tetikler */
        snprintf(anahtar, sizeof anahtar, "anahtar-%d", i);
        KONTROL(tablo_koy(t, anahtar, i * 3) == 0);
    }
    KONTROL(t->boyut == 20000 && t->kova_sayisi >= 32768);
    int tamam = 1;
    for (int i = 0; i < 20000; i++) {
        snprintf(anahtar, sizeof anahtar, "anahtar-%d", i);
        if (!tablo_al(t, anahtar, &v) || v != i * 3)
            tamam = 0;
    }
    KONTROL(tamam);
    KONTROL(!tablo_al(t, "yok", &v));
    tablo_koy(t, "anahtar-5", -1); /* güncelleme boyutu artırmaz */
    KONTROL(t->boyut == 20000 && tablo_al(t, "anahtar-5", &v) && v == -1);
    for (int i = 0; i < 20000; i += 2) {
        snprintf(anahtar, sizeof anahtar, "anahtar-%d", i);
        if (!tablo_sil(t, anahtar))
            tamam = 0;
    }
    KONTROL(tamam && t->boyut == 10000);
    KONTROL(!tablo_sil(t, "anahtar-0") && !tablo_al(t, "anahtar-0", &v) && tablo_al(t, "anahtar-1", &v) && v == 3);
    tablo_koy(t, "", 7); /* boş dize de geçerli anahtar */
    KONTROL(tablo_al(t, "", &v) && v == 7);
    tablo_yok(t);

    /* --- ifade ayrıştırıcı --- */
    KONTROL(hesap("2 + 3 * 4") == 14);
    KONTROL(hesap("(2 + 3) * 4") == 20);
    KONTROL(hesap("10 - 3 - 2") == 5);   /* soldan birleşim */
    KONTROL(hesap("2 ^ 3 ^ 2") == 512);  /* sağdan birleşim */
    KONTROL(hesap("-2 ^ 2") == -4);
    KONTROL(hesap("2 ^ -1") == 0.5);
    KONTROL(hesap("--3") == 3 && hesap("+ 4") == 4);
    KONTROL(hesap("7 % 4") == 3 && hesap("1.5 * 2") == 3 && hesap(".5 + .5") == 1);
    KONTROL(hesap("sqrt(16) + abs(-3)") == 7);
    KONTROL(fabs(hesap("sin(0) + cos(0)") - 1) < 1e-12);
    KONTROL(hatali("") && hatali("1 +") && hatali("(1") && hatali("1 2") && hatali("1 / 0"));
    KONTROL(hatali("5 % 0") && hatali("sqrt(-1)") && hatali("foo(1)") && hatali("abs 3") && hatali("1 $ 2"));
    char derin[2048];
    memset(derin, '(', 1000);
    derin[1000] = '1';
    memset(derin + 1001, ')', 1000);
    derin[2001] = '\0'; /* 1000 iç içe parantez: çökmeden hata vermeli */
    KONTROL(hatali(derin));
    memset(derin, '-', 1500);
    derin[1500] = '1';
    derin[1501] = '\0';
    KONTROL(hatali(derin));

    /* --- bellek havuzu --- */
    Havuz h = havuz_yeni(128);
    KONTROL(h.son != NULL);
    int hizali = 1;
    size_t hizalar[] = {1, 2, 4, 8, 16, 32, 64};
    for (int i = 0; i < 200; i++) {
        size_t hz = hizalar[i % 7];
        unsigned char *p = havuz_al(&h, (size_t)(i % 37) + 1, hz);
        if (!p || (uintptr_t)p % hz != 0)
            hizali = 0;
        else
            memset(p, 0xAB, (size_t)(i % 37) + 1); /* taşma varsa UBSan/çökme yakalar */
    }
    KONTROL(hizali);
    int *a = havuz_al(&h, sizeof(int) * 10, _Alignof(int));
    for (int i = 0; i < 10; i++)
        a[i] = i;
    for (int i = 0; i < 50; i++) /* blok değişse de eski işaretçi geçerli kalır */
        havuz_al(&h, 100, 8);
    KONTROL(a[9] == 9);
    void *buyuk = havuz_al(&h, 100000, 16); /* blok boyutundan büyük istek */
    KONTROL(buyuk && (uintptr_t)buyuk % 16 == 0);
    memset(buyuk, 1, 100000);
    KONTROL(havuz_al(&h, 8, 3) == NULL && havuz_al(&h, 8, 0) == NULL); /* geçersiz hiza */
    KONTROL(havuz_al(&h, SIZE_MAX, 8) == NULL && havuz_al(&h, SIZE_MAX - 4, 16) == NULL);
    havuz_sifirla(&h);
    void *ilk = havuz_al(&h, 16, 16);
    havuz_sifirla(&h);
    KONTROL(havuz_al(&h, 16, 16) == ilk); /* sıfırlama sonrası aynı yer yeniden kullanılır */
    havuz_yok(&h);
    KONTROL(h.son == NULL && havuz_al(&h, 8, 8) == NULL);

    /* --- sanal makine --- */
    Program p;
    VM vm;
    faktoriyel_programi(&p, 10);
    KONTROL(kos(&vm, &p) == S_TAMAM && vm.cikti_sayisi == 1 && vm.cikti[0] == 3628800);
    faktoriyel_programi(&p, 0);
    KONTROL(kos(&vm, &p) == S_TAMAM && vm.cikti[0] == 1);

    p.n = 0; /* (7 - 2) * 3 = 15 ve 7 < 2 = 0 */
    yukle(&p, 7); yukle(&p, 2); ekle_komut(&p, K_CIKAR); yukle(&p, 3); ekle_komut(&p, K_CARP); ekle_komut(&p, K_YAZDIR);
    yukle(&p, 7); yukle(&p, 2); ekle_komut(&p, K_KUCUK); ekle_komut(&p, K_YAZDIR);
    ekle_komut(&p, K_DURDUR);
    KONTROL(kos(&vm, &p) == S_TAMAM && vm.cikti[0] == 15 && vm.cikti[1] == 0);

    p.n = 0;
    yukle(&p, 1); yukle(&p, 0); ekle_komut(&p, K_BOL); ekle_komut(&p, K_DURDUR);
    KONTROL(kos(&vm, &p) == S_SIFIRA_BOLME);

    p.n = 0; /* INT_MIN / -1 ve INT_MIN % -1: taşma olmadan sarmalanmalı */
    yukle(&p, INT32_MIN); yukle(&p, -1); ekle_komut(&p, K_BOL); ekle_komut(&p, K_YAZDIR);
    yukle(&p, INT32_MIN); yukle(&p, -1); ekle_komut(&p, K_MOD); ekle_komut(&p, K_YAZDIR);
    yukle(&p, INT32_MAX); yukle(&p, 1); ekle_komut(&p, K_TOPLA); ekle_komut(&p, K_YAZDIR);
    ekle_komut(&p, K_DURDUR);
    KONTROL(kos(&vm, &p) == S_TAMAM && vm.cikti[0] == INT32_MIN && vm.cikti[1] == 0 && vm.cikti[2] == INT32_MIN);

    p.n = 0;
    ekle_komut(&p, K_TOPLA);
    KONTROL(kos(&vm, &p) == S_YIGIN_BOS);

    p.n = 0; /* yığın taşması: sonsuz push döngüsü */
    size_t bas = p.n;
    yukle(&p, 1);
    atla(&p, K_ATLA, (uint32_t)bas);
    KONTROL(kos(&vm, &p) == S_YIGIN_TASMA);

    p.n = 0; /* sonsuz döngü adım sınırıyla durmalı */
    atla(&p, K_ATLA, 0);
    KONTROL(kos(&vm, &p) == S_ADIM_SINIRI);

    p.n = 0;
    atla(&p, K_ATLA, 9999);
    KONTROL(kos(&vm, &p) == S_GECERSIZ_ATLAMA);

    p.n = 0;
    p.kod[p.n++] = 200;
    KONTROL(kos(&vm, &p) == S_GECERSIZ_KOMUT);

    p.n = 0;
    p.kod[p.n++] = K_YUKLE; /* operand eksik */
    p.kod[p.n++] = 1;
    KONTROL(kos(&vm, &p) == S_KOD_BITTI);
    p.n = 0;
    yukle(&p, 1);
    KONTROL(kos(&vm, &p) == S_KOD_BITTI); /* DURDUR yok */

    p.n = 0;
    yukle(&p, 1); degisken(&p, K_YAZ, 16);
    KONTROL(kos(&vm, &p) == S_GECERSIZ_DEGISKEN);

    p.n = 0; /* çıktı tamponu dolar: döngüde sürekli yazdır */
    yukle(&p, 1);
    ekle_komut(&p, K_YAZDIR);
    atla(&p, K_ATLA, 0);
    KONTROL(kos(&vm, &p) == S_CIKTI_DOLU && vm.cikti_sayisi == CIKTI_BOYUT);

    puts(hata_sayisi ? "senior: HATA VAR" : "senior: hepsi geçti");
    return hata_sayisi != 0;
}
