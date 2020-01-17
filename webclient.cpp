#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <iterator>

using namespace std;

//ziska z hlavicky statuscode i s nazvem
string statusCode(const string& header)
{
    string code;
    int i = 0;

    while (header[i] != ' ' && header[i] != '\0')
    {
        i++;
    }

    //hlavicka nebyla nalezena, stahne se cely soubor
    if (header[i] == '\0') return "200 OK";

    for (i = i + 1; header[i] != '\n'; ++i)
    {
        code += header[i];
    }

    return code;
}

//rozdeli url na hostname, cestu k souboru, nazev souboru a port
//pokud neni cesta nebo soubor specifikovana, bude v techto promennych prazdny retezec
//nekontroluje spravnost tvaru adresy
void parse(const string& url, string& name, string& path, string& file, int& port)
{
    enum State { FIRST_CHAR, HOSTNAME, PORT, PATH, END };

    port = 0;
    int begin_name = 0;   //index, na kterem zacina hostname
    int end_name = 0;     //index, na kterem konci hostname
    int begin_path = 0;   //index, na kterem zacina cesta k souboru
    int begin_file = 0;   //index, na kterem zacina jmeno souboru
    int i = 0;
    State state = FIRST_CHAR;

    file = "";
    path = "";

    //http://www.jmeno.cz:port/cesta/soubor - priklad url, ktere ma vsechny casti
    while (state != END) //koncovy stav
    {
        switch (state)
        {
        case FIRST_CHAR: //prvni znak
            if ((url[i] == 'h' || url[i] == 'H') &&
                (url[i + 1] == 't' || url[i + 1] == 'T') &&
                (url[i + 2] == 't' || url[i + 2] == 'T') &&
                (url[i + 3] == 'p' || url[i + 3] == 'P') &&
                (url[i + 4] == ':') && (url[i + 5] == '/') && (url[i + 6] == '/'))
            {
                i = i + 7;
                begin_name = i;   //tady zacina hostname
                state = HOSTNAME; //jsme za http://, nasleduje hostname		
            }
            else if (url[i] == '\0')
            {
                state = END;      //konec retezce
            }
            else
            {
                begin_name = i;   //tady zacina hostname
                state = HOSTNAME; //neni tu http://, nasleduje hostname 
            }
            break;
        case HOSTNAME: //www.jmeno.cz = hostname
            if (url[i] == ':')
            {
                end_name = i - 1; //tady konci hostname
                i++;
                state = PORT;     //nasleduje port
            }
            else if (url[i] == '/')
            {
                end_name = i - 1;   //tady konci hostname
                i++;
                if (url[i] != '\0')
                {
                    begin_path = i; //tady zacina cesta k souboru
                    begin_file = i; //tady by mohlo zacinat jmeno souboru
                }
                state = PATH;       //nasleduje cesta/soubor
            }
            else if (url[i] == '\0')
            {
                end_name = i - 1; //tady konci hostname
                state = END;      //konec retezce
            }
            else
            {
                i++;
                state = HOSTNAME; //pokracuje hostname
            }
            break;
        case PORT: //port
            if (url[i] == '/')
            {
                i++;
                if (url[i] != '\0')
                {
                    begin_path = i; //tady zacina cesta k souboru
                    begin_file = i; //tady by mohlo zacinat jmeno souboru
                }
                state = PATH;       //nasleduje cesta/soubor
            }
            else if (url[i] == '\0')
            {
                state = END; //konec retezce
            }
            else
            {
                //prevod na cislo
                port *= 10;
                port += url[i] - '0';
                i++;
                state = PORT; //port pokracuje
            }
            break;
        case PATH: //cesta/soubor
            if (url[i] == '/')
            {
                i++;
                if (url[i] != '\0')
                {
                    begin_file = i; //tady by mohlo zacinat jmeno souboru
                }
                state = PATH;       //cesta/soubor pokracuje
            }
            else if (url[i] == '\0')
            {
                state = END; //konec retezce
            }
            else
            {
                i++;
                state = PATH; //cesta/soubor pokracuje
            }
            break;
        }
    }

    //rozkouskujeme vstupni adresu na 3 casti
    name = url.substr(begin_name, end_name - begin_name + 1);

    if (begin_path != 0)
    {
        path = url.substr(begin_path, url.size() - begin_path);
    }

    bool sb = false; //urcuje, zda bylo zadano jmeno souboru 

    if (begin_file != 0)
    {
        for (i = 0; i < url.size() - begin_file; ++i)
        {
            if (url[begin_file + i] == '.' && url[begin_file + i + 1] != '\0')
            {
                sb = true;
            }
        }
        if (sb) file = url.substr(begin_file, url.size() - begin_file);
    }
}

