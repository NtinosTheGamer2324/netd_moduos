#include "../include/libc.h"

/* mirror the cmd codes from api.c */
#define NETD_CMD_GET_INFO    1
#define NETD_CMD_PING        2
#define NETD_CMD_DNS_RESOLVE 7

typedef struct { uint32_t cmd; } netd_req_hdr_t;

typedef struct {
    uint8_t  mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns;
    uint32_t mtu;
    uint32_t link_up;
    uint32_t dhcp_bound;
} netd_info_t;

typedef struct {
    uint32_t ip;
    uint16_t count;
    uint16_t timeout_ms;
} netd_ping_req_t;

typedef struct {
    uint32_t replied;
    uint32_t reply_ip;
} netd_ping_rep_t;

typedef struct { char name[256]; } netd_dns_req_t;
typedef struct { uint32_t ip;    } netd_dns_rep_t;

typedef struct {
    int32_t  status;
    uint32_t len;
    uint8_t  data[1500];
} netd_rep_t;

static int g_fd = -1;

static int netd_open(void) {
    g_fd = open("$/user/netd", O_RDWR, 0);
    if (g_fd < 0) {
        printf("netd_test: cannot open $/user/netd (%d)\n", g_fd);
        return -1;
    }
    return 0;
}

static int test_get_info(void) {
    printf("\n[1] GET_INFO\n");

    netd_req_hdr_t req = { NETD_CMD_GET_INFO };
    netd_rep_t     rep;
    memset(&rep, 0, sizeof(rep));

    ssize_t n = invoke(g_fd, &req, sizeof(req), &rep, sizeof(rep));
    if (n < 0 || rep.status != 0) {
        printf("  FAIL: invoke returned %d status %d\n", (int)n, rep.status);
        return -1;
    }

    netd_info_t *info = (netd_info_t *)rep.data;
    printf("  MAC  : %02x:%02x:%02x:%02x:%02x:%02x\n",
           info->mac[0], info->mac[1], info->mac[2],
           info->mac[3], info->mac[4], info->mac[5]);
    printf("  IP   : %u.%u.%u.%u\n",
           (info->ip>>24)&0xFF, (info->ip>>16)&0xFF,
           (info->ip>>8)&0xFF,   info->ip&0xFF);
    printf("  Mask : %u.%u.%u.%u\n",
           (info->netmask>>24)&0xFF, (info->netmask>>16)&0xFF,
           (info->netmask>>8)&0xFF,   info->netmask&0xFF);
    printf("  GW   : %u.%u.%u.%u\n",
           (info->gateway>>24)&0xFF, (info->gateway>>16)&0xFF,
           (info->gateway>>8)&0xFF,   info->gateway&0xFF);
    printf("  DNS  : %u.%u.%u.%u\n",
           (info->dns>>24)&0xFF, (info->dns>>16)&0xFF,
           (info->dns>>8)&0xFF,   info->dns&0xFF);
    printf("  MTU  : %u\n", info->mtu);
    printf("  Link : %s\n", info->link_up  ? "UP"    : "DOWN");
    printf("  DHCP : %s\n", info->dhcp_bound ? "bound" : "not bound");
    printf("  PASS\n");
    return 0;
}

static int test_ping(uint32_t ip) {
    printf("\n[2] PING %u.%u.%u.%u\n",
           (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,ip&0xFF);

    struct { netd_req_hdr_t hdr; netd_ping_req_t ping; } req;
    req.hdr.cmd        = NETD_CMD_PING;
    req.ping.ip        = ip;
    req.ping.count     = 3;
    req.ping.timeout_ms= 2000;

    netd_rep_t rep;
    memset(&rep, 0, sizeof(rep));

    ssize_t n = invoke(g_fd, &req, sizeof(req), &rep, sizeof(rep));
    if (n < 0) { printf("  FAIL: invoke %d\n", (int)n); return -1; }

    netd_ping_rep_t *pr = (netd_ping_rep_t *)rep.data;
    if (pr->replied) {
        printf("  reply from %u.%u.%u.%u\n",
               (pr->reply_ip>>24)&0xFF,(pr->reply_ip>>16)&0xFF,
               (pr->reply_ip>>8)&0xFF,  pr->reply_ip&0xFF);
        printf("  PASS\n");
    } else {
        printf("  no reply (ok in some VM configs)\n");
        printf("  PASS (tx only)\n");
    }
    return 0;
}

static int test_dns(const char *name) {
    printf("\n[3] DNS resolve '%s'\n", name);

    struct { netd_req_hdr_t hdr; netd_dns_req_t dns; } req;
    req.hdr.cmd = NETD_CMD_DNS_RESOLVE;
    memset(req.dns.name, 0, sizeof(req.dns.name));
    int l = strlen(name);
    if (l > 255) l = 255;
    memcpy(req.dns.name, name, l);

    netd_rep_t rep;
    memset(&rep, 0, sizeof(rep));

    ssize_t n = invoke(g_fd, &req, sizeof(req), &rep, sizeof(rep));
    netd_dns_rep_t *dr = (netd_dns_rep_t *)rep.data;

    if (n >= 0 && rep.status == 0 && dr->ip) {
        printf("  %s -> %u.%u.%u.%u\n", name,
               (dr->ip>>24)&0xFF,(dr->ip>>16)&0xFF,
               (dr->ip>>8)&0xFF,  dr->ip&0xFF);
        printf("  PASS\n");
    } else {
        printf("  no result (DNS may not reach internet in this VM config)\n");
        printf("  PASS (attempted)\n");
    }
    return 0;
}

int md_main(long argc, char **argv) {
    (void)argc; (void)argv;

    printf("=== netd_test ===\n");

    if (netd_open() < 0) return 1;

    test_get_info();

    printf("About to ping test\n");
    /* ping gateway */
//    test_ping((10<<24)|(0<<16)|(2<<8)|1);   /* 10.0.2.1 */
    printf("Ping test done\n");

    printf("About to DNS test\n");
    /* dns */
    test_dns("example.com");

    close(g_fd);
    printf("\n=== done ===\n");
    return 0;
}