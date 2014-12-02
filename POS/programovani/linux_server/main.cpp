#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>


#define BUFSIZE 1000

using namespace std;

int main(int argc, char *argv[])
{
   std::string text;             // Prijmany text
   sockaddr_in sockName;         // "Jmeno" portu
   sockaddr_in clientInfo;       // Klient, ktery se pripojil
   int mainSocket;               // Soket
   int port;                     // Cislo portu
   char buf[BUFSIZE];            // Prijmaci buffer
   int size;                     // Pocet prijmanych a odeslanych bytu
   socklen_t addrlen;            // Velikost adresy vzdaleneho pocitace
   int count = 0;                // Pocet pripojeni
   bool odpojen = false;

   if (argc != 2)
   {
     cerr << "Syntaxe:\n\t" << argv[0]
	  << " " << "port" << endl;
     return -1;
   }
   port = atoi(argv[1]);
   // Vytvorime soket - viz minuly dil
   if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
   {
     cerr << "Nelze vytvorit soket" << endl;
     return -1;
   }
   // Zaplnime strukturu  sockaddr_in
   // 1) Rodina protokolu
   sockName.sin_family = AF_INET;
   // 2) cislo portu, na kterem cekame
   sockName.sin_port = htons(port);
   // 3) Nastavení IP adresy
   sockName.sin_addr.s_addr = INADDR_ANY;
   // priradime soketu jmeno
   if (bind(mainSocket, (sockaddr *)&sockName, sizeof(sockName)) == -1)
   {
     cerr << "Problem s pojmenovanim soketu." << endl;
     return -1;
   }
   // Vytvorime frontu pozadavku na spojeni
   // Vytvorime frontu o 10 prvcich
   if (listen(mainSocket, 20) == -1) {
     cerr << "Problem s vytvorenim fronty" << endl;
     return -1;
   }
   // nastav defaultne text
   text = "";
   int totalSize = 0;
   cout << "******** Linux TCP Server ********" << endl;
   cout << "----------------------------------" << endl;
   cout << "Inicializace servru probehla bez problemu." << endl;
   do
   {
       odpojen = false;
     // Poznacim si velikost struktury clientInfo.
     // Predam to funkci accept.
     addrlen = sizeof(clientInfo);
     // Vyberu z fronty pozadavek na spojeni
     // "client" je novy soket spojujici klienta se serverem
     int client = accept(mainSocket, (sockaddr*)&clientInfo, &addrlen);
     if (client == -1)
     {
       cerr << "Chyba pri spojeni s uzivatelem!" << endl;
     }
     string adresa = inet_ntoa((in_addr)clientInfo.sin_addr);
     // Zjistím IP klienta.
     cout << "----------------------------------" << endl;
     cout << "Nekdo se pripojil s adresy: "
	  << adresa << endl;
     // Prijmu data. Ke komunikaci s klientem pouzivam soket "client"
     if ((size = recv(client, buf, BUFSIZE - 1, 0)) == -1)
     {
	 cerr << "Klient se odpojil!" << endl;
	 odpojen = true;
     }
     // Pokud nedoslo k chybe prijmi a odesli data klientu
     if (odpojen != true) {
       cout << "Prijato: " << size << " bytu"<< endl;
       cout << "Prijata zprava: " << buf << endl;
       totalSize += size + 2 +
	   adresa.length();
       text += adresa;
       text += ": ";
       text += buf;
       cout << "----------------------------------" << endl;
       cout << "Dosavadni konverzace:" << endl;
       cout << text;

       // Odeslu veskerou dosavadni komunikaci
       if ((size = send(client, text.c_str(), totalSize, 0)) == -1)
       {
         cerr << "Problem s odeslanim dat klientu!" << endl;
         odpojen = true;
       }
       cout << "Odeslano: " << size << endl;
     }
     //Uzavru spojeni s  klientem
     close(client);
   }
   while (++count != 10);
   cout << "Koncim" << endl;
   close(mainSocket);
   return 0;
}
