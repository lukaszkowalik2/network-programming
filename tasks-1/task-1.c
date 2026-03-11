/*
Polecenie:
Napisz program w C deklarujący w funkcji main tablicę int liczby[50] i wczytujący do niej z klawiatury kolejne liczby. Wczytywanie należy przerwać gdy użytkownik wpisze zero albo gdy skończy się miejsce w tablicy (tzn. po wczytaniu 50 liczb).

Z main należy następnie wywoływać pomocniczą funkcję drukuj, przekazując jej jako argumenty adres tablicy oraz liczbę wczytanych do niej liczb. Funkcję tę zadeklaruj jako void drukuj(int tablica[], int liczba_elementow). W jej ciele ma być pętla for drukująca te elementy tablicy, które są większe od 10 i mniejsze od 100.
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void drukuj(int *numbers, int num_counted){
  printf("Wypisuję liczby większe od 10 i mniejsze od 100:\n");
  printf("[");
  bool first = true;
  for (int i = 0; i < num_counted; i++) {
    if (numbers[i] > 10 && numbers[i] < 100) {
      if (first) {
        printf("%d", numbers[i]);
        first = false;
      } else {
        printf(", %d", numbers[i]);
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
  drukuj(numbers, num_counted);

  return 0;
}
