/*
Polecenie:
Opracuj alternatywną wersję funkcji, biorącą jako argument łańcuch w sensie języka C, czyli ciąg niezerowych bajtów zakończony bajtem równym zero (ten końcowy bajt nie jest uznawany za należący do łańcucha). Ta wersja funkcji ma mieć sygnaturę bool is_printable_str(const char * str).
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool is_printable_str(const char *str){
  for (const char *ptr = str; *ptr != '\0'; ptr++) {
    if (*ptr < 32 || *ptr > 126) {
      return false;
    }
  }
  return true;
}

int main(){
  char printable_str[] = "Hello, World!";
  if (is_printable_str(printable_str)) {
    printf("\"%s\" is printable.\n", printable_str);
  } else {
    printf("\"%s\" is not printable.\n", printable_str);
  }

  char non_printable_str[] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
  if (is_printable_str(non_printable_str)) {
    printf("\"%s\" is printable.\n", non_printable_str);
  } else {
    printf("\"%s\" is not printable.\n", non_printable_str);
  }
  return 0;
}