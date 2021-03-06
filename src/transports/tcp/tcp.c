/*
    Copyright (c) 2012 250bpm s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "tcp.h"

#include "../../utils/err.h"
#include "../../utils/addr.h"
#include "../../utils/alloc.h"
#include "../../utils/fast.h"
#include "../../utils/bstream.h"
#include "../../utils/cstream.h"

#define SP_TCP_BACKLOG 100

/*  Private functions. */
static int sp_tcp_binit (const char *addr, struct sp_usock *usock,
    struct sp_cp *cp, int backlog);
static int sp_tcp_csockinit (struct sp_usock *usock, int sndbuf, int rcvbuf,
    struct sp_cp *cp);
static int sp_tcp_cresolve (const char *addr, struct sockaddr_storage *ss,
    socklen_t *sslen);

/*  sp_transport interface. */
static const char *sp_tcp_name (void);
static void sp_tcp_init (void);
static void sp_tcp_term (void);
static int sp_tcp_bind (const char *addr, void *hint,
    struct sp_epbase **epbase);
static int sp_tcp_connect (const char *addr, void *hint,
    struct sp_epbase **epbase);

static struct sp_transport sp_tcp_vfptr = {
    sp_tcp_name,
    sp_tcp_init,
    sp_tcp_term,
    sp_tcp_bind,
    sp_tcp_connect
};

struct sp_transport *sp_tcp = &sp_tcp_vfptr;

static const char *sp_tcp_name (void)
{
    return "tcp";
}

static void sp_tcp_init (void)
{
}

static void sp_tcp_term (void)
{
}

static int sp_tcp_bind (const char *addr, void *hint,
    struct sp_epbase **epbase)
{
    int rc;
    struct sp_bstream *bstream;

    bstream = sp_alloc (sizeof (struct sp_bstream), "bstream (tcp)");
    alloc_assert (bstream);
    rc = sp_bstream_init (bstream, addr, hint, sp_tcp_binit, SP_TCP_BACKLOG);
    if (sp_slow (rc != 0)) {
        sp_free (bstream);
        return rc;
    }
    *epbase = &bstream->epbase;

    return 0;
}

static int sp_tcp_connect (const char *addr, void *hint,
    struct sp_epbase **epbase)
{
    int rc;
    struct sp_cstream *cstream;

    /*  TODO: Check the syntax of the address here! */

    cstream = sp_alloc (sizeof (struct sp_cstream), "cstream (tcp)");
    alloc_assert (cstream);
    rc = sp_cstream_init (cstream, addr, hint, sp_tcp_csockinit,
        sp_tcp_cresolve);
    if (sp_slow (rc != 0)) {
        sp_free (cstream);
        return rc;
    }
    *epbase = &cstream->epbase;

    return 0;
}

static int sp_tcp_binit (const char *addr, struct sp_usock *usock,
    struct sp_cp *cp, int backlog)
{
    int rc;
    int port;
    const char *colon;
    struct sockaddr_storage ss;
    socklen_t sslen;

    /*  Make sure we're working from a clean slate. Required on Mac OS X. */
    memset (&ss, 0, sizeof (ss));

    /*  Parse the port. */
    rc = sp_addr_parse_port (addr, &colon);
    if (rc < 0)
        return rc;
    port = rc;

    /*  Parse the address. */
    /*  TODO:  Get the actual value of the IPV4ONLY socket option. */
    rc = sp_addr_parse_local (addr, colon - addr, SP_ADDR_IPV4ONLY,
        &ss, &sslen);
    if (rc < 0)
        return rc;

    /*  Combine the port and the address. */
    if (ss.ss_family == AF_INET)
        ((struct sockaddr_in*) &ss)->sin_port = htons (port);
    else if (ss.ss_family == AF_INET6)
        ((struct sockaddr_in6*) &ss)->sin6_port = htons (port);
    else
        sp_assert (0);

    /*  Open the listening socket. */
    rc = sp_usock_init (usock, NULL, AF_INET, SOCK_STREAM, IPPROTO_TCP,
        -1, -1, cp);
    errnum_assert (rc == 0, -rc);
    rc = sp_usock_listen (usock, (struct sockaddr*) &ss, sslen, SP_TCP_BACKLOG);
    errnum_assert (rc == 0, -rc);

    return 0;
}

static int sp_tcp_csockinit (struct sp_usock *usock, int sndbuf, int rcvbuf,
    struct sp_cp *cp)
{
    return sp_usock_init (usock, NULL, AF_INET, SOCK_STREAM, IPPROTO_TCP,
        sndbuf, rcvbuf, cp);
}

static int sp_tcp_cresolve (const char *addr, struct sockaddr_storage *ss,
    socklen_t *sslen)
{
    int rc;
    int port;
    const char *colon;

    /*  Make sure we're working from a clean slate. Required on Mac OS X. */
    memset (ss, 0, sizeof (struct sockaddr_storage));

    /*  Parse the port. */
    port = sp_addr_parse_port (addr, &colon);
    errnum_assert (port > 0, -port);

    /*  TODO: Parse the local address, if any. */

    /*  Parse the remote address. */
    /*  TODO:  Get the actual value of the IPV4ONLY socket option. */
    rc = sp_addr_parse_remote (addr, colon - addr, SP_ADDR_IPV4ONLY,
        ss, sslen);
    if (sp_slow (rc < 0))
        return rc;

    /*  Combine the port and the address. */
    if (ss->ss_family == AF_INET)
        ((struct sockaddr_in*) ss)->sin_port = htons (port);
    else if (ss->ss_family == AF_INET6)
        ((struct sockaddr_in6*) ss)->sin6_port = htons (port);
    else
        sp_assert (0);

    return 0;
}

