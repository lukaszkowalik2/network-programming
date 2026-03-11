/*
Polecenie:
Opracuj funkcję sprawdzającą, czy przekazany jej bufor zawiera tylko i wyłącznie drukowalne znaki ASCII, tzn. bajty o wartościach z przedziału domkniętego [32, 126]. Funkcja ma mieć następującą sygnaturę: bool is_printable_buf(const void * buf, int len). Pamiętaj o włączeniu nagłówka <stdbool.h>, bez niego kompilator nie rozpozna ani nazwy typu bool, ani nazw stałych true i false.

Konieczne będzie użycie rzutowania wskaźników, bo typ void * oznacza „adres w pamięci, ale bez informacji o tym co w tym fragmencie pamięci się znajduje". Na początku ciała funkcji trzeba go więc zrzutować do typu „adres fragmentu pamięci zawierającego ciąg bajtów".

Napisz też jakiś prosty program, który pozwoli Ci przetestować działanie funkcji is_printable_buf.
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool is_printable_buf(const void *buf, int len){
  const char *ptr = (const char *)buf;
  for (const char *end = ptr + len; ptr < end; ptr++) {
    if (*ptr < 32 || *ptr > 126) {
      return false;
    }
  }
  return true;
}

int main(){
  char printable_buf[] = "Hello, World!";
  if (is_printable_buf(printable_buf, strlen(printable_buf))) {
    printf("\"%s\" is printable.\n", printable_buf);
  } else {
    printf("\"%s\" is not printable.\n", printable_buf);
  }

  char non_printable_buf[] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
  if (is_printable_buf(non_printable_buf, strlen(non_printable_buf))) {
    printf("\"%s\" is printable.\n", non_printable_buf);
  } else {
    printf("\"%s\" is not printable.\n", non_printable_buf);
  }

  return 0;
}
