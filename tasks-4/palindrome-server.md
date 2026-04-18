# Serwer TCP palindromów — jak to działa

Dokument opisuje implementację w pliku `palindrome-server.cpp`: serwer TCP/IPv4 na porcie **2020**, obsługa wielu klientów równocześnie dzięki **epoll** i gniazdom **non-blocking**.

---

## 0. Epoll — mechanizm (dlaczego i jak)

### Problem bez epoll

Gdybyś w pętli robił `blocking read()` na jednym gnieździe, **inne klienty czekałyby**, dopóki ten pierwszy coś nie wyśle. Wątek na każdego klienta rozwiązuje to kosztownie (pamięć, przełączanie kontekstu). **I/O wielokrotny w jednym wątku** wymaga gniazd **nieblokujących** i sposobu, by **nie kręcić się w kółko** (`while` z `read` na wszystkich FD) — potrzebujesz powiadomień z jądra: „ten deskryptor jest teraz gotowy”.

### Czym jest epoll (Linux)

**epoll** to interfejs jądra Linux do **multiplexingu I/O**: rejestrujesz zestaw deskryptorów (gniazd), a potem **blokujesz** w `epoll_wait()` aż **któryś** z nich będzie gotowy do odczytu lub zapisu (albo wystąpi błąd/rozłączenie). Po przebudzeniu dostajesz **listę zdarzeń** — wiesz, **które FD** obsłużyć w tej iteracji.

W skrócie:

| Funkcja | Rola |
|--------|------|
| `epoll_create1()` | Tworzy **obiekt epoll** (wewnętrzna kolejka zdarzeń); zwraca deskryptor `epfd`. |
| `epoll_ctl(EPOLL_CTL_ADD, ...)` | **Podłącza** gniazdo do epoll: „informuj mnie o zdarzeniach na tym FD”. |
| `epoll_ctl(EPOLL_CTL_MOD, ...)` | **Zmienia** maskę zdarzeń (np. dodajesz `EPOLLOUT` gdy masz co wysłać). |
| `epoll_ctl(EPOLL_CTL_DEL, ...)` | **Odłącza** FD od epoll (np. przed `close`). |
| `epoll_wait(epfd, tablica, max, timeout)` | **Czeka** na zdarzenia; wypełnia tablicę struktur `epoll_event` (m.in. `data.fd`, `events`). |

### Maski zdarzeń używane w tym serwerze

- **`EPOLLIN`** — można **czytać** bez blokowania (dla nasłuchu: jest **co zaakceptować**; dla klienta: są **dane w buforze** lub peer zamknął połączenie — wtedy `read` zwróci 0).
- **`EPOLLOUT`** — bufor wyjściowy jądra ma **miejsce** — można **pisać** bez blokowania (albo dostaniesz `EAGAIN`).
- **`EPOLLERR` / `EPOLLHUP`** — błąd na gnieździe lub „hang up” — w kodzie zamykasz połączenie.

### Dlaczego non-blocking (`O_NONBLOCK`)

Gdy epoll mówi „`EPOLLIN` na `client_fd`”, **nie wiadomo ile** bajtów jest w buforze. W pętli wołasz `read()` **dopóki** nie dostaniesz `EAGAIN` — wtedy opróżniłeś bufor jądra dla tego momentu i **nie blokujesz** innych klientów. To samo przy `write()` i `EPOLLOUT`: piszesz w pętli aż `EAGAIN` (kolejna porcja przy następnym obudzeniu).

### Przepływ w tej aplikacji (jedna iteracja pętli)

```text
epoll_wait()  →  lista „gotowych” FD
     │
     ├─ server_fd + EPOLLIN  →  pętla accept() (non-blocking) aż EAGAIN; każdy nowy klient: ADD do epoll, EPOLLIN
     │
     └─ client_fd
            ├─ EPOLLERR|EPOLLHUP  →  DEL, close, usuń z mapy
            ├─ EPOLLIN  →  read w pętli → bufor input → linie \r\n → process() → output;
            │              jeśli output niepusty: MOD na EPOLLIN|EPOLLOUT
            └─ EPOLLOUT →  write w pętli → skróć output; jeśli pusto: MOD z powrotem na samo EPOLLIN
```

**Jeden wątek** obsługuje wielu klientów **naprzemiennie**: epoll wybiera, kto jest „na kolejkę”, a krótkie pętle read/write bez blokowania gwarantują sprawiedliwość i brak zajęcia CPU na pustym kręceniu.

### epoll a select/poll

`select`/`poll` skalują się gorzej przy dużej liczbie FD (koszt rośnie z liczbą deskryptorów). **epoll** jest zaprojektowany tak, by koszt **nie zależał liniowo** od liczby obserwowanych gniazd (w praktyce używa się go w serwerach wysokiego obciążenia na Linuxie).

---

## 1. Protokół (warstwa logiczna)

