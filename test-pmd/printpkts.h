#ifndef _PRINTPKTS_H_
#define _PRINTPKTS_H_

#include "readpktfwd.h"


extern void printpkts_print_link(struct readpkt_link_hdr * link_hdr);
extern void printpkts_print_ip(struct readpkt_network_hdr * network_hdr);
extern void printpkts_print_udp(struct rte_udp_hdr * udp_hdr);
extern void printpkts_print_tcp(struct readpkt_tcp * tcp_struct);
extern void printpkts_print_packet(struct readpkt_packet *packet);


#endif /*_PRINTPKTS_H_*/