/* Hash tablosu: dize anahtar -> int değer. Zincirleme (chaining) çakışma çözümü, FNV-1a özeti,
   doluluk 0.75'i geçince kova sayısını ikiye katlayıp düğümleri yeniden bağlar (düğümler kopyalanmaz).
   Derle: gcc hash_tablosu.c -o hash */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Girdi {
    char *anahtar;
    int deger;
    uint64_t ozet; /* yeniden boyutlandırmada tekrar hesaplanmasın diye saklanır */
    struct Girdi *sonraki;
} Girdi;

typedef struct {
    Girdi **kovalar;
    size_t kova_sayisi; /* her zaman 2'nin kuvveti: indeks = ozet & (kova_sayisi - 1) */
    size_t boyut;
} Tablo;

static uint64_t fnv1a(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    for (; *s; s++) {
        h ^= (unsigned char)*s;
        h *= 1099511628211ULL;
    }
    return h;
}

Tablo *tablo_yeni(void)
{
    Tablo *t = malloc(sizeof *t);
    if (!t)
        return NULL;
    t->kova_sayisi = 8;
    t->boyut = 0;
    t->kovalar = calloc(t->kova_sayisi, sizeof *t->kovalar);
    if (!t->kovalar) {
        free(t);
        return NULL;
    }
    return t;
}

static int buyut(Tablo *t)
{
    size_t yeni_sayi = t->kova_sayisi * 2;
    Girdi **yeni = calloc(yeni_sayi, sizeof *yeni);
    if (!yeni)
        return -1; /* tablo eski hâliyle kullanılabilir kalır */
    for (size_t i = 0; i < t->kova_sayisi; i++)
        for (Girdi *g = t->kovalar[i], *sonraki; g; g = sonraki) {
            sonraki = g->sonraki;
            size_t k = g->ozet & (yeni_sayi - 1);
            g->sonraki = yeni[k];
            yeni[k] = g;
        }
    free(t->kovalar);
    t->kovalar = yeni;
    t->kova_sayisi = yeni_sayi;
    return 0;
}

/* Ekler ya da var olan anahtarın değerini günceller. Başarıda 0, bellek hatasında -1. */
int tablo_koy(Tablo *t, const char *anahtar, int deger)
{
    uint64_t ozet = fnv1a(anahtar);
    for (Girdi *g = t->kovalar[ozet & (t->kova_sayisi - 1)]; g; g = g->sonraki)
        if (g->ozet == ozet && strcmp(g->anahtar, anahtar) == 0) {
            g->deger = deger;
            return 0;
        }

    if ((t->boyut + 1) * 4 > t->kova_sayisi * 3)
        buyut(t); /* başarısız olursa zincirler uzar ama doğruluk bozulmaz */

    Girdi *g = malloc(sizeof *g);
    size_t n = strlen(anahtar) + 1;
    char *kopya = g ? malloc(n) : NULL;
    if (!kopya) {
        free(g);
        return -1;
    }
    memcpy(kopya, anahtar, n);
    size_t k = ozet & (t->kova_sayisi - 1);
    *g = (Girdi){kopya, deger, ozet, t->kovalar[k]};
    t->kovalar[k] = g;
    t->boyut++;
    return 0;
}

/* Bulunursa 1 döner ve *deger'e yazar, yoksa 0. */
int tablo_al(const Tablo *t, const char *anahtar, int *deger)
{
    uint64_t ozet = fnv1a(anahtar);
    for (const Girdi *g = t->kovalar[ozet & (t->kova_sayisi - 1)]; g; g = g->sonraki)
        if (g->ozet == ozet && strcmp(g->anahtar, anahtar) == 0) {
            *deger = g->deger;
            return 1;
        }
    return 0;
}

/* Silindiyse 1, yoksa 0. */
int tablo_sil(Tablo *t, const char *anahtar)
{
    uint64_t ozet = fnv1a(anahtar);
    for (Girdi **p = &t->kovalar[ozet & (t->kova_sayisi - 1)]; *p; p = &(*p)->sonraki) {
        Girdi *g = *p;
        if (g->ozet == ozet && strcmp(g->anahtar, anahtar) == 0) {
            *p = g->sonraki; /* işaretçinin adresini tutmak baş/orta ayrımını ortadan kaldırır */
            free(g->anahtar);
            free(g);
            t->boyut--;
            return 1;
        }
    }
    return 0;
}

void tablo_yok(Tablo *t)
{
    if (!t)
        return;
    for (size_t i = 0; i < t->kova_sayisi; i++)
        for (Girdi *g = t->kovalar[i], *sonraki; g; g = sonraki) {
            sonraki = g->sonraki;
            free(g->anahtar);
            free(g);
        }
    free(t->kovalar);
    free(t);
}

#ifndef TEST
int main(void)
{
    Tablo *t = tablo_yeni();
    const char *kelimeler[] = {"elma", "armut", "elma", "kiraz", "elma", "armut"};
    for (size_t i = 0; i < sizeof kelimeler / sizeof *kelimeler; i++) {
        int sayi = 0;
        tablo_al(t, kelimeler[i], &sayi);
        tablo_koy(t, kelimeler[i], sayi + 1);
    }
    const char *sorgu[] = {"elma", "armut", "kiraz", "muz"};
    for (size_t i = 0; i < 4; i++) {
        int sayi;
        if (tablo_al(t, sorgu[i], &sayi))
            printf("%-6s %d\n", sorgu[i], sayi);
        else
            printf("%-6s yok\n", sorgu[i]);
    }
    tablo_yok(t);
    return 0;
}
#endif
