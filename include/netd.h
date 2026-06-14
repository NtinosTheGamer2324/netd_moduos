#pragma once
#include "./libc.h"

/* ── endian helpers ───────────────────────────────────────────────────── */
static inline uint16_t htons(uint16_t v){ return (uint16_t)((v>>8)|(v<<8)); }
static inline uint16_t ntohs(uint16_t v){ return htons(v); }
static inline uint32_t htonl(uint32_t v){
    return ((v>>24)&0xFF)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|((v<<24)&0xFF000000);
}
static inline uint32_t ntohl(uint32_t v){ return htonl(v); }

/* ── wire constants ───────────────────────────────────────────────────── */
#define ETHERTYPE_ARP   0x0806
#define ETHERTYPE_IP    0x0800

#define IPPROTO_ICMP    1
#define IPPROTO_UDP     17

#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

#define ICMP_ECHO_REQ   8
#define ICMP_ECHO_REP   0
#define ICMP_UNREACH    3

#define DHCP_PORT_SERVER 67
#define DHCP_PORT_CLIENT 68
#define DNS_PORT         53

/* ── devfs protocol ───────────────────────────────────────────────────── */
#define NET_CMD_GET_MODE    1u
#define NET_CMD_GET_LINK_UP 2u
#define NET_CMD_GET_MTU     3u
#define NET_CMD_GET_MAC     4u
#define NET_CMD_TX_FRAME    5u
#define NET_CMD_RX_POLL     6u

/* ── IP address helpers ───────────────────────────────────────────────── */
#define MAKE_IP(a,b,c,d) \
    ((uint32_t)(a)<<24|(uint32_t)(b)<<16|(uint32_t)(c)<<8|(uint32_t)(d))
#define IP_BCAST        0xFFFFFFFFu

/* ── ARP cache ────────────────────────────────────────────────────────── */
#define ARP_CACHE_SIZE  16
#define ARP_TIMEOUT_MS  30000

typedef struct {
    uint32_t ip;
    uint8_t  mac[6];
    uint64_t ts;        /* time_ms() when learned */
    int      valid;
} arp_entry_t;

/* ── UDP port binding ─────────────────────────────────────────────────── */
#define UDP_MAX_BINDS   16
#define UDP_RXBUF_SIZE  2048

typedef struct {
    uint16_t port;
    uint8_t  buf[UDP_RXBUF_SIZE];
    int      len;       /* bytes waiting, 0 = empty */
    uint32_t src_ip;
    uint16_t src_port;
    int      used;
} udp_bind_t;

/* ── DHCP state ───────────────────────────────────────────────────────── */
typedef enum {
    DHCP_IDLE = 0,
    DHCP_DISCOVER,
    DHCP_OFFER,
    DHCP_REQUEST,
    DHCP_BOUND,
    DHCP_FAILED
} dhcp_state_t;

/* ── global network state ─────────────────────────────────────────────── */
typedef struct {
    /* NIC */
    int      fd;
    uint8_t  mac[6];
    uint32_t mtu;
    int      link_up;

    /* IP config */
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns;

    /* DHCP */
    dhcp_state_t dhcp_state;
    uint32_t     dhcp_xid;
    uint32_t     dhcp_server;
    uint64_t     dhcp_lease_ms;

    /* ARP */
    arp_entry_t arp_cache[ARP_CACHE_SIZE];

    /* UDP */
    udp_bind_t  udp_binds[UDP_MAX_BINDS];

    /* ICMP ping reply tracking */
    uint16_t    ping_id;
    uint16_t    ping_seq;
    uint32_t    ping_reply_ip;
    int         ping_replied;

    /* DNS */
    uint16_t    dns_txid;
    char        dns_name[256];
    uint32_t    dns_result;
    int         dns_done;
} netd_state_t;

extern netd_state_t g_net;

/* ── checksum ─────────────────────────────────────────────────────────── */
static inline uint16_t inet_cksum(const uint8_t *data, int len) {
    uint32_t sum = 0;
    for (int i = 0; i+1 < len; i+=2)
        sum += (uint32_t)((data[i]<<8)|data[i+1]);
    if (len & 1) sum += (uint32_t)(data[len-1]<<8);
    while (sum>>16) sum = (sum&0xFFFF)+(sum>>16);
    return (uint16_t)(~sum);
}

/* ── forward declarations ─────────────────────────────────────────────── */

/* driver.c */
int  driver_open(const char *path);
int  driver_transact(uint32_t cmd, const uint8_t *payload, uint32_t plen,
                     uint8_t *rep_data, uint32_t *rep_len);
int  driver_tx(const uint8_t *frame, uint32_t len);
int  driver_rx(uint8_t *frame, uint32_t *len);

/* arp.c */
void arp_init(void);
int  arp_lookup(uint32_t ip, uint8_t *mac_out);
void arp_learn(uint32_t ip, const uint8_t *mac);
void arp_announce(void);
void arp_request(uint32_t ip);
void arp_rx(const uint8_t *frame, int len);

/* ip.c */
int  ip_tx(uint32_t dst, uint8_t proto,
           const uint8_t *payload, uint16_t plen);
void ip_rx(const uint8_t *frame, int len);

/* icmp.c */
void icmp_rx(uint32_t src, const uint8_t *data, int len);
int  icmp_ping(uint32_t dst, uint16_t id, uint16_t seq);

/* udp.c */
int  udp_tx(uint32_t dst, uint16_t src_port, uint16_t dst_port,
            const uint8_t *data, uint16_t len);
void udp_rx(uint32_t src_ip, const uint8_t *data, int len);
int  udp_bind(uint16_t port);
int  udp_recv(uint16_t port, uint8_t *buf, int max,
              uint32_t *src_ip, uint16_t *src_port);
void udp_unbind(uint16_t port);

/* dhcp.c */
void dhcp_start(void);
void dhcp_rx(uint32_t src_ip, const uint8_t *data, int len);
int  dhcp_bound(void);

/* dns.c */
int  dns_resolve(const char *name, uint32_t *ip_out);
void dns_rx(uint32_t src_ip, const uint8_t *data, int len);

/* api.c */
void api_init(void);
void api_poll(void);