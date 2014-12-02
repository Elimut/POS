#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1000
#define CRLF "\r\n"

using namespace std;

int main(int argc, char *argv[])
{
    string text("GET /~drymld/ps/ HTTP/1.1" CRLF
                "Host: phoenix.inf.upol.cz" CRLF
                "Connection: close" CRLF); // Odesílaný a přijímaný text
    hostent *host; // Vzdálený počítač;
    sockaddr_in serverSock; // Vzdálený "konec potrubí"
    int mySocket; // Soket
    int port; // Číslo portu
    char buf[BUFSIZE]; // Přijímací buffer
    int size; // Počet přijatých a odeslaných bytů
    if (argc != 3)
    {
        cerr << "Syntaxe:\n\t" << argv[0]
                  << " " << "adresa port" << endl;
        return 1;
    }
    port = atoi(argv[2]);

    // Zjistíme info o vzdáleném počítači
    if ((host = gethostbyname(argv[1])) == NULL)
    {
        cerr << "2: Špatná adresa" << endl;
        return 2;
    }
    // Vytvoříme soket
    if ((mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        cerr << "3: Nelze vytvořit soket" << endl;//TODO zkusit errno a strerror()
        return 3;
    }
    // Zaplníme strukturu sockaddr_in

    // 1) Rodina protokolů
    serverSock.sin_family = AF_INET;

    // 2) Číslo portu, ke kterému se připojíme
    serverSock.sin_port = htons(port);

    // 3) Nastavení IP adresy, ke které se připojíme
    memcpy(&(serverSock.sin_addr), host->h_addr, host->h_length);
    // Připojení soketu
    if (connect(mySocket, (sockaddr *)&serverSock, sizeof(serverSock)) == -1)
    {
        cerr << "3: Nelze navázat spojení" << endl;
        return 4;
    }
    // Odeslání dat
    if ((size = send(mySocket, text.c_str(), text.size() + 1, 0)) == -1)
    {
        cerr << "5: Problém s odesláním dat" << endl;
        return 5;
    }
    cout << "Odesláno " << size << endl;

    // Příjem dat
    text = "";
    while ((size = recv(mySocket, buf, BUFSIZE, 0)) != -1)
    {
        cout << "Prijato " << size << endl;
        text += buf;
        break;
    }
    int delka_hlavicky=strstr(buf, "\r\n\r\n")-buf;

    // Uzavřu spojení
    close(mySocket);
    cout << "delka_hlavicky: " << delka_hlavicky << endl << text << endl;
    return EXIT_SUCCESS;
}
