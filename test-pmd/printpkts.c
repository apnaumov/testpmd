#include "printpkts.h"

void printpkts_print_link(struct readpkt_link_hdr * link_hdr) {
	printf("Ethernet frame:\n");
	char buf[18];
	rte_ether_format_addr(&buf[0], 18, &link_hdr->eth_hdr->dst_addr);
	printf("	Destination address: %s\n", &buf[0]);
	rte_ether_format_addr(&buf[0], 18, &link_hdr->eth_hdr->src_addr);
	printf("	Source address: %s\n", &buf[0]);
	if (link_hdr->vlan_hdr != NULL) {
		printf("	VLAN Information:\n");
		uint16_t vlan_tci = rte_be_to_cpu_16(link_hdr->vlan_hdr->vlan_tci);
		uint16_t vlan_priority = vlan_tci >> 13;
		uint16_t cfi = (vlan_tci >> 12) & 0x1;
		uint16_t ident_code =  vlan_tci & 0x3ff;

		printf("		Identifier: %#x\n", rte_be_to_cpu_16(link_hdr->eth_hdr->ether_type));
		printf("		Priority: %#x\n", vlan_priority);
		printf("		CFI: %#x\n", cfi);
		printf("		VLAN: %u\n", ident_code);
		printf("	Ethernet type: %#x\n", rte_be_to_cpu_16(link_hdr->vlan_hdr->eth_proto));
	} else {
		printf("	Ethernet type: %#x\n", rte_be_to_cpu_16(link_hdr->eth_hdr->ether_type));
	}

}

void printpkts_print_ip(struct readpkt_network_hdr * network_hdr) {
	printf("Ip packet:\n");
	struct rte_ipv4_hdr *ip_hdr = network_hdr->ip_hdr;
	uint8_t version = ip_hdr->version_ihl >> 4;
	uint8_t len = rte_ipv4_hdr_len(ip_hdr);
	printf("	Version: %u\n", version);
	printf("	Header length: %u\n", len);
	printf("	TOS: %u\n", ip_hdr->type_of_service);
	printf("	Total length: %u\n", rte_be_to_cpu_16(ip_hdr->total_length));
	printf("	Packet ID: %#x\n", rte_be_to_cpu_16(ip_hdr->packet_id));
	uint16_t fragment_offset = rte_be_to_cpu_16(ip_hdr->fragment_offset);
	printf("	Flags: %#x\n", fragment_offset >> 13);
	printf("	Fragment offset: %#x\n", fragment_offset & 0x1fff);
	printf("	TTL: %u\n", ip_hdr->time_to_live);
	printf("	Protocol: %u\n", ip_hdr->next_proto_id);
	printf("	Checksum: %#x\n", rte_be_to_cpu_16(ip_hdr->hdr_checksum));
	struct in_addr addr;
	addr.s_addr = ip_hdr->src_addr;
	printf("	Source address: %s\n", inet_ntoa(addr));
	addr.s_addr = ip_hdr->dst_addr;
	printf("	Destination address: %s\n", inet_ntoa(addr));

	if (network_hdr->options.options_array != NULL) {
		printf("	Options (as char array): ");
		char *options_array = network_hdr->options.options_array;
		for (int i = 0; i < network_hdr->options.options_size; ++i) {
			printf("%#x ", options_array[i] & 0xff);
		}
		printf("\n");
	}
}

void printpkts_print_udp(struct rte_udp_hdr * udp_hdr) {
	printf("Udp datagram:\n");
	printf("	Source port: %u\n", rte_be_to_cpu_16(udp_hdr->src_port));
	printf("	Destination port: %u\n", rte_be_to_cpu_16(udp_hdr->dst_port));
	printf("	Length: %u\n", rte_be_to_cpu_16(udp_hdr->dgram_len));
	printf("	Checksum: %#x\n", rte_be_to_cpu_16(udp_hdr->dgram_cksum));
}

void printpkts_print_tcp(struct readpkt_tcp * tcp_struct) {
	struct rte_tcp_hdr *tcp_hdr = tcp_struct->tcp_hdr;
	printf("Tcp datagram:\n");
	printf("	Source port: %u\n", rte_be_to_cpu_16(tcp_hdr->src_port));
	printf("	Destination port: %u\n", rte_be_to_cpu_16(tcp_hdr->dst_port));
	printf("	Sequence number: %u\n", rte_be_to_cpu_32(tcp_hdr->sent_seq));
	printf("	Acknowledgment number: %u\n", rte_be_to_cpu_32(tcp_hdr->recv_ack));
	printf("	Data offset: %u\n", (uint8_t) rte_be_to_cpu_16(tcp_hdr->data_off));
	printf("	Flags: %#x\n", tcp_hdr->tcp_flags);
	printf("	Window size: %u\n", rte_be_to_cpu_16(tcp_hdr->rx_win));
	printf("	Checksum: %#x\n", rte_be_to_cpu_16(tcp_hdr->cksum));
	printf("	Urgent pointer: %u\n", rte_be_to_cpu_16(tcp_hdr->tcp_urp));
	if (tcp_struct->options_array != NULL) {
		printf("	Options (as char array): ");
		char *options_array = tcp_struct->options_array;
		for (int i = 0; i < tcp_struct->options_size; ++i) {
			printf("%#x ", options_array[i] & 0xff);
		}
		printf("\n");
	}
}

void printpkts_print_packet(struct readpkt_packet *packet) {
	static uint packet_number = 1;
	printf("Packet number %u:\n\n", packet_number);


	printpkts_print_link(&packet->link_hdr);
	printpkts_print_ip(&packet->net_hdr);
	
	switch (packet->transport_hdr.type) {
		case TCP:
			printpkts_print_tcp(&packet->transport_hdr.transport_payload.tcp_struct);
			break;
		case UDP:
			printpkts_print_udp(packet->transport_hdr.transport_payload.udp_hdr);
			break;
	}

	packet_number++;
	
	printf("\n");
}