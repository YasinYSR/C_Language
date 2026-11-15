/* Eratosthenes eleği ile asal sayı bulma.
   Derle: gcc asal_sayilar.c -o asal      Çalıştır: asal 100 */
#include <stdio.h>
#include <stdlib.h>

/* [2, n] aralığındaki asalları döndürür, sayısını *adet'e yazar. Dizi malloc'ludur, çağıran free eder.
   n < 2 ya da bellek yetmezse NULL döner ve *adet = 0 olur. */
int *asal_bul(int n, int *adet)
{
    *adet = 0;
    if (n < 2)
        return NULL;

    char *elenmis = calloc((size_t)n + 1, 1);
    if (!elenmis)
        return NULL;

    for (long i = 2; i * i <= n; i++)
        if (!elenmis[i])
            for (long j = i * i; j <= n; j += i) /* i'nin i*i'den küçük katları zaten elenmiş */
                elenmis[j] = 1;

    int say = 0;
    for (int i = 2; i <= n; i++)
        if (!elenmis[i])
            say++;

    int *asallar = malloc(sizeof(int) * (size_t)say);
    if (asallar) {
        int k = 0;
        for (int i = 2; i <= n; i++)
            if (!elenmis[i])
                asallar[k++] = i;
        *adet = say;
    }
    free(elenmis);
    return asallar;
}

#ifndef TEST
int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 100;
    int adet;
    int *asallar = asal_bul(n, &adet);

    printf("%d sayısına kadar %d asal var:\n", n, adet);
    for (int i = 0; i < adet; i++)
        printf("%d%c", asallar[i], (i + 1) % 15 == 0 || i + 1 == adet ? '\n' : ' ');
    free(asallar);
    return 0;
}
#endif
