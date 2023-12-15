#include "readpktfwd.h"
#include "testpmd.h"
#include "printpkts.h"

static void
readpkt_stream_init_receive(struct fwd_stream *fs)
{
	fs->disabled = ports[fs->rx_port].rxq[fs->rx_queue].state ==
						RTE_ETH_QUEUE_STATE_STOPPED;
}


static bool readpkt_parse_mbuf(struct rte_mbuf *mbuf, struct readpkt_packet *mbuf_to_pack) {

	struct rte_ether_hdr *eth_hdr;
	struct rte_vlan_hdr *vlan_hdr = NULL;
	uint64_t len = sizeof(struct rte_ether_hdr);

	/* Ether block */

	eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr *);
	rte_be16_t current_proto = eth_hdr->ether_type;


	if (rte_cpu_to_be_16(RTE_ETHER_TYPE_VLAN) == current_proto) {
		len += sizeof(vlan_hdr);
		vlan_hdr = (struct rte_vlan_hdr *)(eth_hdr + 1);
		current_proto = vlan_hdr->eth_proto;
	}
	if (rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4) != current_proto) {
		return false;
	}

	/* Ip block */

	struct rte_ipv4_hdr *ip_hdr = (struct rte_ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, char*) + len);
	uint8_t options_size = 0;
	char *options_array = NULL;
	len += sizeof(struct rte_ipv4_hdr);

	if (ip_hdr->next_proto_id != UDP && ip_hdr->next_proto_id != TCP) {
		return false;
	}

	options_size = rte_ipv4_hdr_len(ip_hdr) - 20;
	if (options_size != 0) {
		options_array = rte_pktmbuf_mtod(mbuf, char*) + len;
		len += options_size;
	}

	mbuf_to_pack->link_hdr.eth_hdr = eth_hdr;
	mbuf_to_pack->link_hdr.vlan_hdr = vlan_hdr;
	mbuf_to_pack->net_hdr.ip_hdr = ip_hdr;
	mbuf_to_pack->net_hdr.options.options_size = options_size;
	mbuf_to_pack->net_hdr.options.options_array = options_array;

	/* Transport block */
	
	switch (ip_hdr->next_proto_id) {
		case TCP:
			struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *)(rte_pktmbuf_mtod(mbuf, char*) + len);
			mbuf_to_pack->transport_hdr.type = TCP;
			mbuf_to_pack->transport_hdr.transport_payload.tcp_struct.tcp_hdr = tcp_hdr;
			len += sizeof(struct rte_tcp_hdr);

			uint8_t tcp_opt_size = 0;
			char *tcp_opt_arr = NULL;

			tcp_opt_size = ( (uint8_t) rte_be_to_cpu_16(tcp_hdr->data_off )) * 4;
			if (tcp_opt_size != 0) {
				tcp_opt_arr = rte_pktmbuf_mtod(mbuf, char*) + len;
			}

			mbuf_to_pack->transport_hdr.transport_payload.tcp_struct.options_size = tcp_opt_size;
			mbuf_to_pack->transport_hdr.transport_payload.tcp_struct.options_array = tcp_opt_arr;
			break;
		case UDP:
			struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)(rte_pktmbuf_mtod(mbuf, char*) + len);
			mbuf_to_pack->transport_hdr.type = UDP;
			mbuf_to_pack->transport_hdr.transport_payload.udp_hdr = udp_hdr;
			break;
	}

	return true;
} 

static bool
readpkt_pkt_burst_receive(struct fwd_stream *fs)
{
	struct rte_mbuf  *pkts_burst[MAX_PKT_BURST];
	uint16_t nb_rx;

	/*
	 * Receive a burst of packets.
	 */
	nb_rx = common_fwd_stream_receive(fs, pkts_burst, nb_pkt_per_burst);
	if (unlikely(nb_rx == 0))
		return false;


	for (int i = 0; i < nb_rx; ++i) {
		struct readpkt_packet packet;
		if (!readpkt_parse_mbuf(pkts_burst[i], &packet)) {
			continue;
		}
		printpkts_print_packet(&packet);
	}


	rte_pktmbuf_free_bulk(pkts_burst, nb_rx);
	return true;
}

struct fwd_engine readpkt_fwd_engine = {
	.fwd_mode_name  = "readpkt",
	.stream_init    = readpkt_stream_init_receive,
	.packet_fwd     = readpkt_pkt_burst_receive,
};
