/* Not istatistikleri: ortalama, standart sapma, en küçük/büyük, medyan.
   Derle: gcc not_ortalamasi.c -o notlar -lm      Çalıştır: notlar 70 85 90 55 100 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double ortalama, sapma, en_kucuk, en_buyuk, medyan;
} Istatistik;

static int not_karsilastir(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

/* Başarılıysa 0, boş girdi ya da bellek hatasında -1 döner. Sapma popülasyon sapmasıdır (n'e bölünür). */
int istatistik_hesapla(const double *v, size_t n, Istatistik *s)
{
    if (n == 0)
        return -1;

    double *kopya = malloc(n * sizeof *kopya); /* medyan için sıralamak gerek, girdiyi bozma */
    if (!kopya)
        return -1;
    for (size_t i = 0; i < n; i++)
        kopya[i] = v[i];
    qsort(kopya, n, sizeof *kopya, not_karsilastir);

    double toplam = 0;
    for (size_t i = 0; i < n; i++)
        toplam += v[i];
    s->ortalama = toplam / (double)n;

    double kare = 0;
    for (size_t i = 0; i < n; i++)
        kare += (v[i] - s->ortalama) * (v[i] - s->ortalama);
    s->sapma = sqrt(kare / (double)n);

    s->en_kucuk = kopya[0];
    s->en_buyuk = kopya[n - 1];
    s->medyan = n % 2 ? kopya[n / 2] : (kopya[n / 2 - 1] + kopya[n / 2]) / 2;
    free(kopya);
    return 0;
}

#ifndef TEST
int main(int argc, char **argv)
{
    double notlar[64];
    size_t n = 0;
    for (int i = 1; i < argc && n < 64; i++)
        notlar[n++] = atof(argv[i]);
    if (n == 0) {
        double ornek[] = {70, 85, 90, 55, 100, 45};
        for (n = 0; n < 6; n++)
            notlar[n] = ornek[n];
    }

    Istatistik s;
    if (istatistik_hesapla(notlar, n, &s) != 0)
        return 1;
    printf("ortalama: %.2f\nsapma   : %.2f\nen küçük: %.0f\nen büyük: %.0f\nmedyan  : %.1f\n",
           s.ortalama, s.sapma, s.en_kucuk, s.en_buyuk, s.medyan);
    return 0;
}
#endif
