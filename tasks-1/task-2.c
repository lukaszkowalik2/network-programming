/*
Polecenie:
Przypomnij sobie wiadomości o wskaźnikach i arytmetyce wskaźnikowej w C. Napisz alternatywną wersję funkcji drukującej liczby, o sygnaturze void drukuj_alt(int * tablica, int liczba_elementow). Nie używaj w niej indeksowania zmienną całkowitoliczbową (nie może się więc pojawić ani tablica[i], ani *(tablica+i)), zamiast tego użyj wskaźnika przesuwanego z elementu na element przy pomocy ++.

W dwóch następnych zadaniach też używaj przesuwanego wskaźnika zamiast indeksowania zmienną całkowitoliczbową.
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void drukuj_alt(int *numbers, int num_counted){
  printf("Wypisuję liczby większe od 10 i mniejsze od 100:\n");
  printf("[");
  bool first = true;
  for (int *ptr = numbers; ptr < numbers + num_counted; ptr++) {
    if (*ptr > 10 && *ptr < 100) {
      if (first) {
        printf("%d", *ptr);
        first = false;
      } else {
        printf(", %d", *ptr);
      }
    }
  }
  printf("]\n");
}


int main(int argc, char *argv[]) {
  int numbers[50];
  int num_counted = 0;

  while (num_counted < 50) {
    printf("Podaj liczbę %d: ", num_counted + 1);
    int result = scanf("%d", &numbers[num_counted]);
    if (result == 0) {
      break;
    }
    if (numbers[num_counted] == 0) {
      break;
    }
    num_counted++;
  }
  drukuj_alt(numbers, num_counted);

  return 0;
}
