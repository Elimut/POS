#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 65536

int main(int argc, char *argv[])
{
  int sock, lenght,
    count, index_rozhrani,
	index_vypsanych_ramcu, vypis,
	frag_off;
  char buffer[MAX];
  sockaddr_ll addr;
  socklen_t size;
  iphdr *ip;
  char *addrString;
  register char mf;
  register short int offset;
  fd_set mySet;

  // overime ze uzivatel zadal
  // spravny pocet parametru
  if (argc < 2 && argc > 3)
  {
    fprintf(stderr, "Syntaxe v1:\n\t %s pocet-paketu\n", argv[0]);
    fprintf(stderr, "Syntaxe v2:\n\t %s pocet-paketu index-rozhrani\n", argv[0]);
    return -1;
  }

  sscanf(argv[1], "%d", &count);
  if(argc == 3)
  {
      sscanf(argv[2], "%d", &index_rozhrani);
  }

  //vytvoreni soketu
  if ((sock = socket(PF_PACKET,
		     SOCK_DGRAM,
		     htons(ETH_P_IP))) == -1)
  {
      perror("Selhal socket");
      return -1;
  }


  //prijem dat
  index_vypsanych_ramcu = 0;
  vypis = 0;
  do
  {
      // nebude se nic vypisovat
      vypis = 0;

      //precti ramec
      FD_ZERO(&mySet);
      FD_SET(sock, &mySet);
      if(select(sock + 1, &mySet, NULL,
		NULL, NULL) == -1)
      {
	  perror("Selhal select");
	  close(sock);
	  return -1;
      }
      size = sizeof(addr);
      if ((lenght = recvfrom(sock,
			     buffer, MAX, 0,
			     (sockaddr *)&addr,
			     &size)) == -1)
      {
	  perror("Selhal recvfrom");
	  close(sock);
	  return -1;
      }

      // dle verze vstupu
      // bud data vypisuj ci je ignoruj
      if(argc == 3)
      {
         if(addr.sll_ifindex == index_rozhrani)
         {
	     vypis = 1;
	     index_vypsanych_ramcu++;
	 }
      }
      else
      {
	  vypis = 1;
	  index_vypsanych_ramcu++;
      }

      if(vypis == 1)
      {
         if(index_vypsanych_ramcu == 1)
	    printf("---------------------------------------------\n");
         //vypsani informaci o linkovem ramci
         printf("Prijato: %d bytu\n", lenght);
         printf("Protokol (linkovy): %d\n", addr.sll_protocol);
         printf("Index sitoveho rozhrani: %d\n",
	        addr.sll_ifindex);
         printf("Velikost (linkove) adresy odesilatele: %d\n",
	        addr.sll_halen);
         if (addr.sll_halen != 0)
         {
             printf("Adresa (linkova) odesilatele: ");
             for(int i = 0; i < addr.sll_halen - 1; i++)
             {
	         printf("%x:", addr.sll_addr[i]);
             }
             printf("%x\n", addr.sll_addr[addr.sll_halen -1]);
         }
	 //ted vypiseme data z hlavicky
	 ip = (iphdr*)buffer;
	 printf("Verze IP protokolu: %d\n", ip->version);
	 printf("Velikost IP hlavicky: %d\n", ip->ihl * 4);
	 printf("Typ sluzby: %d\n", ip->tos);
	 printf("Velikost IP paketu: %d\n",
		ntohs(ip->tot_len));
	 printf("Identifikator paketu: %d\n",
		ntohs(ip->id));
	 printf("TTL: %d\n", ip->ttl);
	 frag_off = ntohs(ip->frag_off);
	 printf("Priznak DF: %d\n",
		(frag_off & 0x4000) >> 14);
	 mf = (frag_off & 0x2000) >> 13;
	 printf("Priznak MF: %d\n", mf);
	 offset = (frag_off & 0x1FFF);
     	 if(!mf && offset == 0)
	 {
	     printf("IP paket neni fragmentovan\n");
	 }
	 else
     	 {
	     printf("Posunuti fragmentu od zacatku: %d\n",
		    offset);
	 }
	 printf("Protokol: ");
     	 switch(ip->protocol)
	 {
	     case IPPROTO_UDP: printf("UDP\n"); break;
	     case IPPROTO_TCP: printf("TCP\n"); break;
	     case IPPROTO_ICMP: printf("ICMP\n"); break;
	     default: printf("%d\n", ip->protocol); break;
	 }
	 addrString=strdup(inet_ntoa(*(in_addr*)&ip->saddr));
	 printf("IP adresa odesilatele: %s\n", addrString);
	 free(addrString);
	 addrString=strdup(inet_ntoa(*(in_addr*)&ip->daddr));
	 printf("IP adresa prijemce: %s\n", addrString);
	 free(addrString);
         printf("---------------------------------------------\n");
      }
  }
  while(index_vypsanych_ramcu != count);
}