- Połączenie jest **strumieniowe** (TCP). Klient może wysłać wiele linii w jednym połączeniu.
- **Linia** to ciąg drukowalnych znaków ASCII (może być pusty), zakończony terminatorem **`\r\n`** (CRLF).
- **Zapytanie (linia od klienta):** tylko litery (A–Z, a–z) i spacje jako separatory słów. Te same zasady co w wersji UDP z zadania: pusta linia (zero słów) jest poprawna, wielkie i małe litery są traktowane tak samo przy sprawdzaniu palindromu, brak spacji na początku/końcu, brak podwójnych spacji między słowami.
- **Odpowiedź serwera:** albo `palindromy/wszystkie` + `\r\n` (dwa niepuste ciągi cyfr oddzielone `/`), albo `ERROR` + `\r\n`.

Przykłady:

| Zapytanie (treść linii) | Odpowiedź |
|-------------------------|-----------|
| *(pusta)* | `0/0` |
| `kajak ala` | `2/2` |
| `hello 1` | `ERROR` |

---

## 2. Architektura serwera

### Dlaczego epoll?

Jeden proces i jeden wątek obsługuje **wiele połączeń**. Zamiast osobnego wątku na klienta, jądro informuje nas (przez `epoll_wait`), które **deskryptory plików** są gotowe do odczytu lub zapisu.

### Non-blocking

Gniazdo nasłuchujące i gniazda klientów mają flagę **`O_NONBLOCK`**. Wywołania `read()` / `write()` / `accept()` nie blokują w nieskończoność: jeśli nie ma danych, zwracają `-1` z `errno == EAGAIN` (lub `EWOULDBLOCK`).

Dzięki temu w jednej iteracji pętli można:

- odebrać wszystko, co jest już w buforze jądra (pętla `read()` aż do `EAGAIN`),
- wysłać tyle, ile się da (pętla `write()` aż do `EAGAIN`).

---

## 3. Struktury danych

```text
map<int, ClientData> clients
```

- **Klucz:** deskryptor gniazda klienta (`int`).
- **`ClientData`:**
  - **`input`** — bufor odebranych bajtów jeszcze **nie** złożonych w kompletne linie (TCP nie gwarantuje „jedna linia = jeden read”).
  - **`output`** — kolejka odpowiedzi do wysłania (może być kilka linii naraz).

---

## 4. Uruchomienie gniazda nasłuchującego

1. `socket(AF_INET, SOCK_STREAM, 0)` — gniazdo TCP IPv4.
2. `setsockopt(..., SO_REUSEADDR, ...)` — ułatwia ponowne uruchomienie serwera zaraz po zamknięciu (unikanie stanu `TIME_WAIT` blokującego port).
3. `bind()` na `INADDR_ANY` i porcie `2020`.
4. `listen()` — kolejka połączeń przychodzących.
5. `fcntl(..., O_NONBLOCK)` — nasłuch nie blokuje na `accept()`.
6. `epoll_create1()` + `epoll_ctl(EPOLL_CTL_ADD, server_fd, EPOLLIN)` — czekamy na możliwość **zaakceptowania** nowego klienta.

---

## 5. Pętla główna: `epoll_wait`

```text
while (true) {
    n = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for każde zdarzenie:
        ...
}
```

- **`MAX_EVENTS` (64):** w jednym wywołaniu epoll może wrócić co najwyżej tyle zdarzeń. Przy większej liczbie gotowych FD epoll zwróci kolejne partie w następnych iteracjach.

Dla każdego zdarzenia znany jest **`fd`** z `events[i].data.fd`.

---

## 6. Nowy klient: `fd == server_fd`

Gdy epoll sygnalizuje `EPOLLIN` na gnieździe nasłuchującym, znaczy to zwykle: **jest co najmniej jedno oczekujące połączenie**.

W pętli:

- `accept()` — tworzy nowy deskryptor klienta.
- Jeśli `accept` zwraca błąd inny niż `EAGAIN`, wychodzimy z pętli (np. brak więcej połączeń w kolejce).
- Nowe gniazdo: znowu **non-blocking**, `epoll_ctl(ADD, EPOLLIN)`, wpis w `clients`.

---

## 7. Błędy i rozłączenie: `EPOLLERR | EPOLLHUP`

Jeśli na gnieździe klienta jest błąd lub peer zamknął połączenie w sposób widoczny jako hang-up, usuwamy FD z epoll, zamykamy gniazdo i kasujemy wpis w `clients`.

---

## 8. Odczyt: `EPOLLIN` (klient)

### 8.1. Zrzut danych z jądra

W pętli `read(fd, buf, sizeof(buf))`:

- **`r > 0`:** dopisujemy bajty do `clients[fd].input`.
- **`r == 0`:** druga strona zamknęła połączenie (EOF) — traktujemy jak rozłączenie.
- **`r < 0` i `errno == EAGAIN`:** w buforze jądra nie ma już więcej danych do odczytu bez blokowania — **koniec** pętli odczytu.
- **`r < 0` i inny błąd:** rozłączenie / błąd.

