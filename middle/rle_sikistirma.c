/* Çalışma uzunluğu kodlaması (RLE): tekrar eden baytları (sayı, bayt) çiftine sıkıştırır.
   Biçim: [tekrar sayısı 1..255][bayt] çiftleri. Her türlü bayt dizisiyle çalışır (metin olmak zorunda değil).
   Derle: gcc rle_sikistirma.c -o rle      Çalıştır: rle AAAAAAABBBC */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RLE_HATA ((size_t)-1)

/* girdiyi cikti'ya kodlar, yazılan bayt sayısını döndürür. Kapasite yetmezse RLE_HATA.
   En kötü durum (hiç tekrar yok) çıktı girdinin iki katıdır. */
size_t rle_kodla(const uint8_t *girdi, size_t n, uint8_t *cikti, size_t kapasite)
{
    size_t yazilan = 0;
    for (size_t i = 0; i < n;) {
        uint8_t bayt = girdi[i];
        size_t tekrar = 1;
        while (i + tekrar < n && girdi[i + tekrar] == bayt && tekrar < 255)
            tekrar++;
        if (yazilan + 2 > kapasite)
            return RLE_HATA;
        cikti[yazilan++] = (uint8_t)tekrar;
        cikti[yazilan++] = bayt;
        i += tekrar;
    }
    return yazilan;
}

/* Kodu açar. Tek sayıda bayt, sıfır sayaç ya da kapasite aşımında RLE_HATA döner. */
size_t rle_coz(const uint8_t *girdi, size_t n, uint8_t *cikti, size_t kapasite)
{
    if (n % 2)
        return RLE_HATA;
    size_t yazilan = 0;
    for (size_t i = 0; i < n; i += 2) {
        size_t tekrar = girdi[i];
        if (tekrar == 0 || yazilan + tekrar > kapasite)
            return RLE_HATA;
        memset(cikti + yazilan, girdi[i + 1], tekrar);
        yazilan += tekrar;
    }
    return yazilan;
}

#ifndef TEST
int main(int argc, char **argv)
{
    const char *metin = argc > 1 ? argv[1] : "AAAAAAAABBBCCDAAAA";
    uint8_t kod[512], geri[512];
    size_t n = strlen(metin);

    size_t k = rle_kodla((const uint8_t *)metin, n, kod, sizeof kod);
    if (k == RLE_HATA)
        return 1;
    printf("girdi: %zu bayt, kod: %zu bayt\n", n, k);
    for (size_t i = 0; i < k; i += 2)
        printf("%u×'%c' ", kod[i], kod[i + 1]);

    size_t g = rle_coz(kod, k, geri, sizeof geri);
    printf("\ngeri çözülen: %.*s\n", (int)g, (const char *)geri);
    return 0;
}
#endif
