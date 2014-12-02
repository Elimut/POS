#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/ether.h>
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFSIZE 10000

using namespace std;

int main (int argc,char *argv[]){

    string textout;
    string textin;
    hostent *host;              // Vzdálený počítač;
    sockaddr_in serverSock;     // Vzdálený "konec potrubí"
    int mySocket;               // Soket
    char buf[BUFSIZE];          // Přijímací buffer
    int size,celasize;          // Počet přijatých a odeslaných bytů

 if (argc != 4){
	cerr <<"vstup: adresa port cesta\n ";
	return -1;
   
}
	// zpracovani vstupu
	int port = atoi(argv[2]); //čílo portu
	string filename = argv[3];
	textout += "GET ";
	textout += argv[3];
	textout += " HTTP/1.1\nhost: ";
	textout += argv[1];
	textout+= "\nConnection: close\n\n";
	cout<< textout;
	// preklad adresy
	if ((host = gethostbyname(argv[1])) == NULL)
    {
        cerr << "Adresu se nepodarilo zpracovat\n" << endl;
        return -1;
    }
	// novy socket
    if ((mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        cerr << "Nelze vytvorit soket\n";
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
        return -1;
    }
	
    //Odesílání dat
	if ((size = send(mySocket, textout.c_str(), textout.size() + 1, 0)) == -1)
    {
        cerr << "Problem s odeslanim dat" << endl;
        return -1;
    }
	cout << "Odeslano " << size << endl;
	// Příjem dat
	textin="";
  while (((size = recv(mySocket, buf, BUFSIZE - 1, 0)) != -1) && (size != 0))
    {
        buf[size] = '\0';
        celasize += size;
        textin += buf;
    }
	// Uzavrení spojeni
    close(mySocket);
  
    cout << "Prijato " << celasize << " bytu\n\n";
     // cout << textout << endl<< textin<<endl;
    cout<< "HTTP Hlavicka:" << endl;
    int offset = textin.find("\r\n\r\n");
    string hlavicka = textin.substr(0,offset+2);
    string telo = textin.substr(offset+4);
    cout << hlavicka << endl;
    cout << "HTTP Telo:\n" << telo << endl << endl;
    return 0;



}
