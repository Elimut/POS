#include <iostream>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define BUFSIZE 1024

using namespace std;

uint16_t checksum(uint8_t *addr_b, unsigned len)
{
    uint16_t* addr = (uint16_t*) addr_b;
    uint16_t answer = 0;
    /*
* Our algorithm is simple, using a 32 bit accumulator (sum), we add
* sequential 16 bit words to it, and at the end, fold back all the
* carry bits from the top 16 bits into the lower 16 bits.
*/
    uint32_t sum = 0;
    while (len > 1) {
        sum += *addr++;
        len -= 2;
    }

    // mop up an odd byte, if necessary
    if (len == 1) {
        *(unsigned char *)&answer = *(unsigned char *)addr ;
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
    sum += (sum >> 16); // add carry
    answer = ~sum; // truncate to 16 bits
    return answer;
}

int main(int argc, char *argv[])
{
  /*size_t*/ socklen_t size;//misto size_t pouzit socklen_t !!!
  hostent *host;
  icmphdr *icmp, *icmpRecv;
  iphdr *ip;
  int sock, total, lenght, pocet_dotazu;
  unsigned int ttl;
  sockaddr_in sendSockAddr, receiveSockAddr;
  char buffer[BUFSIZE];
  fd_set mySet;
  timeval tv, t_begin, t_end;
  char *addrString;
  in_addr addr;
  unsigned short int pid = getpid(), p;

  if (argc != 3)
  {
    cerr << "Syntaxe: " << argv[0] << " adresa pocet-dotazu" << endl;
    return 1;
  }

  // preklad domenoveho jmena
  if ((host = gethostbyname(argv[1])) == NULL)
  {
    cerr << "Spatna adresa" << endl;
    return 2;
  }

  // vytvoreni soketu
  if ((sock =
     socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
  {
    cerr << "Nelze vytvorit soket" << endl;
    return 3;
  }

  ttl = 255;
  setsockopt(sock, IPPROTO_IP, IP_TTL,
       (const char *)&ttl, sizeof(ttl));
  // vytvorime ICMP paket echo zadost
  icmp = (icmphdr *)malloc(sizeof(icmphdr));
  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->un.echo.id = pid;

  // priprava odeslani dat
  sendSockAddr.sin_family = AF_INET;
  sendSockAddr.sin_port = 0;
  memcpy(&sendSockAddr.sin_addr,
         host->h_addr, host->h_length);

  // nastavime pocet dotazu ze vstupu
  sscanf(argv[2], "%d", &pocet_dotazu);//opravit

  for (p = 1; p <= pocet_dotazu; p++)
  {
    icmp->checksum = 0;
    icmp->un.echo.sequence = p;
    icmp->checksum =
         checksum((unsigned char *)icmp, sizeof(icmphdr));

    // odesleme vyplnenou hlavicku
    sendto(sock,
        (char *)icmp, sizeof(icmphdr), 0,
        (sockaddr *)&sendSockAddr, sizeof(sockaddr));

    // zacneme merit cas
    gettimeofday(&t_begin, NULL);

    // pockame max. 5 sekund
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    do
    {
      FD_ZERO(&mySet);
      FD_SET(sock, &mySet);
      if (select(sock + 1, &mySet, NULL, NULL, &tv) < 0)
      {
            cerr << "Selhal select" << endl;
            break;
      }

      // jestli doslo k udalosti na soketu
      // prijmeme data a proverime je
      if (FD_ISSET(sock, &mySet))
      {
            size = sizeof(sockaddr_in);
            if ((lenght = recvfrom(sock,
               buffer, BUFSIZE, 0,
               (sockaddr *)&receiveSockAddr, &size)) == -1)
            {

              cerr << "Problem pri prijmani dat" << endl;
            }

         // Ted budeme analizovat data
         ip = (iphdr *) buffer;
            icmpRecv = (icmphdr *) (buffer + ip->ihl * 4);
         // je to echo odpoved ? s nasim id ?
            if ((icmpRecv->un.echo.id == pid)
               && (icmpRecv->type == ICMP_ECHOREPLY)
               && (icmpRecv->un.echo.sequence == p))
            {

         // ukoncime pocitani
         gettimeofday(&t_end, NULL);

               addrString =
               strdup(inet_ntoa(receiveSockAddr.sin_addr));
               host = gethostbyaddr(&receiveSockAddr.sin_addr,
                              4, AF_INET);
               cout << lenght << " bytu z "
                    << (host == NULL? "?" : host->h_name)
                    << " (" << addrString << "): icmp_seq="
                    << icmpRecv->un.echo.sequence << " ttl="
                    << (int)ip->ttl
                 << "time= "<< (t_end.tv_usec - t_begin.tv_usec) / 1000.0<<
         " ms" << endl;
               free(addrString);
            }
      }
      else
      {
        cout << "Cas vyprsel" << endl;
        break;
      }
     } while (!((icmpRecv->un.echo.id == pid)
         && (icmpRecv->type == ICMP_ECHOREPLY)
         && (icmpRecv->un.echo.sequence == p)));
   }
  close(sock);
  free(icmp);
  return 0;
}
