/* Dinamik matris: çarpma, devrik (transpoze) ve determinant (kısmi pivotlu Gauss eliminasyonu).
   Derle: gcc matris.c -o matris -lm */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t sat, sut;
    double *v; /* satır sıralı: eleman(i, j) = v[i * sut + j] */
} Matris;

#define ELEMAN(m, i, j) ((m)->v[(i) * (m)->sut + (j)])

/* Sıfırlarla dolu matris. Boyut 0 ya da bellek yetmezse v = NULL döner. */
Matris matris_olustur(size_t sat, size_t sut)
{
    Matris m = {sat, sut, NULL};
    if (sat && sut && sat <= SIZE_MAX / sut / sizeof(double))
        m.v = calloc(sat * sut, sizeof(double));
    return m;
}

void matris_yok(Matris *m)
{
    free(m->v);
    m->v = NULL;
}

/* c = a * b. 0: tamam, -1: boyutlar uyumsuz, -2: bellek yok. */
int matris_carp(const Matris *a, const Matris *b, Matris *c)
{
    if (a->sut != b->sat)
        return -1;
    *c = matris_olustur(a->sat, b->sut);
    if (!c->v)
        return -2;
    for (size_t i = 0; i < a->sat; i++)
        for (size_t k = 0; k < a->sut; k++) { /* i-k-j sırası: bellekte ardışık erişir */
            double aik = ELEMAN(a, i, k);
            for (size_t j = 0; j < b->sut; j++)
                ELEMAN(c, i, j) += aik * ELEMAN(b, k, j);
        }
    return 0;
}

Matris matris_devrik(const Matris *a)
{
    Matris t = matris_olustur(a->sut, a->sat);
    if (t.v)
        for (size_t i = 0; i < a->sat; i++)
            for (size_t j = 0; j < a->sut; j++)
                ELEMAN(&t, j, i) = ELEMAN(a, i, j);
    return t;
}

/* Determinantı *det'e yazar. 0: tamam, -1: kare değil, -2: bellek yok. */
int matris_det(const Matris *a, double *det)
{
    if (a->sat != a->sut)
        return -1;
    size_t n = a->sat;
    Matris g = matris_olustur(n, n); /* girdiyi bozmamak için kopya üzerinde çalış */
    if (!g.v)
        return -2;
    memcpy(g.v, a->v, n * n * sizeof(double));

    double sonuc = 1;
    for (size_t k = 0; k < n; k++) {
        size_t en_iyi = k; /* kısmi pivot: sütunda mutlak değeri en büyük satır */
        for (size_t i = k + 1; i < n; i++)
            if (fabs(ELEMAN(&g, i, k)) > fabs(ELEMAN(&g, en_iyi, k)))
                en_iyi = i;
        if (ELEMAN(&g, en_iyi, k) == 0) {
            sonuc = 0;
            break;
        }
        if (en_iyi != k) {
            for (size_t j = 0; j < n; j++) {
                double t = ELEMAN(&g, k, j);
                ELEMAN(&g, k, j) = ELEMAN(&g, en_iyi, j);
                ELEMAN(&g, en_iyi, j) = t;
            }
            sonuc = -sonuc; /* satır değişimi işareti çevirir */
        }
        sonuc *= ELEMAN(&g, k, k);
        for (size_t i = k + 1; i < n; i++) {
            double oran = ELEMAN(&g, i, k) / ELEMAN(&g, k, k);
            for (size_t j = k; j < n; j++)
                ELEMAN(&g, i, j) -= oran * ELEMAN(&g, k, j);
        }
    }
    matris_yok(&g);
    *det = sonuc;
    return 0;
}

#ifndef TEST
static void matris_yazdir(const Matris *m)
{
    for (size_t i = 0; i < m->sat; i++) {
        for (size_t j = 0; j < m->sut; j++)
            printf("%8.2f", ELEMAN(m, i, j));
        putchar('\n');
    }
}

int main(void)
{
    Matris a = matris_olustur(3, 3);
    double deger[] = {6, 1, 1, 4, -2, 5, 2, 8, 7};
    memcpy(a.v, deger, sizeof deger);
    double d;
    matris_det(&a, &d);
    matris_yazdir(&a);
    printf("determinant = %.1f\n", d);
    matris_yok(&a);
    return 0;
}
#endif
