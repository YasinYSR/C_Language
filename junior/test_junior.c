#define TEST
#include <string.h>

#include "asal_sayilar.c"
#include "dize_islemleri.c"
#include "not_ortalamasi.c"

static int hata_sayisi;
#define KONTROL(k) do { if (!(k)) { printf("BASARISIZ %s:%d  %s\n", __FILE__, __LINE__, #k); hata_sayisi++; } } while (0)

int main(void)
{
    int adet;
    int *a = asal_bul(100, &adet);
    KONTROL(adet == 25 && a[0] == 2 && a[24] == 97);
    free(a);
    a = asal_bul(100000, &adet);
    KONTROL(adet == 9592);
    free(a);
    KONTROL(asal_bul(1, &adet) == NULL && adet == 0);
    a = asal_bul(2, &adet);
    KONTROL(adet == 1 && a[0] == 2);
    free(a);

    char s[32] = "merhaba";
    dize_ters(s);
    KONTROL(strcmp(s, "abahrem") == 0);
    char bos[1] = "";
    dize_ters(bos);
    KONTROL(bos[0] == '\0');
    KONTROL(palindrom_mu("Ey Edip Adanada pide ye"));
    KONTROL(palindrom_mu("A man, a plan, a canal: Panama"));
    KONTROL(!palindrom_mu("merhaba"));
    KONTROL(palindrom_mu("") && palindrom_mu("!!"));
    KONTROL(kelime_say("  bir  iki\tuc\n") == 3 && kelime_say("") == 0);
    char k[] = "abc-1";
    buyut(k);
    KONTROL(strcmp(k, "ABC-1") == 0);

    double v[] = {2, 4, 4, 4, 5, 5, 7, 9};
    Istatistik st;
    KONTROL(istatistik_hesapla(v, 8, &st) == 0);
    KONTROL(st.ortalama == 5.0 && st.sapma == 2.0 && st.en_kucuk == 2 && st.en_buyuk == 9 && st.medyan == 4.5);
    KONTROL(v[0] == 2 && v[7] == 9); /* girdi sıralanmamış kalmalı */
    double tek[] = {9, 1, 5};
    istatistik_hesapla(tek, 3, &st);
    KONTROL(st.medyan == 5);
    KONTROL(istatistik_hesapla(tek, 0, &st) == -1);

    puts(hata_sayisi ? "junior: HATA VAR" : "junior: hepsi geçti");
    return hata_sayisi != 0;
}
