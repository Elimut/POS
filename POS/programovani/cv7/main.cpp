#include <iostream>
#include <string>
#include <windows.h>

#define BUFSIZE 1000

using namespace std;

int main(int argc, char *argv[])
{
    WORD wVersionRequested = MAKEWORD(1,1); // Číslo verze
    WSADATA data;                           // Struktura s info. o knihovně
    hostent *host;                          // Vzdálený počítač
    sockaddr_in serverSock;                 // Vzdálený "konec potrubí"
    int mySocket;                           // Soket
    int port;                               // Číslo portu
    char buf[BUFSIZE];                      // Přijímací buffer
    int size;                             // Počet přijatých a odeslaných bytů
    if (argc != 3)
    {
        cerr << "Syntaxe:\n\t" << argv[0]
             << " " << "adresa port" << endl;
        return -1;
    }
	cout << "******** Windows TCP Klient ********" << endl;
	while(true) {
		// Připravíme soket na práci
		if (WSAStartup(wVersionRequested, &data) != 0)
		{
			cout << "Nepodařilo se inicializovat sokety" << endl;
			// Podle všeho, zde se WSACleanup volat nemusí.
			return -1;
		}
		port = atoi(argv[2]);
		// Zjistíme info o vzdáleném počítači
		if ((host = gethostbyname(argv[1])) == NULL)
		{
			cerr << "Špatná adresa" << endl;
			WSACleanup();
			return -1;
		}
		// Vytvoříme soket
		if ((mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		{
			cerr << "Nelze vytvořit soket" << endl;
			WSACleanup();
			return -1;
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
			cerr << "Nelze navazat spojeni" << endl;
			WSACleanup();
			return -1;
		}

		cout << "------------------------------------" << endl;
		cout << "Zadejte zpravu: ";
		string text = "";
		getline(cin, text);
		if(strcmp(text.c_str(), "konec") == 0)
			break;
		//cout << " " << endl;
		text.append("\n");

		// Odeslani dat
		if ((size = send(mySocket, text.c_str(), 
			text.size() + 1, 0)) == -1) {
			cerr << "Problem s odeslanim dat" << endl;
			WSACleanup();
			return -1;
		}

		// Prijem dat
		while (((size = recv(mySocket, buf, BUFSIZE, 0)) != 0) 
			&& (size != -1)) {
			cout << "------------------------------------" << endl;
			cout << "Dosavadni konverzace: " << '\n' << endl;
			text += buf;
			cout << text;
			break;
		}
		if (size == -1)
			cout << "Nelze prijmout data" << endl;

		text = "";

		// Uzavru spojeni
		closesocket(mySocket);
		WSACleanup();
	}
	cout << "------------------------------------" << endl;
	cout << "Konec." << endl;
	cout << "************************************" << endl;
    return 0;
}