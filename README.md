Klaudia Kowalska 148184

Michał Zieliński 148064

Grupa L7

**Sprawozdanie z Projektu Zaliczeniowego na Przetwarzanie Rozproszone**

**Temat projektu:** Skrzaty zabójcy szczurów

**Opis:** Są dwa rodzaje procesów - S skrzatów i G gnomów. Gnomy ubiegają się o jedną z A agrafek i C celowników. Kombinują agrafki z celownikiem tworząc broń. Skrzaty ubiegają się o broń. Po zdobyciu broni zabijają szczury i zwracają agrafki i celowniki do puli.


1. **Struktury i zmienne:**
1. N\_BROWNIE - liczba skrzatów
1. N\_GNOME - liczba gnomów
1. N\_EYE - liczba agrafek
1. N\_GUNPOINT - liczba celowników
1. **Neyei** - liczba wolnych agrafek dla danego procesu i
1. **Ngunpointi** - liczba wolnych celowników dla danego procesu i
1. **Ngunsi** - liczba wolnych broni dla danego procesu i
1. **EyeRequestQueuei-** kolejka skojarzona z danym procesem opisująca kolejność dostępu do sekcji krytycznej, tutaj pobrania agrafki, na podstawie timestampu
1. **GPRequestQueuei-** kolejka skojarzona z danym procesem opisująca kolejność dostępu do sekcji krytycznej, tutaj pobrania celownika, na podstawie timestampu
1. **GunRequestQueuei-** kolejka skojarzona z danym procesem opisująca kolejność dostępu do sekcji krytycznej, tutaj pobrania broni, na podstawie timestampu
1. **tsi**- znacznik czasowy skojarzony z procesem i
1. **Stany**

Początkowym stanem jest FREE

1. FREE - nie ubiega się o dostęp do zasobów
1. WAITING\_FOR\_EYE - czekanie na pobranie agrafki z puli (dostęp do sekcji krytycznej)
1. WAITING\_FOR\_GUNPOINT -  czekanie na pobranie celownika z puli (dostęp do sekcji krytycznej)
1. WAITING\_FOR\_GUN - czekanie na broń (dostęp do sekcji krytycznej)
1. PRODUCING\_GUN - produkcja broni
1. KILLING\_RAT - zabijanie szczurów

1. **Wiadomości**

Każdemu żądaniu sekcji krytycznej nadawany jest znacznik czasu przy użyciu zegara logicznego Lamporta.

Znacznik czasu jest używany do określenia priorytetu żądań sekcji krytycznej. Mniejszy timestamp ma wyższy priorytet niż większy timestamp. Wykonanie żądania sekcji krytycznej odbywa się zawsze w kolejności ich znaczników czasowych

1. **REQ\_EYE -** żądanie dostępu do zasobu agrafka
1. **ACK\_EYE -** pozwolenie na wejście do sekcji krytycznej dla procesu żądającego zasobu agrafka
1. **REQ\_GP -** żądanie dostępu do zasobu celownik
1. **ACK\_GP -** pozwolenie na wejście do sekcji krytycznej dla procesu żądającego zasobu celownik
1. **REQ\_GUN -**  żądanie dostępu do zasobu broń
1. **ACK\_GUN -** pozwolenie na wejście do sekcji krytycznej dla procesu żądającego zasobu cel
1. **RELEASE\_GUN** - opuszczenie sekcji krytycznej, przerobienie zasobu broń na części składowe (agrafka i celownik)
1. **GUN\_PRODUCED** - opuszczenie sekcji krytycznej bez zwiększania jej wielkości
1. **Opis szczegółowy algorytmu dla procesu i:**

Istnieją dwa typy procesów - Skrzaty i Gnomy. Proces typu Gnom chcąc pobrać agrafkę i celownik (ubiegający się o wejście do sekcji krytycznej) wysyła do wszystkich pozostałych prośby REQ\_EYE(tsi, i) (agrafka) i/lub  REQ\_GP(tsi, i) (celownik) o dostęp. Prośba o dostęp jest umieszczana w kolejce EyeRequestQueuei (dla agrafek) lub GPRequestQueuei (dla celowników). Kiedy proces j otrzyma wiadomość z prośbą REQ\_EYE lub REQ\_GP zwraca wiadomość do procesu i-tego podbitą timestampem, jest to odpowiednio ACK\_EYE lub ACK\_GP. Następnie proces j-ty umieszcza w kolejce RequestQueuej prośbę od procesu i-tego. Po zebraniu ACKów z timestampem większym niż (tsi, i) oraz spełnieniu warunku, że na na odpowiednim miejscu kolejki RequestQueuei (wystarcza dla niego zasobów) jest REQ wysłany przez proces i-ty, proces wchodzi do sekcji krytycznej. W sekcji krytycznej odbywa się tworzenie broni przez losowy czas. Kiedy czas ten mija to proces i-ty wychodzi z sekcji krytycznej, usuwając swój własny REQ z początku kolejki oraz wysyłając do pozostałych procesów wiadomość GUN\_PRODUCED podbitą jego timestampem. Kiedy proces j-ty otrzymuje wiadomość GUN\_PRODUCED od procesu i-tego wówczas usuwa REQ od tego procesu z własnej kolejki oraz zwiększa lokalną zmienną reprezentującą ilość danego zasobu.

