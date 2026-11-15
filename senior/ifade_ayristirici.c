/* Özyinelemeli iniş (recursive descent) ayrıştırıcıyla ifade hesaplayıcı.
   Gramer (öncelik aşağı doğru artar):
     toplam : carpim (('+'|'-') carpim)*
     carpim : tekli  (('*'|'/'|'%') tekli)*
     tekli  : ('-'|'+') tekli | us
     us     : atom ('^' tekli)?          sağdan birleşimli, -2^2 = -(2^2), 2^-1 serbest
     atom   : sayı | '(' toplam ')' | ad '(' toplam ')'     ad: sqrt abs sin cos
   Derle: gcc ifade_ayristirici.c -o ifade -lm      Çalıştır: ifade "2 + 3 * (4 - 1)^2" */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKS_DERINLIK 200 /* "((((...", "-----..." gibi girdiler yığını taşırmasın */

typedef struct {
    const char *p;
    const char *hata; /* ilk hata saklanır, sonrakiler ezmez */
    int derinlik;
} Ayristirici;

static double toplam(Ayristirici *a);

static void bosluk_atla(Ayristirici *a)
{
    while (isspace((unsigned char)*a->p))
        a->p++;
}

static int hata_ver(Ayristirici *a, const char *mesaj)
{
    if (!a->hata)
        a->hata = mesaj;
    return 0;
}

static int istek(Ayristirici *a, char c)
{
    bosluk_atla(a);
    if (*a->p == c) {
        a->p++;
        return 1;
    }
    return 0;
}

static double atom(Ayristirici *a)
{
    bosluk_atla(a);
    if (isdigit((unsigned char)*a->p) || *a->p == '.') {
        char *son;
        double v = strtod(a->p, &son);
        a->p = son;
        return v;
    }
    if (*a->p == '(') {
        a->p++;
        double v = toplam(a);
        if (!istek(a, ')'))
            hata_ver(a, "')' bekleniyordu");
        return v;
    }
    if (isalpha((unsigned char)*a->p)) {
        char ad[8] = {0};
        size_t n = 0;
        while (isalpha((unsigned char)*a->p)) {
            if (n < sizeof ad - 1)
                ad[n++] = *a->p;
            a->p++;
        }
        if (!istek(a, '(')) {
            hata_ver(a, "fonksiyon adından sonra '(' bekleniyordu");
            return 0;
        }
        double v = toplam(a);
        if (!istek(a, ')'))
            hata_ver(a, "')' bekleniyordu");
        if (strcmp(ad, "sqrt") == 0) {
            if (v < 0)
                hata_ver(a, "negatif sayının karekökü");
            return sqrt(v);
        }
        if (strcmp(ad, "abs") == 0)
            return fabs(v);
        if (strcmp(ad, "sin") == 0)
            return sin(v);
        if (strcmp(ad, "cos") == 0)
            return cos(v);
        hata_ver(a, "bilinmeyen fonksiyon");
        return 0;
    }
    hata_ver(a, "sayı ya da '(' bekleniyordu");
    return 0;
}

static double tekli(Ayristirici *a);

static double us(Ayristirici *a)
{
    double taban = atom(a);
    if (istek(a, '^')) {
        double kuvvet = tekli(a); /* tekli üzerinden dönmek: sağdan birleşim ve 2^-1 */
        return pow(taban, kuvvet);
    }
    return taban;
}

static double tekli(Ayristirici *a)
{
    if (++a->derinlik > MAKS_DERINLIK) {
        hata_ver(a, "ifade çok iç içe");
        a->derinlik--;
        return 0;
    }
    double v;
    if (istek(a, '-'))
        v = -tekli(a);
    else if (istek(a, '+'))
        v = tekli(a);
    else
        v = us(a);
    a->derinlik--;
    return v;
}

static double carpim(Ayristirici *a)
{
    double v = tekli(a);
    for (;;) {
        if (istek(a, '*'))
            v *= tekli(a);
        else if (istek(a, '/')) {
            double b = tekli(a);
            if (b == 0)
                hata_ver(a, "sıfıra bölme");
            else
                v /= b;
        } else if (istek(a, '%')) {
            double b = tekli(a);
            if (b == 0)
                hata_ver(a, "sıfıra bölme");
            else
                v = fmod(v, b);
        } else
            return v;
    }
}

static double toplam(Ayristirici *a)
{
    if (++a->derinlik > MAKS_DERINLIK) {
        hata_ver(a, "ifade çok iç içe");
        a->derinlik--;
        return 0;
    }
    double v = carpim(a);
    for (;;) {
        if (istek(a, '+'))
            v += carpim(a);
        else if (istek(a, '-'))
            v -= carpim(a);
        else
            break;
    }
    a->derinlik--;
    return v;
}

/* Başarıda 0 döner ve *sonuc'u doldurur. Hatada -1 döner, *hata mesajı gösterir (varsa). */
int ifade_hesapla(const char *metin, double *sonuc, const char **hata)
{
    Ayristirici a = {metin, NULL, 0};
    double v = toplam(&a);
    bosluk_atla(&a);
    if (!a.hata && *a.p)
        hata_ver(&a, "ifadenin sonunda beklenmeyen karakter");
    if (a.hata) {
        if (hata)
            *hata = a.hata;
        return -1;
    }
    *sonuc = v;
    return 0;
}

#ifndef TEST
int main(int argc, char **argv)
{
    const char *ifade = argc > 1 ? argv[1] : "2 + 3 * (4 - 1)^2";
    double v;
    const char *hata;
    if (ifade_hesapla(ifade, &v, &hata) != 0) {
        fprintf(stderr, "hata: %s\n", hata);
        return 1;
    }
    printf("%s = %.10g\n", ifade, v);
    return 0;
}
#endif
