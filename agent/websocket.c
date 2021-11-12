/*
 * MIT License Copyright (c) 2021, h1zzz
 */

/* https://datatracker.ietf.org/doc/html/rfc6455 */

#include "websocket.h"

#include <string.h>
#include <stdint.h>

#include <mbedtls/base64.h>
#include <mbedtls/sha1.h>

#include "debug.h"
#include "util.h"

#define WEBSOCKET_VERSION "13"
#define WEBSOCKET_GUID    "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define FRAME_FIN    1 << 7
#define FRAME_RSV1   1 << 6
#define FRAME_RSV2   1 << 5
#define FRAME_RSV3   1 << 4
#define FRAME_OPCODE 0xf

#define FRAME_MASK 1 << 7

struct frame_hdr {
    uint8_t fin;
    uint8_t opcode;
    uint8_t mask;
    uint64_t len;
};

/*
 * The request MUST include a header field with the name
 * |Sec-WebSocket-Key|.  The value of this header field MUST be a
 * nonce consisting of a randomly selected 16-byte value that has
 * been base64-encoded (see Section 4 of [RFC4648]).  The nonce
 * MUST be selected randomly for each connection.
 */
static int generate_websocket_key(unsigned char *buf, size_t size)
{
    unsigned char tmp[16];
    size_t i, olen;

    for (i = 0; i < sizeof(tmp); i++)
        tmp[i] = xrand() % 0xff;

    if (mbedtls_base64_encode(buf, size, &olen, tmp, sizeof(tmp)) != 0) {
        debug("mbedtls_base64_encode error");
        return -1;
    }

    return olen;
}

/* https://datatracker.ietf.org/doc/html/rfc6455#section-1.3 */
static int generate_websocket_accept(const unsigned char *ws_key,
                                     unsigned char ac_key[128])
{
    char buf[256] = {0};
    unsigned char output[20];
    int ret;
    mbedtls_sha1_context sha1;
    size_t olen;

    ret = snprintf(buf, sizeof(buf), "%s%s", ws_key, WEBSOCKET_GUID);
    if (ret <= 0 || (size_t)ret >= sizeof(buf)) {
        debug("websocke-key or GUID length limit");
        return -1;
    }

    mbedtls_sha1_init(&sha1);
    mbedtls_sha1_starts(&sha1);
    mbedtls_sha1_update(&sha1, (unsigned char *)buf, (size_t)ret);
    mbedtls_sha1_finish(&sha1, output);
    mbedtls_sha1_free(&sha1);

    if (mbedtls_base64_encode(ac_key, 128, &olen, output, 20) != 0) {
        debug("mbedtls_base64_encode error");
        return -1;
    }

    return (int)olen;
}

