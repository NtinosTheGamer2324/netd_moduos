#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

#define DHCP_MAGIC      0x63825363u
#define DHCP_OP_BOOT_REQUEST 1
#define DHCP_OP_BOOT_REPLY   2

/* DHCP option tags */
#define OPT_SUBNET      1
#define OPT_ROUTER      3
#define OPT_DNS         6
#define OPT_LEASE_TIME  51
#define OPT_MSG_TYPE    53
#define OPT_SERVER_ID   54
#define OPT_REQ_IP      50
#define OPT_END         255

#define DHCP_DISCOVER   1
#define DHCP_OFFER      2
#define DHCP_REQUEST    3
#define DHCP_ACK        5
#define DHCP_NAK        6

/* DHCP packet is 236 bytes fixed + options */
#define DHCP_FIXED_LEN  236

static void dhcp_build_base(uint8_t *pkt, uint32_t xid) {
    memset(pkt, 0, DHCP_FIXED_LEN);
    pkt[0] = DHCP_OP_BOOT_REQUEST;
    pkt[1] = 1;   /* hw type Ethernet */
    pkt[2] = 6;   /* hw addr len */
    pkt[3] = 0;   /* hops */
    *(uint32_t*)(pkt+4)  = htonl(xid);
    *(uint16_t*)(pkt+8)  = 0;   /* secs */
    *(uint16_t*)(pkt+10) = htons(0x8000); /* broadcast flag */
    memcpy(pkt+28, g_net.mac, 6); /* chaddr */
    *(uint32_t*)(pkt+236-4+4) /* magic after fixed */;
    /* magic cookie at offset 236 */
}

static int dhcp_send(uint8_t msg_type, uint32_t req_ip, uint32_t server_id) {
    uint8_t pkt[DHCP_FIXED_LEN + 64];
    memset(pkt, 0, sizeof(pkt));

    /* fixed header */
    pkt[0] = DHCP_OP_BOOT_REQUEST;
    pkt[1] = 1; pkt[2] = 6; pkt[3] = 0;
    *(uint32_t*)(pkt+4)  = htonl(g_net.dhcp_xid);
    *(uint16_t*)(pkt+10) = htons(0x8000); /* broadcast */
    memcpy(pkt+28, g_net.mac, 6);

    /* magic cookie */
    *(uint32_t*)(pkt+DHCP_FIXED_LEN) = htonl(DHCP_MAGIC);

    /* options */
    uint8_t *opt = pkt + DHCP_FIXED_LEN + 4;

    /* msg type */
    *opt++ = OPT_MSG_TYPE; *opt++ = 1; *opt++ = msg_type;

    if (msg_type == DHCP_REQUEST) {
        if (req_ip) {
            *opt++ = OPT_REQ_IP; *opt++ = 4;
            *(uint32_t*)opt = htonl(req_ip); opt += 4;
        }
        if (server_id) {
            *opt++ = OPT_SERVER_ID; *opt++ = 4;
            *(uint32_t*)opt = htonl(server_id); opt += 4;
        }
    }

    *opt++ = OPT_END;

    int total = (int)(opt - pkt);

    return udp_tx(IP_BCAST, DHCP_PORT_CLIENT, DHCP_PORT_SERVER, pkt, (uint16_t)total);
}

void dhcp_start(void) {
    g_net.dhcp_xid   = (uint32_t)(time_ms() ^ 0xABCD1234u);
    g_net.dhcp_state = DHCP_DISCOVER;
    g_net.ip         = 0;
    g_net.netmask    = 0;
    g_net.gateway    = 0;
    g_net.dns        = 0;

    udp_bind(DHCP_PORT_CLIENT);
    dhcp_send(DHCP_DISCOVER, 0, 0);
    printf("[DHCP] DISCOVER sent (xid=0x%x)\n", g_net.dhcp_xid);
}

static uint32_t dhcp_get_opt32(const uint8_t *opts, int opts_len, uint8_t tag) {
    int i = 0;
    while (i < opts_len) {
        uint8_t t = opts[i++];
        if (t == OPT_END) break;
        if (t == 0) continue; /* pad */
        if (i >= opts_len) break;
        uint8_t l = opts[i++];
        if (t == tag && l >= 4) {
            return ntohl(*(uint32_t*)(opts+i));
        }
        i += l;
    }
    return 0;
}