Analogicznie proces Skrzat, ubiega się o dostęp do zasobu GUN. Wysyła do wszystkich procesów wiadomość REQ\_GUN(tsi, i). Prośba o dostęp jest umieszczana w kolejce GunRequestQueuei . Kiedy proces j otrzyma wiadomość z prośbą REQ\_GUN zwraca wiadomość do procesu i-tego podbitą timestampem, jest to ACK\_GUN. Następnie proces j-ty umieszcza w kolejce RequestQueuej prośbę od procesu i-tego. Po zebraniu ACKów z timestampem większym niż (tsi, i) oraz spełnieniu warunku, że na na odpowiednim miejscu kolejki RequestQueuei (wystarcza dla niego zasobów) jest REQ wysłany przez proces i-ty, proces wchodzi do sekcji krytycznej. W sekcji krytycznej odbywa się zabijanie szczurów przez losowy czas. Kiedy czas ten mija to proces i-ty wychodzi z sekcji krytycznej, usuwając swój własny REQ z początku kolejki oraz wysyłając do pozostałych procesów wiadomość RELEASE\_GUN, podbitą jego timestampem. Kiedy proces j-ty otrzymuje wiadomość RELEASE\_GUN, od procesu i-tego wówczas usuwa REQ od tego procesu z własnej kolejki oraz zwiększa lokalną zmienną reprezentującą ilość danego zasobu (oprócz GUN).

***Proces Gnom:***

**1.** *FREE*: stan początkowy.

**2**. Proces i przebywa w stanie *FREE* do czasu, aż podejmie decyzję o ubieganie się o sekcję krytyczną. Ze stanu *FREE* następuje przejście do stanu *WAITING\_FOR\_EYE* po uprzednim wysłaniu wiadomości *REQ\_GUN* oraz stanu *WAITING\_FOR\_GP* po wysłaniu *REQ\_GP* do wszystkich innych procesów oraz ustawieniu EyeAckCounter oraz GPAckCounter na zero. Wszystkie wiadomości REQ są opisane tym samym priorytetem, równym zegarowi Lamporta w chwili wysłania pierwszej wiadomości REQ.

**3.** Reakcje na wiadomości:

a. REQ\_EYE : sprawdza lokalną kolejkę dostępu do zasobu, odsyła ACK\_EYE

b. REQ\_GP : sprawdza lokalną kolejkę dostępu do zasobu, odsyła ACK\_GP

c. ACK\_EYE: ignoruje

d. ACK\_GP: ignoruje

e. GUN\_PRODUCED: usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei

f. RELEASE\_GUN od procesu j : usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, zwiększa Neye, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei, zwiększa Ngunpoint

g. GUN\_PRODUCED: usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei

**4**. WAITING\_FOR\_EYE + WAITING\_FOR\_GP : ubieganie się o sekcję krytyczną.

**5**. Ze stanu  *WAITING\_FOR\_EYE + WAITING\_FOR\_GP* następuje przejście do stanu *PRODUCING\_GUN* pod warunkiem, że proces otrzyma *ACK\_EYE* oraz *ACK\_GP* od wszystkich innych procesów (EyeAckCounter == n - 1 && GPAckCounter == n - 1).

a. REQ\_EYE: od procesu j: odsyła ACK\_EYE i  REQ\_EYE zapamiętywany jest w kolejce EyeWaitQueuei

b. REQ\_GP: od procesu j: jeżeli priorytet zawarty w REQ\_GP jest większy odsyła ACK\_GP i REQ\_GP zapamiętywany jest w kolejce GPWaitQueuei

c.RELEASE\_GUN od procesu j : usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, zwiększa Neye, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei, zwiększa Ngunpoint

e. ACK: zwiększa licznik otrzymanych ACK (odpowiednio EyeAckCounter++ lub  GPAckCounter++).

f. GUN\_PRODUCED: usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei

Tak, jak opisano to wyżej, gdy otrzymano ACK\_EYE oraz ACK\_GP od

