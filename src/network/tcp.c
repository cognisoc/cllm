#include "net.h"
#include "../../include/kernel.h"
#include "../../include/string.h"
#include "../../include/network.h"
#include "../../include/api_v1.h"

#define TCP_MAX_SOCKETS 4
#define TCP_RX_BUF_SIZE 2048
#define TCP_TX_BUF_SIZE 2048
#define HTTP_PORT 8080

typedef enum {
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

typedef struct {
    tcp_state_t state;
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t  rx_buf[TCP_RX_BUF_SIZE];
    size_t   rx_len;
    uint8_t  tx_buf[TCP_TX_BUF_SIZE];
    size_t   tx_len;
    uint32_t last_activity;
} tcp_socket_t;

static tcp_socket_t sockets[TCP_MAX_SOCKETS];
static uint32_t tcp_now = 0;

static inline uint16_t be16(uint16_t x) { return (x >> 8) | (x << 8); }
static inline uint32_t be32(uint32_t x) {
    return ((x >> 24) & 0xFF) | (((x >> 16) & 0xFF) << 8) |
           (((x >> 8) & 0xFF) << 16) | ((x & 0xFF) << 24);
}

static tcp_socket_t* socket_alloc(void) {
    for (int i = 0; i < TCP_MAX_SOCKETS; i++) {
        if (sockets[i].state == TCP_CLOSED) {
            memset(&sockets[i], 0, sizeof(tcp_socket_t));
            return &sockets[i];
        }
    }
    return NULL;
}

static tcp_socket_t* socket_lookup(uint32_t remote_ip, uint16_t remote_port,
                                    uint16_t local_port, int allow_any_remote_ip) {
    for (int i = 0; i < TCP_MAX_SOCKETS; i++) {
        tcp_socket_t* s = &sockets[i];
        if (s->state == TCP_CLOSED) continue;
        if (s->local_port != local_port) continue;
        if (s->remote_port != remote_port) continue;
        if (!allow_any_remote_ip && s->remote_ip != 0 && s->remote_ip != remote_ip) continue;
        return s;
    }
    return NULL;
}

static tcp_socket_t* listener_lookup(uint16_t local_port) {
    for (int i = 0; i < TCP_MAX_SOCKETS; i++) {
        if (sockets[i].state == TCP_LISTEN && sockets[i].local_port == local_port) {
            return &sockets[i];
        }
    }
    return NULL;
}

static void socket_close(tcp_socket_t* s) {
    s->state = TCP_CLOSED;
    s->rx_len = 0;
    s->tx_len = 0;
}

void tcp_init(void) {
    memset(sockets, 0, sizeof(sockets));
    tcp_socket_t* listen = socket_alloc();
    if (listen) {
        listen->state = TCP_LISTEN;
        listen->local_port = HTTP_PORT;
        listen->local_ip = NET_IP_ADDR;
    }
}

static void tcp_send_ctrl(tcp_socket_t* s, uint8_t flags) {
    net_send_tcp(s->local_port, s->remote_port, s->remote_ip,
                 s->seq, s->ack, flags, NULL, 0);
}

static void tcp_send_data(tcp_socket_t* s, const uint8_t* data, size_t len) {
    net_send_tcp(s->local_port, s->remote_port, s->remote_ip,
                 s->seq, s->ack, TCP_FLAG_ACK | TCP_FLAG_PSH, data, len);
    s->seq += (uint32_t)len;
}

uint32_t tcp_current_remote_ip = 0;

void tcp_receive(const uint8_t* packet, size_t len) {
    if (len < TCP_HDR_LEN) return;
    const tcp_header_t* tcp = (const tcp_header_t*)packet;
    uint16_t src_port = be16(tcp->src_port);
    uint16_t dst_port = be16(tcp->dst_port);
    uint32_t seq = be32(tcp->seq);
    uint32_t ack = be32(tcp->ack);
    uint8_t  flags = tcp->flags;
    uint16_t hdr_len = (tcp->data_offset >> 4) * 4;
    const uint8_t* payload = packet + hdr_len;
    size_t payload_len = len > hdr_len ? len - hdr_len : 0;
    uint32_t remote_ip = tcp_current_remote_ip;

    tcp_socket_t* s = socket_lookup(remote_ip, src_port, dst_port, 0);
    if (!s && (flags & TCP_FLAG_SYN)) {
        tcp_socket_t* listener = listener_lookup(dst_port);
        if (listener) {
            s = socket_alloc();
            if (s) {
                s->state = TCP_SYN_RECEIVED;
                s->local_port = dst_port;
                s->remote_port = src_port;
                s->remote_ip = remote_ip;
                // Use a simple initial sequence number based on the socket index and time.
                s->seq = 1000 + (uint32_t)(s - sockets) * 100 + (tcp_now & 0xFFFF);
            }
        }
    }
    if (!s) {
        // Send RST for unknown connection.
        if (!(flags & TCP_FLAG_RST)) {
            net_send_tcp(dst_port, src_port, remote_ip, 0,
                         seq + 1 + ((flags & TCP_FLAG_SYN) ? 1 : 0),
                         TCP_FLAG_RST, NULL, 0);
        }
        return;
    }

    if (s->remote_ip == 0) {
        s->remote_ip = remote_ip;
    }

    s->last_activity = tcp_now;

    switch (s->state) {
        case TCP_LISTEN:
            break;

        case TCP_SYN_RECEIVED:
            {
                uint32_t ack_seq = seq + 1;
                s->ack = ack_seq;
                if (flags & TCP_FLAG_ACK) {
                    // Final ACK of the three-way handshake.
                    s->state = TCP_ESTABLISHED;
                    s->seq = ack;
                    tcp_send_ctrl(s, TCP_FLAG_ACK);
                } else {
                    // SYN request: reply with SYN-ACK.
                    tcp_send_ctrl(s, TCP_FLAG_SYN | TCP_FLAG_ACK);
                }
            }
            break;

        case TCP_ESTABLISHED:
        case TCP_CLOSE_WAIT:
            if (flags & TCP_FLAG_SYN) {
                // Ignore stray SYN on established connection.
                break;
            }
            s->ack = seq + (uint32_t)payload_len;
            if (flags & TCP_FLAG_FIN) {
                s->ack++;
                tcp_send_ctrl(s, TCP_FLAG_ACK);
                s->state = (s->state == TCP_ESTABLISHED) ? TCP_CLOSE_WAIT : TCP_LAST_ACK;
                if (s->state == TCP_LAST_ACK) {
                    socket_close(s);
                }
                break;
            }
            if (!(flags & TCP_FLAG_ACK)) {
                // Need ACK to process data.
                break;
            }
            if (payload_len > 0 && s->rx_len + payload_len < TCP_RX_BUF_SIZE) {
                memcpy(s->rx_buf + s->rx_len, payload, payload_len);
                s->rx_len += payload_len;
            }
            if (payload_len > 0) {
                // ACK data immediately so the sender can continue.
                tcp_send_ctrl(s, TCP_FLAG_ACK);
            }
            if (s->rx_len > 0) {
                // Try to process a complete HTTP request.
                char* end = NULL;
                for (size_t i = 0; i + 3 < s->rx_len; i++) {
                    if (s->rx_buf[i] == '\r' && s->rx_buf[i+1] == '\n' &&
                        s->rx_buf[i+2] == '\r' && s->rx_buf[i+3] == '\n') {
                        end = (char*)(s->rx_buf + i + 4);
                        break;
                    }
                }
                if (end) {
                    http_request_t req;
                    http_response_t resp;
                    memset(&req, 0, sizeof(req));
                    memset(&resp, 0, sizeof(resp));
                    size_t request_len = (size_t)(end - (char*)s->rx_buf);
                    if (http_parse_request((const char*)s->rx_buf, request_len, &req) == 0) {
                        route_request(&req, &resp);
                    } else {
                        resp.status_code = 400;
                        strncpy(resp.headers, "Content-Type: application/json\r\n", sizeof(resp.headers));
                        strncpy(resp.body, "{\"error\":\"bad request\"}", sizeof(resp.body));
                        resp.body_length = strlen(resp.body);
                    }

                    char response_buf[HTTP_RESPONSE_BODY_SIZE + 256];
                    size_t response_len = sizeof(response_buf);
                    if (http_generate_response(&resp, response_buf, &response_len) == 0) {
                        tcp_send_data(s, (const uint8_t*)response_buf, response_len);
                    }

                    // Drop any remaining RX data; this is a single-request-per-connection server.
                    s->rx_len = 0;

                    // Close connection after one request (HTTP/1.0 style).
                    s->state = TCP_FIN_WAIT_1;
                    tcp_send_ctrl(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
                }
            } else {
                tcp_send_ctrl(s, TCP_FLAG_ACK);
            }
            break;

        case TCP_FIN_WAIT_1:
            if (flags & TCP_FLAG_FIN) {
                s->ack++;
                s->state = TCP_TIME_WAIT;
            } else if (flags & TCP_FLAG_ACK) {
                s->state = TCP_FIN_WAIT_2;
            }
            break;

        case TCP_FIN_WAIT_2:
            if (flags & TCP_FLAG_FIN) {
                s->ack++;
                tcp_send_ctrl(s, TCP_FLAG_ACK);
                s->state = TCP_TIME_WAIT;
                socket_close(s);
            }
            break;

        case TCP_LAST_ACK:
            if (flags & TCP_FLAG_ACK) {
                socket_close(s);
            }
            break;

        default:
            break;
    }
}

void tcp_periodic(void) {
    tcp_now++;
    // Retransmit pending SYN-ACK responses.
    for (int i = 0; i < TCP_MAX_SOCKETS; i++) {
        tcp_socket_t* s = &sockets[i];
        if (s->state == TCP_SYN_RECEIVED && (tcp_now - s->last_activity) > 1) {
            // Retry SYN-ACK.
            tcp_send_ctrl(s, TCP_FLAG_SYN | TCP_FLAG_ACK);
            s->last_activity = tcp_now;
        }
    }
}