//zjisti z hlavicky novou adresu pro presmerovani
//pokud neni nalezena, vrati prazdny retezec
string redirectURL(const string& header)
{
    int i = 0;
    string url = "";
    string divider = "\r\n\r\n";
    string searched = "Location: ";

    //zjisti index prvniho znaku v nove url
    while ((header.substr(i, divider.size()) != divider))
    {
        if (header.substr(i, searched.size()) == searched)
        {
            i += searched.size();
            break;
        }
        i++;
    }

    //nacte novou url
    if ((header.substr(i, divider.size()) != divider))
    {
        while (header[i + 1] != '\n') //pred koncem radku je jeste nejaky znak
        {
            url += header[i];
            i++;
        }
    }

    return url;
}

//stazeni pozadovaneho objektu z webu
//pri chybe vraci -1
int download(string& name, string& path, int& port, string& response, string& file)
{
    int size, s;
    char buffer[1000];
    struct sockaddr_in sin;
    struct hostent* hptr;
    response = "";
    string request = "GET /" + path + " HTTP/1.0\r\nhost: " + name + "\r\n\r\n";

    //vytvori socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }

    sin.sin_family = PF_INET;   //rodina protokolu
    if (port == 0) port = 80;   //defaultni cislo portu, pokud nebylo zadano
    sin.sin_port = htons(port); //cislo portu

    //zjistime info o vzdalenem pocitaci
    if ((hptr = gethostbyname(name.c_str())) == nullptr)
    {
        fprintf(stderr, "gethostname error\n");
        return -1;
    }

    //nastaveni IP adresy, ke ktere se pripojime
    memcpy(&sin.sin_addr, hptr->h_addr, hptr->h_length);

    //navazani spojeni
    if (connect(s, (struct sockaddr*) & sin, sizeof(sin)) < 0)
    {
        perror("connect");
        return -1;
    }

    //poslani pozadavku
    if (write(s, request.c_str(), request.size() + 1) < 0)
    {
        perror("write");
        return -1;
    }

    //prijmuti odpovedi
    while ((size = read(s, buffer, sizeof(buffer) - 1)) != 0)
    {
        if (size == -1)
        {
            perror("read");
            return -1;
        }
        for (int i = 0; i < size; ++i)
        {
            response += buffer[i];
        }
    }

    //uzavreni spojeni
    if (close(s) < 0)
    {
        perror("close");
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Neni zadana URL.\n");
        return -1;
    }

    string url = argv[1];
    string response, file, name, path;
    bool urlOK;  //kdyz je false, je potreba ziskat dalsi url kvuli presmerovani
    int port, count = 0;

    parse(url, name, path, file, port);

    do
    {
        if (download(name, path, port, response, file) == -1)
            return -1;
        string code = statusCode(response);
        switch (code[0])
        {
        case '1':
        case '2':
            //vse ok
            urlOK = true;
            break;
        case '3':
            //presmerovani
            count++;
            if (count > 5)
            {
                fprintf(stderr, "Chyba: Prilis mnoho presmerovani.\n");
                return -1;
            }
            url = redirectURL(response);
            if (url == "")
            {
                //nove url nebylo nalezeno
                fprintf(stderr, "Chyba: URL pro presmerovani nenalezeno.\n");
                return -1;
            }
            else if (url[0] == '/')
            {
                //relativni url
                url = name + url;
                parse(url, name, path, file, port);
            }
            else
            {
                //normalni url
                parse(url, name, path, file, port);
            }
            urlOK = false;
            break;
        case '4':
        case '5':
            //chyba
            fprintf(stderr, "%s\n", code.c_str());
            return -1;
            break;
        }
    } while (urlOK == false);

    string divider = "\r\n\r\n";
    int offset = response.find(divider); //zjistime, kde konci hlavicka a zacina vlastni obsah

    if (file == "")
    {
        //nebylo zadano jmeno souboru
        file = "index.html";
    }

    //zapis do souboru
    ofstream file;
    file.open(file.c_str());
    ostream_iterator<char> out(file, "");
    copy(response.begin() + offset + divider.size(), response.end(), out);
    file.close();

    return 0;
}