wszystkich pozostałych procesów z timestampem większym niż (ts i, i) orazREQ\_EYEi  jest jednym z Neye pierwszych procesów kolejki EyeRequestQueuei i REQ\_GPi  jest jednym z Ngunpoint pierwszych procesów kolejki GpRequestQueuei, proces i przechodzi do stanu *PRODUCING\_GUN.*

**6**. *PRODUCING\_GUN* : przebywanie w sekcji krytycznej.

**7**. Proces przebywa w sekcji krytycznej do czasu podjęcia decyzji o jej opuszczeniu. Po podjęciu decyzji o opuszczeniu sekcji, proces usuwa z własnej kolejki  EyeRequestQueueiżądanie REQ\_EYEi   i z kolejki  GpRequestQueuei żądanie REQ\_GPi , następnie wysyła GunProduced podbite znacznikiem czasowym, a następnie przechodzi do stanu FREE 

a. REQ\_EYE: dodaje żądanie do kolejki EyeWaitQueue

b. REQ\_GP: dodaje żądanie do kolejki GPWaitQueue

c. ACK\_EYE: Niemożliwe. Ignorowane

d. ACK\_GP: Niemożliwe. Ignorowane

***Proces Skrzat:***

**1.** *FREE*: stan początkowy.

**2**. Proces i przebywa w stanie *FREE* do czasu, aż podejmie decyzję o ubieganie się o sekcję krytyczną. Ze stanu *FREE* następuje przejście do stanu *WAITING\_FOR\_GUN* po uprzednim wysłaniu wiadomości *REQ\_GUN* do wszystkich innych procesów oraz ustawieniu GunAckCounter na zero. Wszystkie wiadomości REQ są opisane tym samym priorytetem, równym zegarowi Lamporta w chwili wysłania pierwszej wiadomości REQ.

**3.** Reakcje na wiadomości:

a. REQ\_EYE : odsyła ACK\_EYE

b. REQ\_GP : odsyła ACK\_GP

c. REQ\_GUN: sprawdza lokalną kolejkę dostępu do zasobu, odsyła ACK\_GUN

d. ACK\_EYE: ignoruje

e. ACK\_GP: ignoruje

f. ACK\_GUN: ignoruje

g. GUN\_PRODUCED: usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei, zwiększa lokalną zmienną reprezentującą ilość zasobu

h. RELEASE\_GUN od procesu j : usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, zwiększa Neye,  usuwa REQ\_GPj z własnej kolejki GPRequestQueuei, zwiększa Ngunpoint

**4**. WAITING\_FOR\_GUN : ubieganie się o sekcję krytyczną.

**5**. Ze stanu  WAITING\_FOR\_GUN  następuje przejście do stanu *KILLING\_RAT* pod warunkiem, że proces i-ty otrzyma *ACK\_GUN* od wszystkich innych procesów (GunAckCounter == n - 1) z timestampem większym niż (tsi, i) i jego własna prośba o dostęp REQ\_GUNi jest na szczycie kolejki GunRequestQueuei.

a. REQ\_GUN: od procesu j: sprawdza lokalną kolejkę dostępu do zasobu, odsyła ACK\_GUN, a REQ\_GUN zapamiętywany jest w kolejce GunWaitQueuei

b. RELEASE\_GUN od procesu j : usuwa REQ\_GUNj z własnej kolejki GunRequestQueuei, zwiększa Ngun

c. GUN\_PRODUCED: usuwa REQ\_EYEj z własnej kolejki EyeRequestQueuei, usuwa REQ\_GPj z własnej kolejki GPRequestQueuei, zwiększa lokalną zmienną reprezentującą ilość zasobu

d. ACK: zwiększa licznik otrzymanych ACK ( GunAckCounter++ )

Tak, jak opisano to wyżej, gdy otrzymano ACK\_GUN od

wszystkich pozostałych procesów z timestampem większym niż (ts i, i) orazREQ\_GUNi  jest jednym z Ngun pierwszych procesów kolejki GunRequestQueuei proces i przechodzi do stanu *KILLING\_RAT*

**5**. *KILLING\_RAT*: przebywanie w sekcji krytycznej.

**6**. Proces przebywa w sekcji krytycznej do czasu podjęcia decyzji o jej opuszczeniu. Po podjęciu decyzji o opuszczeniu sekcji, proces usuwa z własnej kolejki  GunRequestQueueiżądanie REQ\_GUNi   , następnie wysyła RELEASE\_GUNi podbite znacznikiem czasowym, a następnie przechodzi do stanu FREE

a. REQ\_EYE: dodaje żądanie do kolejki EyeWaitQueue

b. REQ\_GP: dodaje żądanie do kolejki GPWaitQueue

c. ACK\_EYE: Niemożliwe. Ignorowane

d. ACK\_GP: Niemożliwe. Ignorowane
