# C_Language

C programlama dili üzerine küçük örnekler.

| Dosya | Ne yapar |
| --- | --- |
| [`birim_kare_matris.c`](birim_kare_matris.c) | Girilen büyüklükte birim matris (köşegen 1, diğerleri 0) üretip yazdırır |
| [`coder.c`](coder.c) | Girilen metni bir anahtarla (`0xFACA`) kodlar, sonra geri çözer |
| [`karakterin ascii kodu.c`](karakterin%20ascii%20kodu.c) | Girilen karakterin ASCII kodunu yazar |

## Derleme

```bash
gcc dosya.c -o dosya
```

Notlar:

- `birim_kare_matris.c`, `conio.h` kullandığı için Windows derleyicisi (MinGW ya da MSVC) ister.
- `coder.c` içinde `code` ve `decode` fonksiyonları kullanımdan sonra tanımlı. Yeni derleyiciler bunu hata sayabilir; gerekirse `-std=gnu89` ile derle.