static int websocket_handshake(struct websocket *ws, const char *host,
                               const char *path)
{
    unsigned char ws_key[128] = {0}, ac_key[128] = {0};
    char buf[4096] = {0}, *ptr;
    int ret;

    /* Generate sec-websocket-key */
    ret = generate_websocket_key(ws_key, sizeof(ws_key));
    if (ret == -1) {
        debug("generate_websocket_key error");
        return -1;
    }

    /*
     * the WebSocket client's handshake is an HTTP Upgrade request:
     * GET /chat HTTP/1.1
     * Host: server.example.com
     * Upgrade: websocket
     * Connection: Upgrade
     * Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
     * Origin: http://example.com
     * Sec-WebSocket-Protocol: chat, superchat
     * Sec-WebSocket-Version: 13
     */

    /* Build request message */
    ret = snprintf(buf, sizeof(buf),
                   "GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\n"
                   "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
                   "Sec-WebSocket-Version: %s\r\n\r\n",
                   path, host, ws_key, WEBSOCKET_VERSION);
    if (ret <= 0 || (size_t)ret >= sizeof(buf)) {
        debug("snprintf error");
        return -1;
    }

    /* Send websocket handshake request */
    ret = net_write(&ws->net, buf, ret);
    if (ret == -1) {
        debug("net_write error");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    /* Receive the status returned by the websocket server and parse it */
    /* -1 is to prevent overflow of string operations */
    ret = net_read(&ws->net, buf, sizeof(buf) - 1);
    if (ret == -1) {
        debug("net_read error");
        return -1;
    }

    /* TODO: Strictly check the websocket handshake response */

    /* Check status */
    if (memcmp(buf, "HTTP/1.1 101 Switching Protocols", 32) != 0) {
        debugf("request failed, invalid status: %s", buf);
        return -1;
    }

    ret = generate_websocket_accept(ws_key, ac_key);
    if (ret == -1) {
        debug("generate_websocket_accept error");
        return -1;
    }

    ptr = strstr(buf, "Sec-WebSocket-Accept: ");
    if (!ptr) {
        debugf("the handshake failed and the required Sec-WebSocket-Accept was "
               "not found: %s",
               buf);
        return -1;
    }

    ptr += 22; /* strlen("Sec-WebSocket-Accept: "); */

    /* Verify WebSocket-Accept */
    if (memcmp(ptr, ac_key, ret) != 0) {
        debugf("Sec-WebSocket-Accept verification failed: %s", buf);
        return -1;
    }

    return 0;
}

int websocket_connect(struct websocket *ws, const char *host, uint16_t port,
                      const char *path, int tls, const struct proxy *proxy)
{
    int ret;

    memset(ws, 0, sizeof(struct websocket));

    /* Connect to server */
    ret = net_connect(&ws->net, host, port, proxy);
    if (ret == -1) {
        net_close(&ws->net);
        debug("net_connect error");
        return -1;
    }

    if (tls) {
        ret = net_tls_handshake(&ws->net);
        if (ret == -1) {
            net_close(&ws->net);
            debug("net_tls_handshake error");
            return -1;
        }
    }

    ret = websocket_handshake(ws, host, path);
    if (ret == -1) {
        net_close(&ws->net);
        debug("websocket_handshake error");
        return -1;
    }

    return 0;
}

/*
 *   0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-------+-+-------------+-------------------------------+
 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 * | |1|2|3|       |K|             |                               |
 * +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 * |     Extended payload length continued, if payload len == 127  |
 * + - - - - - - - - - - - - - - - +-------------------------------+
 * |                               |Masking-key, if MASK set to 1  |
 * +-------------------------------+-------------------------------+
 * | Masking-key (continued)       |          Payload Data         |
 * +-------------------------------- - - - - - - - - - - - - - - - +
 * :                     Payload Data continued ...                :
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * |                     Payload Data continued ...                |
 * +---------------------------------------------------------------+
 */

static int websocket_read_frame_hdr(struct websocket *ws, struct frame_hdr *hdr)
{
    unsigned char buf[8] = {0};
    int ret;
    uint64_t len;

    ret = net_readn(&ws->net, buf, 2);
    if (ret == -1) {
        debug("net_readn error");
        return -1;
    }

    hdr->fin = buf[0] & FRAME_FIN;
    hdr->opcode = buf[0] & FRAME_OPCODE;

    /*
     * MUST be 0 unless an extension is negotiated that defines meanings
     * for non-zero values.  If a nonzero value is received and none of
     * the negotiated extensions defines the meaning of such a nonzero
     * value, the receiving endpoint MUST _Fail the WebSocket Connection_.
     */
    if ((buf[0] & (FRAME_RSV1 | FRAME_RSV2 | FRAME_RSV3)) != 0) {
        debug("RSVx reserved field, must be 0");
        return -1;
    }

    hdr->mask = buf[1] & FRAME_MASK;
    /* 0x7f = 0111 1111, Take the value of the lower seven bits */
    /* if 0-125, that is the payload length. */
    len = buf[1] & 0x7f;

    /* Multibyte length quantities are expressed in network byte order. */

    if (len == 126) {
        /*
         * If 126, the following 2 bytes interpreted as a 16-bit unsigned
         * integer are the payload length.
         */
        ret = net_readn(&ws->net, buf, 2);
        if (ret == -1) {
            debug("net_readn error");
            return -1;
        }
        len = htons(*(uint16_t *)buf);
    } else if (len == 127) {
        /*
         * If 127, the following 8 bytes interpreted as a 64-bit unsigned
         * integer (the most significant bit MUST be 0) are the payload length.
         */
        ret = net_readn(&ws->net, buf, 8);
        if (ret == -1) {
            debug("net_readn error");
            return -1;
        }
        len = htonll(*(uint64_t *)buf);
    }

    hdr->len = len;

    return 0;
}

static int websocket_skip_remaining(struct websocket *ws)
{
    char buf[1024];
    size_t n;
    int ret;

    while (ws->remaining > 0) {
        n = ws->remaining > sizeof(buf) ? sizeof(buf) : ws->remaining;
        ret = net_readn(&ws->net, buf, n);
        if (ret == -1) {
            debug("net_readn error");
            return -1;
        }
        ws->remaining -= ret;
    }

    return 0;
}

int websocket_recv(struct websocket *ws, int *type, void *buf, size_t n)
{
    struct frame_hdr hdr;
    unsigned char mask_key[4];
    int ret;

    /* Skip the remaining unread data */
    if (ws->remaining > 0) {
        ret = websocket_skip_remaining(ws);
        if (ret == -1) {
            debug("websocket_skip_remaing error");
            return -1;
        }
    }

    ret = websocket_read_frame_hdr(ws, &hdr);
    if (ret == -1) {
        debug("websocket_read_frame_hdr error");
        return -1;
    }

    /*
     * For the current project, the size of the data transmitted in one frame is
     * sufficient, and the frame transmission is not supported for the time
     * being
     */
    if (hdr.fin == 0) {
        debug("no support continuation frame");
        return -1;
    }

    if (type)
        *type = hdr.opcode;

    ws->remaining = hdr.len;

    /*
     * Masking-key: 0 or 4 bytes
     * All frames sent from the client to the server are masked by a
     * 32-bit value that is contained within the frame.  This field is
     * present if the mask bit is set to 1 and is absent if the mask bit
     * is set to 0.
     * See Section 5.3 for further information on client-to-server masking.
     */
    if (hdr.mask) {
        ret = net_readn(&ws->net, mask_key, sizeof(mask_key));
        if (ret == -1) {
            debug("net_readn error");
            return -1;
        }
    }

    /* TODO: https://datatracker.ietf.org/doc/html/rfc6455#section-5.3 */
    ret = net_readn(&ws->net, buf, n > ws->remaining ? ws->remaining : n);
    if (ret == -1) {
        debug("net_readn error");
        return -1;
    }

    ws->remaining -= ret;

    return ret;
}

void websokcet_close(struct websocket *ws)
{
    net_close(&ws->net);
}
