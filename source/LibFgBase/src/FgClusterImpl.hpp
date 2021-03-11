//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Implementation file to be included in OS-specific libraries

#include <boost/asio.hpp>
#include "FgCluster.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"

using namespace boost::asio;

namespace Fg {

// TCP provides a full duplex stream but no mechanism for discrete messages (although each direction of
// the stream can be closed separately) so we do our own 'framing' of messsages using a size-block format
// in the functions below:

static
void
sendFrame(ip::tcp::socket & sock,String const & msg)
{
    boost::system::error_code err;
    size_t              msgSz = msg.size();
    write(sock,buffer(&msgSz,8),err);
    FGASSERT(!err);
    write(sock,buffer(msg.data(),msgSz),err);
    FGASSERT(!err);
}

// Returns 'false' if connection closed by sender
static
bool
recvFrame(ip::tcp::socket & sock,String & msg)
{
    boost::system::error_code err;
    size_t              msgSz,
                        szRead;
    // 'read' makes repeated use of sock::read_some (== receive) to read the requested amount:
    szRead = read(sock,buffer(&msgSz,8),err);
    if (err == error::eof)
        return false;
    FGASSERT(!err);
    FGASSERT(szRead == 8);
    FGASSERT(msgSz <= 0x02000000);      // < 32MB sanity check
    msg.resize(msgSz);
    szRead = read(sock,buffer(&msg[0],msgSz),err);
    if (err == error::eof)
        return false;
    FGASSERT(!err);
    FGASSERT(szRead == msgSz);
    return true;
}

// The worker can receive and send messages in the same thread as the dispatcher won't send
// another message until it receives its response:
void
fgClustWorker(FgFnStr2Str handler,uint16 port)
{
    io_service              ios;                    // Initialize networking functionality
    ip::tcp::endpoint       ep(ip::tcp::v4(),port);
    ip::tcp::acceptor       acc(ios,ep);
    ip::tcp::socket         sock(ios);
    acc.accept(sock);
    boost::system::error_code err;
    for (;;) {
        String              msg;
        if (!recvFrame(sock,msg))                   // Can block for a long time
            return;                                 // Connection closed, terminate
        String              resp = handler(msg);    // Can take a long time before returning
        sendFrame(sock,resp);
    }
}

// If an exception is thrown, store it's details in the message:
void
recvFrameThread(ip::tcp::socket & sock,String & msg)
{
    bool    connectionOpen = true;
    try {
        connectionOpen = recvFrame(sock,msg);
    }
    catch(FgException const & e) {
        msg = e.tr_message();
    }
    catch(std::exception const & e) {
        msg = String("Standard library exception\n")+e.what();
    }
    catch(...) {
        msg = "Unknown exception type";
    }
    if (!connectionOpen)
        msg = "Worker closed connection";
}

struct  FgClustDispatcherImpl : FgClustDispatcher
{
    typedef std::unique_ptr<ip::tcp::socket>    SockPtr;

    io_service              ios;
    Svec<SockPtr>         sockPtrs;

    FgClustDispatcherImpl(Strings const & hosts,String const & port)
    {
        sockPtrs.reserve(hosts.size());
        for (size_t hh=0; hh<hosts.size(); ++hh) {
            ip::tcp::resolver               res(ios);
            ip::tcp::resolver::query        query(ip::tcp::v4(),hosts[hh],port);
            ip::tcp::resolver::iterator     iter(res.resolve(query));
            sockPtrs.emplace_back(std::unique_ptr<ip::tcp::socket>(new ip::tcp::socket(ios)));
            boost::system::error_code       err;
            connect(*sockPtrs[hh],iter,err);
            if (err)
                fgThrow("Cluster dispatch connect failed",hosts[hh]);
        }
    }

    virtual
    size_t numMachines() const
    {return sockPtrs.size(); }

    virtual
    void
    batchProcess(Strings const & msgsSend,Strings & msgsRecv) const
    {
        FGASSERT(msgsSend.size() == sockPtrs.size());
        msgsRecv.resize(msgsSend.size());
        // Start the receive threads before sending in case of long send and short return times.
        // Since workers will finish in different times, we cannot receive sequentially (not just due to
        // inefficiencies but also because input buffers could overflow) so each send is a thread:
        Svec<std::thread>     recvThreads;
        recvThreads.reserve(msgsSend.size());
        for (size_t mm=0; mm<msgsSend.size(); ++mm)
            recvThreads.push_back(std::thread(recvFrameThread,
                std::ref(*sockPtrs[mm]),
                std::ref(msgsRecv[mm])));
        // Since there's only one physical ethernet cable and recipients are close by we just send each
        // message sequentially. A possible future optimization would be to make this asynchronous with some
        // number of threads (via asio):
        for (size_t mm=0; mm<msgsSend.size(); ++mm)
            sendFrame(*sockPtrs[mm],msgsSend[mm]);
        for (size_t mm=0; mm<recvThreads.size(); ++mm)
            recvThreads[mm].join();
        // Check for errors within the receive threads;
        static String       hdrSer = "\26\0\\0\0\0\0\0\0serialization::archive";    // Character literals in octal
        for (size_t mm=0; mm<msgsRecv.size(); ++mm)
            if (!beginsWith(msgsRecv[mm],hdrSer))
                fgThrow("Cluster worker "+toStr(mm),msgsRecv[mm]);
    }
};

std::shared_ptr<FgClustDispatcher>
fgClustDispatcher(Strings const & hostnames,uint16 port)
{
    return std::make_shared<FgClustDispatcherImpl>(hostnames,toStr(port));
}

}

// */
