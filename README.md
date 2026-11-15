# C_Language

C örnekleri, zorluğa göre üç klasörde. Standart C11, ek kütüphane yok. Her klasörde bir test dosyası var; testler `-Wall -Wextra -Wpedantic -Werror` ve `-fsanitize=undefined` ile temiz derleniyor.

| Seviye | Klasör | Ne öğretir |
| --- | --- | --- |
| Başlangıç | [`junior/`](junior) | Diziler, dizeler, `malloc`, `qsort` |
| Orta | [`middle/`](middle) | İşaretçilerle veri yapıları, dinamik matris, bayt düzeyinde kodlama |
| İleri | [`senior/`](senior) | Hash tablosu, ayrıştırıcı, kendi bellek ayırıcın, sanal makine |

## Başlangıç · `junior/`

| Dosya | Ne yapar |
| --- | --- |
| [`asal_sayilar.c`](junior/asal_sayilar.c) | Eratosthenes eleği ile asalları bulur |
| [`dize_islemleri.c`](junior/dize_islemleri.c) | Dizeyi ters çevirir, palindrom ve kelime sayısı bulur |
| [`not_ortalamasi.c`](junior/not_ortalamasi.c) | Ortalama, standart sapma, en küçük/büyük ve medyan |

## Orta · `middle/`

| Dosya | Ne yapar |
| --- | --- |
| [`bagli_liste.c`](middle/bagli_liste.c) | Tek yönlü bağlı liste: başa/sona ekleme, silme, yerinde ters çevirme. Aynı liste yığın ve kuyruk olarak da kullanılır |
| [`matris.c`](middle/matris.c) | Dinamik matris: çarpma, devrik, kısmi pivotlu Gauss ile determinant |
| [`rle_sikistirma.c`](middle/rle_sikistirma.c) | Çalışma uzunluğu (RLE) sıkıştırma ve açma, 255'ten uzun serileri bölerek yazar |

## İleri · `senior/`

| Dosya | Ne yapar |
| --- | --- |
| [`hash_tablosu.c`](senior/hash_tablosu.c) | Dize anahtarlı hash tablosu: FNV-1a, zincirleme, otomatik büyüme, silme |
| [`ifade_ayristirici.c`](senior/ifade_ayristirici.c) | Özyinelemeli iniş ayrıştırıcı: öncelik, sağdan birleşimli `^`, fonksiyonlar, hata mesajları, iç içe derinlik sınırı |
| [`bellek_havuzu.c`](senior/bellek_havuzu.c) | Arena ayırıcı: hizalama, otomatik blok ekleme, tek seferde serbest bırakma |
| [`sanal_makine.c`](senior/sanal_makine.c) | Yığın tabanlı bytecode sanal makinesi: yığın/değişken/atlama komutları, taşma ve sonsuz döngü korumaları, faktöriyel programı |

## Derleme ve test

```bash
gcc junior/asal_sayilar.c -o asal && ./asal 100
gcc senior/sanal_makine.c -o vm && ./vm
gcc senior/ifade_ayristirici.c -o ifade -lm && ./ifade "2 + 3 * (4 - 1)^2"

gcc -std=c11 -Wall -Wextra junior/test_junior.c -o test_junior -lm && ./test_junior
gcc -std=c11 -Wall -Wextra middle/test_middle.c -o test_middle -lm && ./test_middle
gcc -std=c11 -Wall -Wextra senior/test_senior.c -o test_senior -lm && ./test_senior
```

Her `.c` dosyasının `main`'i `#ifndef TEST` içinde. Test dosyası `#define TEST` yapıp kaynakları `#include` ettiği için testleri derlerken yalnızca test dosyasını ver.

## Eski dosyalar

Kökteki ilk denemeler:

| Dosya | Ne yapar |
| --- | --- |
| [`birim_kare_matris.c`](birim_kare_matris.c) | Girilen büyüklükte birim matris (köşegen 1, diğerleri 0) üretip yazdırır |
| [`coder.c`](coder.c) | Girilen metni bir anahtarla (`0xFACA`) kodlar, sonra geri çözer |
| [`karakterin ascii kodu.c`](karakterin%20ascii%20kodu.c) | Girilen karakterin ASCII kodunu yazar |

`birim_kare_matris.c`, `conio.h` kullandığı için Windows derleyicisi (MinGW ya da MSVC) ister. `coder.c` içinde `code` ve `decode` fonksiyonları kullanımdan sonra tanımlı; yeni derleyiciler bunu hata sayabilir, gerekirse `-std=gnu89` ile derle.
