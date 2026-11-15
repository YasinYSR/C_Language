/* Bellek havuzu (arena allocator): küçük ayırmaları büyük bloklardan keser, hepsini tek seferde serbest bırakır.
   Tek tek free yok; ömrü aynı olan nesneler için (örn. bir ifade ağacı) hızlı ve sızıntısız bir yol.
   Hizalama verilebilir. Havuz dolarsa yeni blok bağlanır, eski bloklardaki işaretçiler geçerli kalır.
   Derle: gcc bellek_havuzu.c -o havuz */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Blok {
    struct Blok *onceki;
    size_t kullanilan, kapasite;
    unsigned char veri[];
} Blok;

typedef struct {
    Blok *son;
    size_t blok_boyutu;
} Havuz;

static Blok *blok_yap(Blok *onceki, size_t kapasite)
{
    Blok *b = malloc(sizeof *b + kapasite);
    if (b) {
        b->onceki = onceki;
        b->kullanilan = 0;
        b->kapasite = kapasite;
    }
    return b;
}

/* İlk blok hemen ayrılır; olmazsa son = NULL döner. */
Havuz havuz_yeni(size_t blok_boyutu)
{
    Havuz h = {NULL, blok_boyutu ? blok_boyutu : 4096};
    h.son = blok_yap(NULL, h.blok_boyutu);
    return h;
}

/* n bayt ayırır. hiza 2'nin kuvveti olmalı (1, 2, 4, 8, 16...). Ayrılan bellek sıfırlanmaz.
   Başarısızlıkta NULL: bellek yok, hiza geçersiz ya da boyut taşması. */
void *havuz_al(Havuz *h, size_t n, size_t hiza)
{
    if (!h->son || hiza == 0 || (hiza & (hiza - 1)) != 0)
        return NULL;

    Blok *b = h->son;
    uintptr_t taban = (uintptr_t)b->veri + b->kullanilan;
    size_t dolgu = (size_t)(-taban & (hiza - 1)); /* bir sonraki hizalı adrese kadar boşluk */
    if (n <= SIZE_MAX - dolgu && dolgu + n <= b->kapasite - b->kullanilan) {
        b->kullanilan += dolgu + n;
        return (void *)(taban + dolgu);
    }

    /* sığmadı: yeni blok. Hizalama payı için hiza kadar fazla yer iste. */
    if (n > SIZE_MAX - hiza)
        return NULL;
    size_t kapasite = n + hiza > h->blok_boyutu ? n + hiza : h->blok_boyutu;
    Blok *yeni = blok_yap(b, kapasite);
    if (!yeni)
        return NULL;
    h->son = yeni;
    taban = (uintptr_t)yeni->veri;
    dolgu = (size_t)(-taban & (hiza - 1));
    yeni->kullanilan = dolgu + n;
    return (void *)(taban + dolgu);
}

/* Her şeyi serbest bırakır ama ilk bloğu saklar; havuz yeniden kullanıma hazır olur. */
void havuz_sifirla(Havuz *h)
{
    Blok *b = h->son;
    while (b && b->onceki) {
        Blok *onceki = b->onceki;
        free(b);
        b = onceki;
    }
    h->son = b;
    if (b)
        b->kullanilan = 0;
}

void havuz_yok(Havuz *h)
{
    while (h->son) {
        Blok *onceki = h->son->onceki;
        free(h->son);
        h->son = onceki;
    }
}

#ifndef TEST
typedef struct Ag {
    int deger;
    struct Ag *sol, *sag;
} Ag;

static Ag *ekle(Havuz *h, Ag *kok, int deger)
{
    if (!kok) {
        Ag *d = havuz_al(h, sizeof *d, _Alignof(Ag));
        *d = (Ag){deger, NULL, NULL};
        return d;
    }
    if (deger < kok->deger)
        kok->sol = ekle(h, kok->sol, deger);
    else
        kok->sag = ekle(h, kok->sag, deger);
    return kok;
}

static void sirali(const Ag *k)
{
    if (k) {
        sirali(k->sol);
        printf("%d ", k->deger);
        sirali(k->sag);
    }
}

int main(void)
{
    /* İkili arama ağacı: düğümler havuzdan gelir, tek tek free gerekmez. */
    Havuz h = havuz_yeni(256);
    Ag *kok = NULL;
    int girdi[] = {50, 30, 70, 20, 40, 60, 80, 35, 65};
    for (size_t i = 0; i < sizeof girdi / sizeof *girdi; i++)
        kok = ekle(&h, kok, girdi[i]);
    sirali(kok);
    putchar('\n');
    havuz_yok(&h);
    return 0;
}
#endif
