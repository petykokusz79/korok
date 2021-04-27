## Körök
**Ezen projektem során egy gráf adott csúcsából kiinduló és oda visszatérő, minden élen legfeljebb egyszer áthaladó összes útvonal növekvő hossz szerinti visszaadására próbálok minél hatékonyabb algoritmust írni.**

---

#### A gráf fontos jellemzői:
- egyszerű (többszörös- és hurokélektől mentes)
- irányítatlan
- súlyozott
- összefüggő
- a csúcsok jelölése 1-től számozva történik

#### A bemenet felépítése:
- az első sorban a csúcsok `N` száma és az élek `M` száma szerepel, szóközzel elválasztva
  - a jelenlegi (és feltehetően végleges) gráfra igaz, hogy `N`<`M`<256
- a következő `M` sorban egy-egy él `K` kezdőpontja, `V` végpontja, illetve `H` hossza szerepel, szóközzel elválasztva
  - minden élre igaz, hogy `K`>`V`, illetve `H`<600

#### Egyéb jellemzők:
- a kiinduló csúcs az 1-es számú, szomszédjai a 2-es és a 3-mas számú csúcsok
- egy útvonal mindig a 2-es számú csúcsot érinti először
- létezik a feltételeknek megfelelő útvonal
- az összes útvonal visszaadása a gyakorlatban nem megoldható, így a cél *minél több* eredmény kiszámítása