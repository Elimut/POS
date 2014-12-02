#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
//#include <sdtarg.h>

#define BUFSIZE 1000

using namespace std;

int main(int argc, char *argv[])
{
   std::string text;             // Přijímaný text
   sockaddr_in sockName;         // "Jméno" portu
   sockaddr_in clientInfo;       // Klient, který se připojil
   int mainSocket;               // Soket
   int port;                     // Číslo portu
   char buf[BUFSIZE];            // Přijímací buffer
   int size;                     // Počet přijatých a odeslaných bytů
   socklen_t addrlen;            // Velikost adresy vzdáleného počítače
   int count = 0;                // Počet připojení

   if (argc != 2)
   {
     cerr << "Syntaxe:\n\t" << argv[0]
	  << " " << "port" << endl;
     return 2;
   }
   port = atoi(argv[1]);

   // Vytvoříme soket - viz minulý díl
   if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
   {
     cerr << "Nelze vytvorit soket" << endl;
     return 3;
   }

   // Zaplníme strukturu sockaddr_in
   // 1) Rodina protokolů
   sockName.sin_family = AF_INET;
   // 2) Číslo portu, na kterém čekáme
   sockName.sin_port = htons(port);
   // 3) Nastavení IP adresy lokální síťové karty, přes kterou je možno se
   //    připojit. Nastavíme možnost připojit se odkudkoliv.
   sockName.sin_addr.s_addr = INADDR_ANY;
   // přiřadíme soketu jméno
   if (bind(mainSocket, (sockaddr *)&sockName, sizeof(sockName)) == -1)
   {
     cerr << "Problem s pojmenovanim soketu." << endl;
     return 1;
   }
   // Vytvoříme frontu požadavků na spojení.
   // Vytvoříme frontu maximální velikosti 10 požadavků.
   if (listen(mainSocket, 10) == -1)
   {
     cerr << "Problem s vytvorenim fronty" << endl;
     return 1;
   }
   do
   {
     // Poznačím si velikost struktury clientInfo.
     // Předám to funkci accept.
     addrlen = sizeof(clientInfo);
     // Vyberu z fronty požadavek na spojení.
     // "client" je nový soket spojující klienta se serverem.
     int client = accept(mainSocket, (sockaddr*)&clientInfo, &addrlen);
     int totalSize = 0;
     if (client == -1)
     {
       cerr << "Problém s prijetím spojeni" <<endl;
       return 1;
     }
     // Zjistím IP klienta.
     cout << "Nekdo se pripojil z adresy: "
	  << inet_ntoa((in_addr)clientInfo.sin_addr) << endl;
     // Přijmu data. Ke komunikaci s klientem používám soket "client"
     text = "";
     // Přijmeme maximálně 6 bytový pozdrav.
     while (totalSize != 6)
     {
       if ((size = recv(client, buf, BUFSIZE - 1, 0)) == -1)
       {
	 cerr << "Problem s prijetim dat." << endl;
	 return 1;
       }
       cout << "Prijato: " << size << endl << buf << endl;
       totalSize += size;
       text += buf;
     }
     cout << text;
     // Odešlu pozdrav
     if ((size = send(client, "Nazdar\n", 8, 0)) == -1)
     {
       cerr << "Problém s odesláním dat" << endl;
       return 2;
     }
     cout << "Odeslano: " << size << endl;
     // Uzavřu spojení s klientem
     close(client);
   }
   while (++count != 3);
   cout << "Koncím" << endl;
   close(mainSocket);
   return 0;
}
