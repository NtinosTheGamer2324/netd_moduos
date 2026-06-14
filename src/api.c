#define LIBC_NO_START
#include "./libc.h"
#include "netd.h"

/* ── API command codes ────────────────────────────────────────────────── */
#define NETD_CMD_GET_INFO       1
#define NETD_CMD_PING           2
#define NETD_CMD_UDP_SEND       3
#define NETD_CMD_UDP_RECV       4
#define NETD_CMD_UDP_BIND       5
#define NETD_CMD_UDP_UNBIND     6
#define NETD_CMD_DNS_RESOLVE    7
#define NETD_CMD_ARP_LOOKUP     8
#define NETD_CMD_ARP_FLUSH      9
#define NETD_CMD_SET_IP         10

/* ── request / response structs ──────────────────────────────────────── */

typedef struct {
    uint32_t cmd;
} __attribute__((packed)) netd_req_hdr_t;

typedef struct {
    uint8_t  mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns;
    uint32_t mtu;
    uint32_t link_up;
    uint32_t dhcp_bound;
} __attribute__((packed)) netd_info_t;

typedef struct {
    uint32_t ip;
    uint16_t count;
    uint16_t timeout_ms;
} __attribute__((packed)) netd_ping_req_t;

typedef struct {
    uint32_t replied;
    uint32_t reply_ip;
} __attribute__((packed)) netd_ping_rep_t;

typedef struct {
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;
    uint8_t  data[1472];
} __attribute__((packed)) netd_udp_send_req_t;

typedef struct {
    uint16_t port;
} __attribute__((packed)) netd_udp_bind_req_t;

typedef struct {
    uint16_t port;
    uint16_t max_len;
} __attribute__((packed)) netd_udp_recv_req_t;

typedef struct {
    uint32_t src_ip;
    uint16_t src_port;
    uint16_t len;
    uint8_t  data[1472];
} __attribute__((packed)) netd_udp_recv_rep_t;

typedef struct {
    char name[256];
} __attribute__((packed)) netd_dns_req_t;

typedef struct {
    uint32_t ip;
} __attribute__((packed)) netd_dns_rep_t;

typedef struct {
    uint32_t ip;
} __attribute__((packed)) netd_arp_req_t;

typedef struct {
    uint8_t  mac[6];
    uint32_t found;
} __attribute__((packed)) netd_arp_rep_t;

typedef struct {
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns;
} __attribute__((packed)) netd_setip_req_t;

/* ── response header ─────────────────────────────────────────────────── */
typedef struct {
    int32_t  status;
    uint32_t len;
    uint8_t  data[1500];
} __attribute__((packed)) netd_rep_t;

/* ── userfs invoke handler ────────────────────────────────────────────── */

static netd_rep_t g_rep;