static uint8_t dhcp_get_msg_type(const uint8_t *opts, int opts_len) {
    int i = 0;
    while (i < opts_len) {
        uint8_t t = opts[i++];
        if (t == OPT_END) break;
        if (t == 0) continue;
        if (i >= opts_len) break;
        uint8_t l = opts[i++];
        if (t == OPT_MSG_TYPE && l >= 1) return opts[i];
        i += l;
    }
    return 0;
}

void dhcp_rx(uint32_t src_ip, const uint8_t *data, int len) {
    if (len < DHCP_FIXED_LEN + 4) return;
    if (data[0] != DHCP_OP_BOOT_REPLY) return;

    uint32_t xid = ntohl(*(uint32_t*)(data+4));
    if (xid != g_net.dhcp_xid) return;

    uint32_t magic = ntohl(*(uint32_t*)(data+DHCP_FIXED_LEN));
    if (magic != DHCP_MAGIC) return;

    const uint8_t *opts     = data + DHCP_FIXED_LEN + 4;
    int            opts_len = len  - DHCP_FIXED_LEN - 4;

    uint8_t  msg_type  = dhcp_get_msg_type(opts, opts_len);
    uint32_t your_ip   = ntohl(*(uint32_t*)(data+16));
    uint32_t server_id = dhcp_get_opt32(opts, opts_len, OPT_SERVER_ID);
    uint32_t subnet    = dhcp_get_opt32(opts, opts_len, OPT_SUBNET);
    uint32_t router    = dhcp_get_opt32(opts, opts_len, OPT_ROUTER);
    uint32_t dns       = dhcp_get_opt32(opts, opts_len, OPT_DNS);

    if (msg_type == DHCP_OFFER && g_net.dhcp_state == DHCP_DISCOVER) {
        g_net.dhcp_server = server_id ? server_id : src_ip;
        printf("[DHCP] OFFER: %u.%u.%u.%u from server %u.%u.%u.%u\n",
               (your_ip>>24)&0xFF,(your_ip>>16)&0xFF,
               (your_ip>>8)&0xFF, your_ip&0xFF,
               (g_net.dhcp_server>>24)&0xFF,(g_net.dhcp_server>>16)&0xFF,
               (g_net.dhcp_server>>8)&0xFF, g_net.dhcp_server&0xFF);
        g_net.dhcp_state = DHCP_REQUEST;
        dhcp_send(DHCP_REQUEST, your_ip, g_net.dhcp_server);
        printf("[DHCP] REQUEST sent\n");
        return;
    }

    if (msg_type == DHCP_ACK && g_net.dhcp_state == DHCP_REQUEST) {
        g_net.ip      = your_ip;
        g_net.netmask = subnet  ? subnet  : 0xFFFFFF00u;
        g_net.gateway = router  ? router  : 0;
        g_net.dns     = dns     ? dns     : 0;
        g_net.dhcp_state = DHCP_BOUND;

        printf("[DHCP] ACK: IP=%u.%u.%u.%u mask=%u.%u.%u.%u gw=%u.%u.%u.%u dns=%u.%u.%u.%u\n",
               (g_net.ip>>24)&0xFF,      (g_net.ip>>16)&0xFF,
               (g_net.ip>>8)&0xFF,        g_net.ip&0xFF,
               (g_net.netmask>>24)&0xFF,  (g_net.netmask>>16)&0xFF,
               (g_net.netmask>>8)&0xFF,    g_net.netmask&0xFF,
               (g_net.gateway>>24)&0xFF,  (g_net.gateway>>16)&0xFF,
               (g_net.gateway>>8)&0xFF,    g_net.gateway&0xFF,
               (g_net.dns>>24)&0xFF,      (g_net.dns>>16)&0xFF,
               (g_net.dns>>8)&0xFF,        g_net.dns&0xFF);

        arp_announce();
        return;
    }

    if (msg_type == DHCP_NAK) {
        printf("[DHCP] NAK received, retrying\n");
        g_net.dhcp_state = DHCP_FAILED;
    }
}

int dhcp_bound(void) {
    return g_net.dhcp_state == DHCP_BOUND;
}