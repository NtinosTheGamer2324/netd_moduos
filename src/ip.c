#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

static uint16_t g_ip_id = 1;

/* Build and send an IPv4 frame.
 * Resolves dst MAC via ARP cache; uses broadcast if not found. */
int ip_tx(uint32_t dst, uint8_t proto,
          const uint8_t *payload, uint16_t plen)
{
    uint8_t dst_mac[6];

    /* determine next-hop */
    uint32_t nexthop = dst;
    if (g_net.gateway && (dst & g_net.netmask) != (g_net.ip & g_net.netmask))
        nexthop = g_net.gateway;

    if (nexthop == IP_BCAST || dst == IP_BCAST) {
        memset(dst_mac, 0xff, 6);
    } else if (arp_lookup(nexthop, dst_mac) != 0) {
        /* not in cache — send ARP request and use broadcast for now */
        arp_request(nexthop);
        memset(dst_mac, 0xff, 6);
    }

    uint16_t total = (uint16_t)(20 + plen);
    uint8_t frame[14 + 20 + 1500];

    /* Ethernet */
    memcpy(frame+0, dst_mac, 6);
    memcpy(frame+6, g_net.mac, 6);
    *(uint16_t*)(frame+12) = htons(ETHERTYPE_IP);

    /* IP header */
    uint8_t *ip = frame+14;
    memset(ip, 0, 20);
    ip[0]  = 0x45;
    ip[1]  = 0;
    *(uint16_t*)(ip+2)  = htons(total);
    *(uint16_t*)(ip+4)  = htons(g_ip_id++);
    *(uint16_t*)(ip+6)  = 0;
    ip[8]  = 64;  /* TTL */
    ip[9]  = proto;
    *(uint32_t*)(ip+12) = htonl(g_net.ip);
    *(uint32_t*)(ip+16) = htonl(dst);
    *(uint16_t*)(ip+10) = htons(inet_cksum(ip, 20));

    memcpy(frame+34, payload, plen);

    return driver_tx(frame, (uint32_t)(14+20+plen));
}

void ip_rx(const uint8_t *frame, int len) {
    if (len < 34) return;
    const uint8_t *ip = frame + 14;

    uint8_t  ihl     = (ip[0] & 0x0F) * 4;
    uint8_t  proto   = ip[9];
    uint32_t src_ip  = ntohl(*(uint32_t*)(ip+12));
    uint32_t dst_ip  = ntohl(*(uint32_t*)(ip+16));
    uint16_t tot_len = ntohs(*(uint16_t*)(ip+2));

    /* drop if not for us and not broadcast */
    if (g_net.ip && dst_ip != g_net.ip && dst_ip != IP_BCAST) return;

    int payload_len = (int)tot_len - ihl;
    if (payload_len <= 0) return;
    const uint8_t *payload = ip + ihl;

    /* learn sender */
    arp_learn(src_ip, frame+6);

    switch (proto) {
        case IPPROTO_ICMP:
            icmp_rx(src_ip, payload, payload_len);
            break;
        case IPPROTO_UDP:
            udp_rx(src_ip, payload, payload_len);
            break;
        default:
            break;
    }
}