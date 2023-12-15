#ifndef _READPKTFWD_H_
#define _READPKTFWD_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>

#include <sys/queue.h>
#include <sys/stat.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_cycles.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_memcpy.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_interrupts.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_string_fns.h>
#include <rte_flow.h>

enum TRANSPORT_TYPE {
	TCP = 0x06,
	UDP = 0x11
};

struct readpkt_packet {
	struct readpkt_link_hdr {
		struct rte_ether_hdr *eth_hdr;
		struct rte_vlan_hdr *vlan_hdr;
	} link_hdr;
	
	struct readpkt_network_hdr {
		struct rte_ipv4_hdr * ip_hdr;
		struct {
			uint8_t options_size;
			char *options_array;
		} options;
	} net_hdr;
	struct readpkt_transport_hdr {
		enum TRANSPORT_TYPE type;
		union {
			struct rte_udp_hdr *udp_hdr;
			struct readpkt_tcp {
				struct rte_tcp_hdr *tcp_hdr;

				uint8_t options_size;
				char *options_array;
			} tcp_struct;
		} transport_payload;
	} transport_hdr;
};



#endif /*_READPKTFWD_H_*/