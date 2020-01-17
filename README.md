# IPK - Počítačové komunikace a sítě - WebClient

### Zadání:

Vytvoření klientské síťové aplikace v prostředí UNIXu s využitím komunikačního rozhraní BSD sockets.

Vytvořte program (klient) s využitím rozhraní schránek (BSD sockets), který implementuje stažení zadaného objektu pomocí URL z WWW serveru s využitím HTTP protokolu do souboru uloženého v lokálním souborovém systému.
Vytvořte program v jazyce C/C++, který je  přeložitelný na studentském unixovém serveru eva.fit.vutbr.cz včetně funkčního Makefile souboru (program přeložitelný po zadání příkazu make). Program využívá spojovanou službu (protokol TCP). Jméno přeloženého programu klienta bude webclient. Program předpokládá jeden povinný parametr a to URL identifikující objekt, který bude uložen do lokáního souborového systému do aktuálního adresáře. Pokud v dotazu URL není uvedeno jméno souboru, obsah bude uložen do souboru index.html. Oznámení o chybách (stavové kódy 4xx, 5xx), které mohou nastat, bude vytištěno na standardní chybový výstup (stderr).

**Synopsis:** 

    webclient URL

**Příklad použití:**

    webclient http://www.fit.vutbr.cz

stáhne defaultní stránku a uloží do souboru index.html

    webclient http://www.fit.vutbr.cz:80/common/img/fit_logo_cz.gif
    
stáhne a uloží obrázek fit_logo_cz.gif do souboru v aktuálním adresáři
    
### Hodnocení: 

**5/5**