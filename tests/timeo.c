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

#include "../src/sp.h"
#include "../src/pair.h"

#include "../src/utils/err.c"
#include "../src/utils/stopwatch.c"

int main ()
{
    int rc;
    int s;
    int timeo;
    char buf [3];
    struct sp_stopwatch stopwatch;
    uint64_t elapsed;

    rc = sp_init ();
    errno_assert (rc == 0);
    s = sp_socket (AF_SP, SP_PAIR);
    errno_assert (s != -1);

    timeo = 100;
    rc = sp_setsockopt (s, SP_SOL_SOCKET, SP_RCVTIMEO, &timeo, sizeof (timeo));
    errno_assert (rc == 0);
    sp_stopwatch_init (&stopwatch);
    rc = sp_recv (s, buf, sizeof (buf), 0);
    elapsed = sp_stopwatch_term (&stopwatch);
    errno_assert (rc < 0 && sp_errno () == EAGAIN);
    time_assert (elapsed, 100000);

    timeo = 100;
    rc = sp_setsockopt (s, SP_SOL_SOCKET, SP_SNDTIMEO, &timeo, sizeof (timeo));
    errno_assert (rc == 0);
    rc = sp_send (s, "ABC", 3, 0);
    errno_assert (rc < 0 && sp_errno () == EAGAIN);
    time_assert (elapsed, 100000);

    rc = sp_close (s);
    errno_assert (rc == 0);
    rc = sp_term ();
    errno_assert (rc == 0);

    return 0;
}

