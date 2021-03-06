#
#   Copyright (c) 2012 250bpm s.r.o.
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom
#   the Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#   IN THE SOFTWARE.
#

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library (nanomsg SHARED
    sp.h
    transport.h
    protocol.h
    inproc.h
    ipc.h
    tcp.h
    pair.h
    pubsub.h
    reqrep.h
    fanin.h
    fanout.h
    survey.h

    core/ctx.h
    core/ctx.c
    core/ep.h
    core/ep.c
    core/pipe.c
    core/sock.h
    core/sock.c

    utils/addr.h
    utils/addr.c
    utils/aio.h
    utils/aio.c
    utils/aio_posix.inc
    utils/aio_win.inc
    utils/alloc.h
    utils/alloc.c
    utils/astream.h
    utils/astream.c
    utils/bstream.h
    utils/bstream.c
    utils/clock.h
    utils/clock.c
    utils/cond.h
    utils/cond.c
    utils/cont.h
    utils/cstream.h
    utils/cstream.c
    utils/efd.h
    utils/efd.c
    utils/err.h 
    utils/err.c
    utils/excl.h
    utils/excl.c
    utils/fast.h
    utils/glock.h
    utils/glock.c
    utils/hash.h
    utils/hash.c
    utils/list.h
    utils/list.c
    utils/msg.h
    utils/msg.c
    utils/mutex.h
    utils/mutex.c
    utils/poller.h
    utils/poller.c
    utils/poller_epoll.inc
    utils/poller_kqueue.inc
    utils/poller_poll.inc
    utils/queue.h
    utils/queue.c
    utils/random.h
    utils/random.c
    utils/sleep.h
    utils/sleep.c
    utils/stream.h
    utils/stream.c
    utils/thread.h
    utils/thread.c
    utils/timeout.h
    utils/timeout.c
    utils/wire.h
    utils/wire.c

    protocols/fanin/sink.h
    protocols/fanin/sink.c
    protocols/fanin/source.h
    protocols/fanin/source.c
    protocols/fanin/xsink.h
    protocols/fanin/xsink.c
    protocols/fanin/xsource.h
    protocols/fanin/xsource.c

    protocols/fanout/push.h
    protocols/fanout/push.c
    protocols/fanout/pull.h
    protocols/fanout/pull.c
    protocols/fanout/xpull.h
    protocols/fanout/xpull.c
    protocols/fanout/xpush.h
    protocols/fanout/xpush.c

    protocols/pair/pair.h
    protocols/pair/pair.c
    protocols/pair/xpair.h
    protocols/pair/xpair.c

    protocols/pubsub/pub.h
    protocols/pubsub/pub.c
    protocols/pubsub/sub.h
    protocols/pubsub/sub.c
    protocols/pubsub/trie.h
    protocols/pubsub/trie.c

    protocols/reqrep/req.h
    protocols/reqrep/req.c
    protocols/reqrep/rep.h
    protocols/reqrep/rep.c
    protocols/reqrep/xrep.h
    protocols/reqrep/xrep.c
    protocols/reqrep/xreq.h
    protocols/reqrep/xreq.c

    protocols/survey/respondent.h
    protocols/survey/respondent.c
    protocols/survey/surveyor.h
    protocols/survey/surveyor.c
    protocols/survey/xrespondent.h
    protocols/survey/xrespondent.c
    protocols/survey/xsurveyor.h
    protocols/survey/xsurveyor.c

    transports/inproc/inproc.h
    transports/inproc/inproc_ctx.h
    transports/inproc/inproc_ctx.c
    transports/inproc/inprocb.h
    transports/inproc/inprocb.c
    transports/inproc/inprocc.h
    transports/inproc/inprocc.c
    transports/inproc/msgpipe.h
    transports/inproc/msgpipe.c
    transports/inproc/msgqueue.h
    transports/inproc/msgqueue.c

    transports/ipc/ipc.h
    transports/ipc/ipc.c

    transports/tcp/tcp.h
    transports/tcp/tcp.c
    )

add_definitions (-DSP_EXPORTS)

if (UNIX AND NOT SP_HAVE_OSX)
    target_link_libraries (nanomsg rt)
endif ()

if (SP_HAVE_WINDOWS)
    target_link_libraries (nanomsg ws2_32)
    target_link_libraries (nanomsg Mswsock.lib)
endif ()

if (SP_HAVE_SOLARIS)
    target_link_libraries (nanomsg socket)
    target_link_libraries (nanomsg nsl)
endif ()

target_link_libraries (nanomsg ${CMAKE_THREAD_LIBS_INIT})

install (TARGETS nanomsg DESTINATION lib)

