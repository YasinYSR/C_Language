/* Tek yönlü bağlı liste. Aynı liste yığın (LIFO) ve kuyruk (FIFO) olarak da kullanılır.
   Baş ve son işaretçisi tutulduğu için hem başa hem sona ekleme O(1).
   Derle: gcc bagli_liste.c -o liste */
#include <stdio.h>
#include <stdlib.h>

typedef struct Dugum {
    int deger;
    struct Dugum *sonraki;
} Dugum;

typedef struct {
    Dugum *bas, *son;
    size_t uzunluk;
} Liste;

void liste_baslat(Liste *l)
{
    l->bas = l->son = NULL;
    l->uzunluk = 0;
}

static Dugum *dugum_yap(int deger)
{
    Dugum *d = malloc(sizeof *d);
    if (d) {
        d->deger = deger;
        d->sonraki = NULL;
    }
    return d;
}

/* Ekleme fonksiyonları başarıda 0, bellek hatasında -1 döner. */
int liste_basa_ekle(Liste *l, int deger)
{
    Dugum *d = dugum_yap(deger);
    if (!d)
        return -1;
    d->sonraki = l->bas;
    l->bas = d;
    if (!l->son)
        l->son = d;
    l->uzunluk++;
    return 0;
}

int liste_sona_ekle(Liste *l, int deger)
{
    Dugum *d = dugum_yap(deger);
    if (!d)
        return -1;
    if (l->son)
        l->son->sonraki = d;
    else
        l->bas = d;
    l->son = d;
    l->uzunluk++;
    return 0;
}

/* Baştaki elemanı çıkarır ve *deger'e yazar. Liste boşsa -1. */
int liste_bastan_cikar(Liste *l, int *deger)
{
    Dugum *d = l->bas;
    if (!d)
        return -1;
    *deger = d->deger;
    l->bas = d->sonraki;
    if (!l->bas)
        l->son = NULL;
    free(d);
    l->uzunluk--;
    return 0;
}

/* Değeri eşleşen ilk düğümü siler. Bulunduysa 1, yoksa 0. */
int liste_sil(Liste *l, int deger)
{
    Dugum *onceki = NULL;
    for (Dugum *d = l->bas; d; onceki = d, d = d->sonraki) {
        if (d->deger != deger)
            continue;
        if (onceki)
            onceki->sonraki = d->sonraki;
        else
            l->bas = d->sonraki;
        if (d == l->son)
            l->son = onceki;
        free(d);
        l->uzunluk--;
        return 1;
    }
    return 0;
}

/* İşaretçileri çevirerek yerinde ters çevirir, yeni düğüm ayırmaz. */
void liste_ters(Liste *l)
{
    Dugum *onceki = NULL, *d = l->bas;
    l->son = l->bas;
    while (d) {
        Dugum *sonraki = d->sonraki;
        d->sonraki = onceki;
        onceki = d;
        d = sonraki;
    }
    l->bas = onceki;
}

void liste_temizle(Liste *l)
{
    Dugum *d = l->bas;
    while (d) {
        Dugum *sonraki = d->sonraki;
        free(d);
        d = sonraki;
    }
    liste_baslat(l);
}

/* Yığın = listenin başı; kuyruk = sonuna ekle, başından çıkar. */
#define yigin_it(l, v) liste_basa_ekle((l), (v))
#define yigin_cek(l, p) liste_bastan_cikar((l), (p))
#define kuyruga_ekle(l, v) liste_sona_ekle((l), (v))
#define kuyruktan_al(l, p) liste_bastan_cikar((l), (p))

#ifndef TEST
static void liste_yazdir(const Liste *l)
{
    for (const Dugum *d = l->bas; d; d = d->sonraki)
        printf("%d%s", d->deger, d->sonraki ? " -> " : "\n");
}

int main(void)
{
    Liste l;
    liste_baslat(&l);
    for (int i = 1; i <= 5; i++)
        liste_sona_ekle(&l, i * 10);
    liste_yazdir(&l);
    liste_ters(&l);
    liste_yazdir(&l);
    liste_sil(&l, 30);
    liste_yazdir(&l);
    liste_temizle(&l);
    return 0;
}
#endif
