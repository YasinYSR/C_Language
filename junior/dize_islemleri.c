/* Dize (string) işlemleri: ters çevirme, palindrom, kelime sayma, büyük harfe çevirme.
   Yalnızca ASCII ile çalışır; UTF-8 çok baytlı karakterler (ç, ğ, ş...) bayt bayt işlenir.
   Derle: gcc dize_islemleri.c -o dize      Çalıştır: dize "Ey Edip Adanada pide ye" */
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Dizeyi yerinde ters çevirir. */
void dize_ters(char *s)
{
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
}

/* Harf ve rakamlara bakar, büyük/küçük harf ve diğer karakterleri yok sayar. */
int palindrom_mu(const char *s)
{
    size_t i = 0, j = strlen(s);
    while (i < j) {
        while (i < j && !isalnum((unsigned char)s[i]))
            i++;
        while (i < j && !isalnum((unsigned char)s[j - 1]))
            j--;
        if (i >= j)
            break;
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)s[j - 1]))
            return 0;
        i++;
        j--;
    }
    return 1;
}

/* Boşlukla ayrılmış kelime sayısı. */
int kelime_say(const char *s)
{
    int say = 0, icinde = 0;
    for (; *s; s++) {
        if (isspace((unsigned char)*s))
            icinde = 0;
        else if (!icinde) {
            icinde = 1;
            say++;
        }
    }
    return say;
}

void buyut(char *s)
{
    for (; *s; s++)
        *s = (char)toupper((unsigned char)*s);
}

#ifndef TEST
int main(int argc, char **argv)
{
    char metin[256];
    snprintf(metin, sizeof metin, "%s", argc > 1 ? argv[1] : "Ey Edip Adanada pide ye");

    printf("palindrom mu : %s\n", palindrom_mu(metin) ? "evet" : "hayır");
    printf("kelime sayısı: %d\n", kelime_say(metin));
    dize_ters(metin);
    printf("ters         : %s\n", metin);
    buyut(metin);
    printf("büyük harf   : %s\n", metin);
    return 0;
}
#endif
