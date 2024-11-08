//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Using Posix sockets, which are essentially Berkeley sockets.
//
// Interesting: https://brooker.co.za/blog/2024/05/09/nagle.html

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#include "FgTcp.hpp"
#include "FgScopeGuard.hpp"
#include "FgFileSystem.hpp"

// Do NOT use std namespace to avoid collision with posix 'bind'

namespace Fg {

bool                runTcpClient_(
    String const &          hostname,
    uint16                  port,
    Bytes const &           data,
    Bytes *                 responsePtr)
{
    int                     clientSock = socket(
        AF_INET,            // IPv4 protocol family
        SOCK_STREAM,        // "stream socket"
        IPPROTO_TCP         // TCP transport protocol
    );
    if (clientSock < 0)
        fgThrow("nix socket() error",strerror(errno));
    ScopeGuard              closeSocket(std::bind(close,clientSock));
    struct hostent *        remoteHost = gethostbyname(hostname.c_str());
    if (remoteHost == NULL) {
        //fgout << fgnl << "Unable to resolve " << hostname;
        return false;
    }
    sockaddr_in  		    client;
    std::memset(&client,0,sizeof(client));
    client.sin_family = AF_INET;
    memmove(&client.sin_addr,remoteHost->h_addr_list[0],remoteHost->h_length);
    client.sin_port = htons(port);  // "host to network short" converts byte order
    int                     status = connect(clientSock,(struct sockaddr*)&client,sizeof(client));
    if (status != 0) {
        //fgout << fgnl << "Unable to connect to " << hostname;
        return false;
    }
    //fgout << fgnl << "Sending data ... " << std::flush;
    // write() is same as send() with flag=0:
    int                     nBytes =
        write(clientSock,reinterpret_cast<std::byte const *>(data.data()),int(data.size()));
    if (nBytes < int(data.size()))
        FGASSERT_FALSE1(toStr(nBytes));
    //fgout << "done" << std::flush;
    // close socket for sending to cause server's recv/read to return a zero
    // size data packet if server is waiting for more (ie to flush the stream).
    // An alternative design, if multiple back-and-forth is ever required, is to
    // send the amount of data before sending the data so the receiver knows when
    // to stop calling recv():
    shutdown(clientSock,1);
    if (responsePtr != nullptr) {
        responsePtr->clear();
        char                    buff[1024];
        do {
            // read() same as recv() with flag=0:
            nBytes = read(clientSock,buff,sizeof(buff));
            FGASSERT(nBytes >= 0);
            if (nBytes > 0)
                append_(*responsePtr,reinterpret_cast<std::byte const *>(buff),nBytes);
        }
        while (nBytes > 0);
        FGASSERT(nBytes == 0);
    }
    return true;
}

void                sigchld_handler(int)
{
    while(waitpid(-1, NULL, WNOHANG) > 0)
    {}      // Avoid warning vs. just a semicolon
}

// get sockaddr, IPv4 or IPv6:
void                *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void                runTcpServer(uint16 port,bool respond,TcpHandlerFunc handler,size_t maxRecvBytes)
{
    struct sockaddr_storage clientAddress;
    int                     listenSockFd = -1;  // Avoid uninitialized warning
    struct addrinfo         hints,
                            *servinfo,
                            *p;
    struct sigaction        sa;
    int                     yes=1;
    std::memset(&hints,0,sizeof hints);
    // On most unix systems, AF_UNSPEC choice will listen for either IPv4 or IPv6 incoming connections:
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int                     rv = getaddrinfo(NULL,toStr(port).c_str(),&hints,&servinfo);
    if (rv != 0)
        fgThrow("nix getaddrinfo() error",gai_strerror(rv));
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((listenSockFd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1) {
            fgout << fgnl << "server_failed_socket_call " << std::flush;
            continue;
        }
        if (setsockopt(listenSockFd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) == -1)
            fgThrow("nix setsockopt(SO_REUSEADDR) error",strerror(errno));
        if (bind(listenSockFd,p->ai_addr,p->ai_addrlen) == -1) {
            close(listenSockFd);
            fgout << fgnl << "server_failed_bind_call " << std::flush;
            continue;
        }
        break;
    }
    FGASSERT(p != NULL);
    freeaddrinfo(servinfo);
    // Set the socket to listen and queue up to 10 incoming connections:
    if (listen(listenSockFd,10) == -1)
        fgThrow("nix listen() error",strerror(errno));
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        fgThrow("nix sigaction error",strerror(errno));
    // Listen for client:
    bool                        handlerRetval;
    do {
        fgout << fgnl << "TCP server waiting. ";
        socklen_t               sz = sizeof(clientAddress);
        // Get incoming message socketFd. Will block until a message arrives since the
        // listen socket does not have the O_NONBLOCK option set.
        // The data socket is unique to the client IP:PORT, so multiple TCP connections can
        // take place simultaneously (not made use of here):
        int                     dataSockFd = accept(listenSockFd,(struct sockaddr *)&clientAddress,&sz);
        if (dataSockFd < 0)
            fgThrow("nix accept() error",strerror(errno));
        fgout << " receiving. ";
        // Set the data receive timeout. This is only for receiving a data pack, not for the connection itself,
        // so not be necessary if the a timeout on the entire connection can be set:
        timeval                 timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        if (setsockopt(dataSockFd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) == -1)
            fgout << "nix setsockopt(SO_RCVTIMEO) failed: " << strerror(errno) << " ";
        // Convert IP address to string:
        char                    sbuf[INET6_ADDRSTRLEN];
        inet_ntop(
            clientAddress.ss_family,
            get_in_addr((struct sockaddr *)&clientAddress),
            sbuf,
            sizeof sbuf);
        String                  ipAddr (sbuf);
        // Read incoming message:
        Bytes                   dataBuff;
        int         	        bytesRecvd;
        do {
            char                    buffer[1024];
            // read() will return when either it has filled the buffer, copied over everything
            // from the socket input buffer (only if non-empty), or when the the connection
            // is closed by the client. Otherwise it will block (ie if input buffer empty).
            bytesRecvd = read(dataSockFd,buffer,sizeof(buffer));
            fgout << ".";
            if (bytesRecvd > 0)
                append_(dataBuff,reinterpret_cast<std::byte const *>(buffer),bytesRecvd);
        }
        while ((bytesRecvd > 0) && (dataBuff.size() <= maxRecvBytes));
        fgout << " " << dataBuff.size() << "B ";
        if (bytesRecvd != 0) {
            fgout << "RECEIVE ERROR: " << bytesRecvd;
            close(dataSockFd);
            if (bytesRecvd > 0)
                fgout << "OVERSIZE MESSAGE IGNORED ";
            continue;
        }
        if (!respond)   // Handler can take arbitrarily long in this case so must close immediately:
            close(dataSockFd);
        String8                 currDir = getCurrentDir();
        Bytes                   response;
        fgout << "Executing. ";
        try {
            handlerRetval = handler(ipAddr,dataBuff,response);
        }
        catch(FgException const & e) {
            fgout << fgnl << "Handler exception (FG exception): " << e.englishMessage();
        }
        catch(std::exception const & e) {
            fgout << fgnl << "Handler exception (std::exception): " << e.what();
        }
        catch(...) {
            fgout << fgnl << "Handler exception (unknown type)";
        }
        if (respond) {
            if (!response.empty()) {
                fgout << fgnl << "Responding. ";
                int                 bytesSent =
                    write(dataSockFd,reinterpret_cast<std::byte const *>(response.data()),response.size());
                if (bytesSent != int(response.size()))
                    fgout << "SEND ERROR: " << bytesSent << " (of " << response.size() << "). ";
            }
            close(dataSockFd);
        }
        fgout << std::flush;                // must flush here since it may be a long time until next packet arrives
        setCurrentDir(currDir);             // In case handler changed it
    } while (handlerRetval == true);
    if (listenSockFd >= 0)
        close(listenSockFd);
}

}

// */
