#include <stdio.h>
#include <errno.h>   
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int main()
{
	int sockfd ;


	sockfd = socket(AF_PACKET , SOCK_DGRAM , htons(ETH_P_ARP));
	if(sockfd == -1)
		printf("\nError : %s",strerror(errno));


	// get the index number of linux network interface
	char *if_name = "eth0" ;
	struct ifreq ifr ;
	size_t if_name_len = strlen(if_name);

	if(if_name_len < sizeof(ifr.ifr_name)) 
	{
		memcpy(ifr.ifr_name , if_name , if_name_len) ;
		ifr.ifr_name[if_name_len] = 0 ;
	}
	else
	{
		printf("\nSize of interface name is greater");
	}

	if(ioctl(sockfd , SIOCGIFINDEX, &ifr) == -1)
	{
		printf("\nCant get the interface index");
	}
	
	int ifindex = ifr.ifr_ifindex ;


	// ethernet broadcast address
	const unsigned char ether_broadcast_addr[] = {0xff , 0xff , 0xff , 0xff , 0xff , 0xff} ;

	struct sockaddr_ll addr = {0};
	addr.sll_family = AF_PACKET ;
	addr.sll_ifindex = ifindex ;
	addr.sll_halen = ETHER_ADDR_LEN ;
	addr.sll_protocol = htons(ETH_P_ARP);
	memcpy(addr.sll_addr , ether_broadcast_addr , ETHER_ADDR_LEN); 


	// construct the arp header 
	struct ether_arp req ;
	req.arp_hrd = htons(ARPHRD_ETHER);
	req.arp_pro = htons(ETH_P_IP);
	req.arp_hln = ETHER_ADDR_LEN ;
	req.arp_pln = sizeof(in_addr_t);
	req.arp_op  = htons(ARPOP_REQUEST);
	memset(&req.arp_tha , 0 , sizeof(req.arp_tha));

	const char* target_ip_string = "10.10.12.1" ;
	struct in_addr target_ip_addr = {0} ;
	if(! inet_aton(target_ip_string , &target_ip_addr))
	{
		printf("\nNot able to convert string to ip addr");
	} 
	memcpy(&req.arp_tpa , &target_ip_addr.s_addr , sizeof(req.arp_tpa));

	// send the frame
	if(sendto(sockfd,&req,sizeof(req) , 0 , (struct sockaddr *)&addr , sizeof(addr)) == -1)
	{	
		printf("\nPacket sending failed");
	}
	
	// recv ARP reply
	char buffer[1024] ;
	struct sockaddr_ll packet_info;
	int packet_info_size = sizeof(packet_info_size);
		
	int len = 0 ;
	
	while(1){
	if((len =(int) recvfrom(sockfd,buffer , 1024 , 0 , (struct sockaddr *)&packet_info , &packet_info_size)) == -1)
	{
	
		printf("\nrecvfrom returned -1 , %s",strerror(errno));
	}
	else
	{
		struct ether_arp *rep = (struct ether_arp *)buffer ;

		if(rep->arp_op == htons(ARPOP_REPLY))
			printf("\nreceived reply : %s %s",rep->arp_tha,rep->arp_tpa);
		else
			printf("\nwrong packet");
	}
	}

	
}
