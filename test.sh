#!/bin/ksh
make
echo "URL: http://www.fit.vutbr.cz:80/common/img/fit_logo_cz.gif"
echo "Stahne obrazek fit_logo_cz.gif do aktualniho adresare."
./webclient http://www.fit.vutbr.cz:80/common/img/fit_logo_cz.gif
echo "URL: http://www.oskar.cz"
echo "Vicenasobne presmerovani. Stahne soubor index.htm do aktualniho adresare."
./webclient http://www.oskar.cz
echo "URL: http://www.gooogle.com"
echo "Vicenasobne presmerovani. Ulozi stranku do souboru index.html v aktualnim adresari."
./webclient http://www.gooogle.com
echo "URL: http://www.rockware.co.uk"
echo "Presmerovani bez Location. Zahlasi chybu."
./webclient http://www.rockware.co.uk
echo "URL: http://en.kioskea.net/contents/internet"
echo "Presmerovani s relativni adresou. Stahne soubor internet.php3 do aktualniho adresare."
./webclient http://en.kioskea.net/contents/internet
echo "URL: http://www.fit.vutbr.cz/neco"
echo "Neexistujici stranka, vypise chybu 404."
./webclient http://www.fit.vutbr.cz/neco
echo "URL: http://www..fit.vutbr.cz"
echo "Nespravny tvar URL, vypise chybu gethostbyname."
./webclient http://www..fit.vutbr.cz/








 
