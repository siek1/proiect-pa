# Proiect PA – Utilitare pentru Fisiere si Directoare

## Descrierea proiectului

Proiectul a presupus crearea unor programe utilitare pentru lucrul cu directoare si fisiere, similare cu comenzile uzuale din Linux precum `ls`, `diff` sau `cd`.

https://site-pa.netlify.app/proiecte/director_fisiere/

Scopul a fost de a intelege si aplica concepte precum:
- Procesarea argumentelor din linia de comanda
- Manipularea directoarelor si fisierelor
- Utilizarea structurilor de date precum:
  - **stive**
  - **arbori**
  - **grafuri**
- Elemente de proiectarea algoritmilor

## Prezentarea task-urilor

### 1. Task 1 – Procesarea argumentelor din fisier
  - Am creat un sistem de parsare a argumentelor bazat pe un fisier input (`parseInFile`).
  - Argumentele sunt incarcate in structuri de tip argument, si pot fi de tip `a` (argument), `f` (flag) sau `o` (optiune).

### 2. Task 2 – Simulare comanda ls cu optiuni
  Am implementat functia `util_ls` pentru a simula comportamentul comenzii `ls`.
  Se pot activa urmatoarele optiuni prin argumente:
  - `d` sau `--directory` – afiseaza subdirectoarele
  - `h` sau `--human-readable` – afiseaza dimensiunile in format prietenos
  - `r` sau `--recursive` – parcurgere recursiva
  Argumentele sunt procesate cu functiile `generateArgsFromArgv` si `procArgv`, apoi interpretate pentru a activa functionalitatile relevante.

### 3. Task 3 – Simulare cd relativ intre directoare
 - Am implementat `util_cd` pentru a calcula calea relativa de la un director sursa la unul destinatie.
 - Sunt utilizate functii de split si reconstructie a cailor.
 - Se trateaza cazurile speciale: directoare identice, subdirectoare, sau drumuri diferite.

### 4. Task 4 – Simulare comanda diff intre fisiere
 - Am implementat functia `util_diff` pentru a compara doua fisiere linie cu linie si a evidentia diferentele.
 - Pentru a determina secventa optima de transformare a unui fisier in altul, am modelat problema ca un graf si am folosit o varianta a algoritmului lui `Dijkstra`.
 - O `coada de prioritati` (heap) a fost folosita pentru a explora starile in ordinea costului minim.
 - In final, calea cu cost minim este interpretata si afisata ca o lista de diferente, similar cu comanda `diff`.

## Cum se utilizeaza?
1. Compilati sursa cu fisierul `Makefile`
```
make
```
2. Rulati executabilul `checker`

Checkerul permite verificarea corectitudinii si conformitatii solutiilor din proiect. Scopul sau este sa ruleze testele predefinite pe solutiile voastre si sa genereze rapoarte detaliate despre rezultate.

```
./checker-linux-amd64 -i
```