static ssize_t api_invoke(void *ctx,
                          const void *in_buf,  size_t in_size,
                          void       *out_buf, size_t out_size)
{
    (void)ctx;
    if (!in_buf || in_size < sizeof(netd_req_hdr_t)) return -1;

    const netd_req_hdr_t *hdr = (const netd_req_hdr_t *)in_buf;
    memset(&g_rep, 0, sizeof(g_rep));

    switch (hdr->cmd) {

        case NETD_CMD_GET_INFO: {
            netd_info_t info;
            memcpy(info.mac, g_net.mac, 6);
            info.ip         = g_net.ip;
            info.netmask    = g_net.netmask;
            info.gateway    = g_net.gateway;
            info.dns        = g_net.dns;
            info.mtu        = g_net.mtu;
            info.link_up    = (uint32_t)g_net.link_up;
            info.dhcp_bound = (uint32_t)dhcp_bound();
            memcpy(g_rep.data, &info, sizeof(info));
            g_rep.len    = sizeof(info);
            g_rep.status = 0;
            break;
        }

        case NETD_CMD_PING: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_ping_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_ping_req_t *req =
                (const netd_ping_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));

            netd_ping_rep_t rep;
            memset(&rep, 0, sizeof(rep));

            uint16_t id  = (uint16_t)(time_ms() & 0xFFFF);
            uint16_t seq = 1;
            uint32_t timeout = req->timeout_ms ? req->timeout_ms : 2000;
            uint16_t count   = req->count      ? req->count      : 1;

            for (int i = 0; i < count; i++) {
                icmp_ping(req->ip, id, (uint16_t)(seq+i));
                uint64_t deadline = time_ms() + timeout;
                while (time_ms() < deadline && !g_net.ping_replied) yield();
                if (g_net.ping_replied) {
                    rep.replied  = 1;
                    rep.reply_ip = g_net.ping_reply_ip;
                    g_net.ping_replied = 0;
                }
            }

            memcpy(g_rep.data, &rep, sizeof(rep));
            g_rep.len    = sizeof(rep);
            g_rep.status = rep.replied ? 0 : -1;
            break;
        }

        case NETD_CMD_UDP_BIND: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_udp_bind_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_udp_bind_req_t *req =
                (const netd_udp_bind_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            g_rep.status = udp_bind(req->port);
            break;
        }

        case NETD_CMD_UDP_UNBIND: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_udp_bind_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_udp_bind_req_t *req =
                (const netd_udp_bind_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            udp_unbind(req->port);
            g_rep.status = 0;
            break;
        }

        case NETD_CMD_UDP_SEND: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_udp_send_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_udp_send_req_t *req =
                (const netd_udp_send_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            g_rep.status = udp_tx(req->dst_ip, req->src_port, req->dst_port,
                                  req->data, req->len);
            break;
        }

        case NETD_CMD_UDP_RECV: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_udp_recv_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_udp_recv_req_t *req =
                (const netd_udp_recv_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            netd_udp_recv_rep_t rep;
            memset(&rep, 0, sizeof(rep));
            int n = udp_recv(req->port, rep.data,
                             req->max_len ? req->max_len : sizeof(rep.data),
                             &rep.src_ip, &rep.src_port);
            rep.len = (uint16_t)(n > 0 ? n : 0);
            memcpy(g_rep.data, &rep, sizeof(rep));
            g_rep.len    = sizeof(rep);
            g_rep.status = n > 0 ? 0 : -1;
            break;
        }

        case NETD_CMD_DNS_RESOLVE: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_dns_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_dns_req_t *req =
                (const netd_dns_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            netd_dns_rep_t rep;
            memset(&rep, 0, sizeof(rep));
            int rc = dns_resolve(req->name, &rep.ip);
            memcpy(g_rep.data, &rep, sizeof(rep));
            g_rep.len    = sizeof(rep);
            g_rep.status = rc;
            break;
        }

        case NETD_CMD_ARP_LOOKUP: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_arp_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_arp_req_t *req =
                (const netd_arp_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            netd_arp_rep_t rep;
            memset(&rep, 0, sizeof(rep));
            int rc = arp_lookup(req->ip, rep.mac);
            rep.found    = (rc == 0) ? 1 : 0;
            memcpy(g_rep.data, &rep, sizeof(rep));
            g_rep.len    = sizeof(rep);
            g_rep.status = rc;
            break;
        }

        case NETD_CMD_ARP_FLUSH: {
            memset(g_net.arp_cache, 0, sizeof(g_net.arp_cache));
            g_rep.status = 0;
            break;
        }

        case NETD_CMD_SET_IP: {
            if (in_size < sizeof(netd_req_hdr_t) + sizeof(netd_setip_req_t)) {
                g_rep.status = -1; break;
            }
            const netd_setip_req_t *req =
                (const netd_setip_req_t *)((const uint8_t*)in_buf + sizeof(netd_req_hdr_t));
            g_net.ip      = req->ip;
            g_net.netmask = req->netmask;
            g_net.gateway = req->gateway;
            g_net.dns     = req->dns;
            if (g_net.ip) arp_announce();
            g_rep.status = 0;
            break;
        }

        default:
            g_rep.status = -99;
            break;
    }

    size_t out_len = sizeof(int32_t) + sizeof(uint32_t) + g_rep.len;
    if (out_len > out_size) out_len = out_size;
    memcpy(out_buf, &g_rep, out_len);
    return (ssize_t)out_len;
}

void api_init(void) {
    userfs_user_node_t node;
    memset(&node, 0, sizeof(node));
    node.path     = "netd";
    node.owner_id = "netd";
    node.perms    = USERFS_PERM_READ_WRITE | USERFS_PERM_INVOKE;
    node.ops.invoke = api_invoke;
    node.ctx      = NULL;

    int rc = userfs_register(&node);
    if (rc == 0)
        printf("[netd] API registered at $/user/netd\n");
    else
        printf("[netd] API register failed: %d\n", rc);
}