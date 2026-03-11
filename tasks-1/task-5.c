/*
Polecenie:
W dokumentacji POSIX API znajdź opisy czterech podstawowych funkcji plikowego wejścia-wyjścia, tzn. open, read, write i close. Czy zgadzają się one z tym, co pamiętasz z przedmiotu „Systemy operacyjne"? Jakie znaczenie ma wartość 0 zwrócona jako wynik funkcji read?

---

Odpowiedź:

Poniżej znajdują się krótkie opisy czterech podstawowych funkcji plikowego wejścia-wyjścia według dokumentacji POSIX API:

1. open - Funkcja open() służy do otwarcia pliku lub urządzenia. Zwraca deskryptor pliku (liczbę całkowitą >=0 przy sukcesie, -1 przy błędzie). Argumenty pozwalają określić sposób otwarcia (np. tylko do odczytu, do zapisu itp.).

2. read - Funkcja read() służy do odczytywania danych z pliku (deskryptora pliku) do bufora w pamięci. Zwraca liczbę bajtów faktycznie odczytanych (lub 0, jeśli napotkany został koniec pliku, lub -1 w przypadku błędu).

3. write - Funkcja write() umożliwia zapisanie danych z bufora do pliku (związanego z deskryptorem pliku). Zwraca liczbę zapisanych bajtów (lub -1 przy błędzie).

4. close - Funkcja close() zamyka deskryptor pliku i zwalnia przypisane do niego zasoby.

Dokładnie takie funkcje były omawiane na zajęciach z „Systemów operacyjnych”, zbliżone są także ich sygnatury i rola.

Znaczenie wartości 0 zwróconej przez read:
Wartość 0 oznacza, że osiągnięto koniec pliku (EOF) – nie ma więcej danych do odczytania z pliku lub urządzenia. Zazwyczaj używane w pętlach czytających dane do wykrycia EOF.

*/

#include <stdio.h>

int main() {
    return 0;
}