To jest **niezbędne** przy TCP: jedna linia może przyjść w wielu `read()`, albo kilka linii w jednym `read()`.

### 8.2. Wyodrębnianie linii (`\r\n`)

W pętli:

1. Szukamy w `input` podciągu `"\r\n"`.
2. Jeśli **nie ma** jeszcze końca linii:
   - jeśli `input.size() > MAX_LINE` (1024), uznajemy linię za zbyt długą i zamykamy połączenie (zgodnie z ideą limitu długości linii).
   - w przeciwnym razie czekamy na kolejne dane w następnym `EPOLLIN`.
3. Jeśli **jest** `pos`:
   - `line = input.substr(0, pos)` — treść linii **bez** terminatora.
   - `input.erase(0, pos + …)` — usuwanie przetworzonej części z bufora (w kodzie użyta jest wartość `pos + 1`; patrz sekcja 11).

### 8.3. Warstwa „surowa” (drukowalne ASCII)

Dla znaków w `line` (już bez `\r\n`): jeśli którykolwiek jest poza zakresem drukowalnego ASCII (`0x20`–`0x7e`), serwer **zamyka połączenie** (dane nienaturalne / binarne).

### 8.4. Logika zadania: `process(line)`

Wywołanie `process(line)` zwraca gotowy łańcuch z **terminatorem** `\r\n` i jest dopisywane do `output`.

### 8.5. Przełączenie na zapis

Jeśli po przetworzeniu `output` nie jest pusty, modyfikujemy epoll: **`EPOLLIN | EPOLLOUT`**, żeby przy następnej okazji wysłać odpowiedź.

---

## 9. Zapis: `EPOLLOUT`

- Dopóki `output` nie jest pusty, wywołujemy `write()`.
- Jeśli `write` zwraca `EAGAIN`, przerywamy pętlę — jądro bufor wyjściowy jest pełny; wrócimy przy kolejnym `EPOLLOUT`.
- Po pełnym wysłaniu bufora wracamy do **`EPOLLIN`**, żeby nie budzić się na zapis bez potrzeby.

---

## 10. Funkcje `process` i `check_palindrome`

### `check_palindrome(word)`

- Kopiuje słowo, zamienia znaki na małe (`tolower`), porównuje symetrycznie od końców do środka — jeśli się zgadza, słowo jest palindromem **bez rozróżniania wielkości liter**.

### `process(line)`

1. Każdy znak musi być spacją lub literą — inaczej `ERROR\r\n`.
2. Pusta linia → `0/0\r\n`.
3. Spacja na początku lub końcu → `ERROR\r\n`.
4. Dwie spacje obok siebie → `ERROR\r\n`.
5. Dzieli linię na słowa (spacje jako separatory), liczy słowa i palindromy, zwraca `palindromy/wszystkie\r\n`.

---

## 11. Uwaga do bufora po `erase` (implementacja)

Po znalezieniu `pos` pozycji `"\r\n"` kod usuwa z początku `input` **`pos + 1`** bajtów zamiast **`pos + 2`**. Skutkuje to pozostawieniem jednego znaku terminatora (np. `'\n'`) w buforze. Kolejna „linia” może wtedy zawierać niedrukowalny znak i zostać uznana za niepoprawną na warstwie surowej — zachowanie wielu zapytań w jednym połączeniu może być wtedy niezgodne z oczekiwaniami. W poprawnej implementacji należałoby usuwać **`pos + 2`** bajty (cały terminator `\r\n`).

---

## 12. Jak przetestować

**Terminal 1 — serwer:**

```bash
cd tasks-4 && make && build/palindrome-server
```

**Terminal 2 — pojedyncze zapytanie:**

```bash
echo -ne "kajak ala test\r\n" | nc -w 1 127.0.0.1 2020
```

Oczekiwany przykład: `2/3` (plus ewentualnie widoczne zakończenia linii w zależności od `nc`).

**Interaktywnie:**

```bash
nc 127.0.0.1 2020
```

Wpisywanie linii zakończonych Enterem zależy od klienta — wiele wersji `netcat` wysyła tylko `\n`; do ścisłego `\r\n` wygodniej użyć `echo -ne '...\r\n'`.

---

## 13. Zależności i kompilacja

- Linux z obsługą **epoll**.
- Kompilacja (Makefile): `g++ -std=c++17 -pedantic -Wall`.

---

## Skrót przepływu danych

```text
Klient --TCP--> read() --> clients[fd].input
                              |
                    szukanie "\r\n"
                              |
                         process(line)
                              |
                    clients[fd].output
                              |
                         write() --> Klient
```

Całość jest **jednowątkowa**; „równoległość” wynika z tego, że epoll przełącza obsługę między wieloma deskryptorami w krótkich fragmentach czasu.
