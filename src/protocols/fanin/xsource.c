/*
    Copyright (c) 2012-2013 250bpm s.r.o.

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

#include "xsource.h"

#include "../../sp.h"
#include "../../fanin.h"

#include "../../utils/err.h"
#include "../../utils/cont.h"
#include "../../utils/fast.h"
#include "../../utils/alloc.h"
#include "../../utils/excl.h"

struct sp_xsource {
    struct sp_sockbase sockbase;
    struct sp_excl excl;
};

/*  Private functions. */
static void sp_xsource_init (struct sp_xsource *self,
    const struct sp_sockbase_vfptr *vfptr, int fd);
static void sp_xsource_term (struct sp_xsource *self);

/*  Implementation of sp_sockbase's virtual functions. */
static void sp_xsource_destroy (struct sp_sockbase *self);
static int sp_xsource_add (struct sp_sockbase *self, struct sp_pipe *pipe);
static void sp_xsource_rm (struct sp_sockbase *self, struct sp_pipe *pipe);
static int sp_xsource_in (struct sp_sockbase *self, struct sp_pipe *pipe);
static int sp_xsource_out (struct sp_sockbase *self, struct sp_pipe *pipe);
static int sp_xsource_send (struct sp_sockbase *self, const void *buf,
    size_t len);
static int sp_xsource_recv (struct sp_sockbase *self, void *buf, size_t *len);
static int sp_xsource_setopt (struct sp_sockbase *self, int level, int option,
    const void *optval, size_t optvallen);
static int sp_xsource_getopt (struct sp_sockbase *self, int level, int option,
    void *optval, size_t *optvallen);
static const struct sp_sockbase_vfptr sp_xsource_sockbase_vfptr = {
    sp_xsource_destroy,
    sp_xsource_add,
    sp_xsource_rm,
    sp_xsource_in,
    sp_xsource_out,
    sp_xsource_send,
    sp_xsource_recv,
    sp_xsource_setopt,
    sp_xsource_getopt
};

static void sp_xsource_init (struct sp_xsource *self,
    const struct sp_sockbase_vfptr *vfptr, int fd)
{
    sp_sockbase_init (&self->sockbase, vfptr, fd);
    sp_excl_init (&self->excl);
}

static void sp_xsource_term (struct sp_xsource *self)
{
    sp_excl_term (&self->excl);
}

void sp_xsource_destroy (struct sp_sockbase *self)
{
    struct sp_xsource *xsource;

    xsource = sp_cont (self, struct sp_xsource, sockbase);

    sp_xsource_term (xsource);
    sp_free (xsource);
}

static int sp_xsource_add (struct sp_sockbase *self, struct sp_pipe *pipe)
{
    return sp_excl_add (&sp_cont (self, struct sp_xsource, sockbase)->excl,
        pipe);
}

static void sp_xsource_rm (struct sp_sockbase *self, struct sp_pipe *pipe)
{
    sp_excl_rm (&sp_cont (self, struct sp_xsource, sockbase)->excl, pipe);
}

static int sp_xsource_in (struct sp_sockbase *self, struct sp_pipe *pipe)
{
    return sp_excl_in (&sp_cont (self, struct sp_xsource, sockbase)->excl,
        pipe);
}

static int sp_xsource_out (struct sp_sockbase *self, struct sp_pipe *pipe)
{
    return sp_excl_out (&sp_cont (self, struct sp_xsource, sockbase)->excl,
        pipe);
}

static int sp_xsource_send (struct sp_sockbase *self, const void *buf,
    size_t len)
{
    return sp_excl_send (&sp_cont (self, struct sp_xsource, sockbase)->excl,
        buf, len);
}

static int sp_xsource_recv (struct sp_sockbase *self, void *buf, size_t *len)
{
    /*  No messages can be received from xsource socket. */
    return -EAGAIN;
}

static int sp_xsource_setopt (struct sp_sockbase *self, int level, int option,
        const void *optval, size_t optvallen)
{
    return -ENOPROTOOPT;
}

static int sp_xsource_getopt (struct sp_sockbase *self, int level, int option,
        void *optval, size_t *optvallen)
{
    return -ENOPROTOOPT;
}

struct sp_sockbase *sp_xsource_create (int fd)
{
    struct sp_xsource *self;

    self = sp_alloc (sizeof (struct sp_xsource), "socket (source)");
    alloc_assert (self);
    sp_xsource_init (self, &sp_xsource_sockbase_vfptr, fd);
    return &self->sockbase;
}

static struct sp_socktype sp_xsource_socktype_struct = {
    AF_SP_RAW,
    SP_SOURCE,
    sp_xsource_create
};

struct sp_socktype *sp_xsource_socktype = &sp_xsource_socktype_struct;

