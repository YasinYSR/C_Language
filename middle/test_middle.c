#define TEST
#include "bagli_liste.c"
#include "matris.c"
#include "rle_sikistirma.c"

static int hata_sayisi;
#define KONTROL(k) do { if (!(k)) { printf("BASARISIZ %s:%d  %s\n", __FILE__, __LINE__, #k); hata_sayisi++; } } while (0)

static int dizi_esit(const Liste *l, const int *beklenen, size_t n)
{
    const Dugum *d = l->bas;
    for (size_t i = 0; i < n; i++, d = d->sonraki)
        if (!d || d->deger != beklenen[i])
            return 0;
    return d == NULL && l->uzunluk == n;
}

int main(void)
{
    /* --- bağlı liste --- */
    Liste l;
    liste_baslat(&l);
    liste_sona_ekle(&l, 2);
    liste_sona_ekle(&l, 3);
    liste_basa_ekle(&l, 1);
    KONTROL(dizi_esit(&l, (int[]){1, 2, 3}, 3));
    liste_ters(&l);
    KONTROL(dizi_esit(&l, (int[]){3, 2, 1}, 3) && l.son->deger == 1);
    liste_sona_ekle(&l, 0); /* ters çevirdikten sonra son doğru mu */
    KONTROL(dizi_esit(&l, (int[]){3, 2, 1, 0}, 4));
    KONTROL(liste_sil(&l, 3) == 1 && l.bas->deger == 2); /* baş */
    KONTROL(liste_sil(&l, 0) == 1 && l.son->deger == 1); /* son */
    KONTROL(liste_sil(&l, 99) == 0);
    KONTROL(dizi_esit(&l, (int[]){2, 1}, 2));
    liste_sil(&l, 2);
    liste_sil(&l, 1);
    KONTROL(l.bas == NULL && l.son == NULL && l.uzunluk == 0);

    int v;
    for (int i = 1; i <= 3; i++)
        yigin_it(&l, i);
    KONTROL(yigin_cek(&l, &v) == 0 && v == 3 && yigin_cek(&l, &v) == 0 && v == 2); /* LIFO */
    liste_temizle(&l);
    for (int i = 1; i <= 3; i++)
        kuyruga_ekle(&l, i);
    KONTROL(kuyruktan_al(&l, &v) == 0 && v == 1 && kuyruktan_al(&l, &v) == 0 && v == 2); /* FIFO */
    kuyruktan_al(&l, &v);
    KONTROL(kuyruktan_al(&l, &v) == -1 && l.son == NULL);
    for (int i = 0; i < 100000; i++)
        liste_sona_ekle(&l, i);
    KONTROL(l.uzunluk == 100000);
    liste_temizle(&l);

    /* --- matris --- */
    Matris a = matris_olustur(2, 3), b = matris_olustur(3, 2), c;
    double av[] = {1, 2, 3, 4, 5, 6}, bv[] = {7, 8, 9, 10, 11, 12};
    memcpy(a.v, av, sizeof av);
    memcpy(b.v, bv, sizeof bv);
    KONTROL(matris_carp(&a, &b, &c) == 0);
    KONTROL(c.sat == 2 && c.sut == 2 && c.v[0] == 58 && c.v[1] == 64 && c.v[2] == 139 && c.v[3] == 154);
    matris_yok(&c);
    KONTROL(matris_carp(&a, &a, &c) == -1);
    Matris t = matris_devrik(&a);
    KONTROL(t.sat == 3 && t.sut == 2 && ELEMAN(&t, 2, 1) == 6 && ELEMAN(&t, 0, 1) == 4);
    matris_yok(&t);

    Matris m = matris_olustur(3, 3);
    double mv[] = {6, 1, 1, 4, -2, 5, 2, 8, 7};
    memcpy(m.v, mv, sizeof mv);
    double d;
    KONTROL(matris_det(&m, &d) == 0 && fabs(d - (-306)) < 1e-9);
    KONTROL(m.v[0] == 6 && m.v[8] == 7); /* girdi bozulmadı */
    double tekil[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    memcpy(m.v, tekil, sizeof tekil);
    KONTROL(matris_det(&m, &d) == 0 && fabs(d) < 1e-9);
    double pivot[] = {0, 1, 1, 0, 0, 1, 1, 0, 0}; /* ilk eleman 0: satır değişimi şart */
    memcpy(m.v, pivot, sizeof pivot);
    KONTROL(matris_det(&m, &d) == 0 && fabs(d - 1) < 1e-9);
    KONTROL(matris_det(&a, &d) == -1);
    KONTROL(matris_olustur(0, 5).v == NULL && matris_olustur(SIZE_MAX, 2).v == NULL);
    matris_yok(&m);
    matris_yok(&a);
    matris_yok(&b);

    /* --- RLE --- */
    uint8_t kod[1024], geri[1024], girdi[1000];
    const char *s = "AAAABBBCCD";
    size_t k = rle_kodla((const uint8_t *)s, 10, kod, sizeof kod);
    KONTROL(k == 8 && kod[0] == 4 && kod[1] == 'A' && kod[6] == 1 && kod[7] == 'D');
    KONTROL(rle_coz(kod, k, geri, sizeof geri) == 10 && memcmp(geri, s, 10) == 0);

    memset(girdi, 'x', 1000); /* 255 sınırını aşan tek uzun seri: 255+255+255+235 */
    k = rle_kodla(girdi, 1000, kod, sizeof kod);
    KONTROL(k == 8 && kod[0] == 255 && kod[6] == 235);
    KONTROL(rle_coz(kod, k, geri, sizeof geri) == 1000 && memcmp(geri, girdi, 1000) == 0);

    for (int i = 0; i < 400; i++) /* tekrarsız ikili veri: boyut iki katı olur, yine de doğru döner */
        girdi[i] = (uint8_t)(i * 7);
    k = rle_kodla(girdi, 400, kod, sizeof kod);
    KONTROL(k != RLE_HATA && rle_coz(kod, k, geri, sizeof geri) == 400 && memcmp(geri, girdi, 400) == 0);

    KONTROL(rle_kodla(girdi, 0, kod, sizeof kod) == 0);
    KONTROL(rle_kodla((const uint8_t *)"ABC", 3, kod, 5) == RLE_HATA); /* kapasite yetmez */
    KONTROL(rle_coz((const uint8_t *)"\x03", 1, geri, sizeof geri) == RLE_HATA); /* tek bayt */
    KONTROL(rle_coz((const uint8_t *)"\x00" "A", 2, geri, sizeof geri) == RLE_HATA); /* sıfır sayaç */
    KONTROL(rle_coz((const uint8_t *)"\xff" "A", 2, geri, 10) == RLE_HATA); /* çıktı sığmaz */

    puts(hata_sayisi ? "middle: HATA VAR" : "middle: hepsi geçti");
    return hata_sayisi != 0;
}
