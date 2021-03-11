//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 1, 2011
//
// Using Posix sockets, which are essentially Berkeley sockets.

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
#include "FgException.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdString.hpp"
#include "FgScopeGuard.hpp"
#include "FgOut.hpp"

// Do NOT use std namespace to avoid collision with posix 'bind'

namespace Fg {

bool
fgTcpClient(
    const std::string & hostname,
    uint16              port,
    const std::string & data,
    bool                getResponse,
    std::string &       response)
{
    int     clientSock = socket(
                AF_INET,            // IPv4 protocol family
                SOCK_STREAM,        // "stream socket"
                IPPROTO_TCP);       // TCP transport protocol
    FGASSERT(clientSock >= 0);
    ScopeGuard        closeSocket(std::bind(close,clientSock));
    // Set the timeout so the user doesn't have to wait forever if the connection fails:
    timeval         timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) == -1)
        FGASSERT_FALSE;
    struct hostent *    remoteHost = gethostbyname(hostname.c_str());
    if (remoteHost == NULL) {
        //fgout << fgnl << "Unable to resolve " << hostname;
        return false;
    }
    sockaddr_in  		client;
    std::memset(&client,0,sizeof(client));
    client.sin_family = AF_INET;
    memmove(&client.sin_addr,remoteHost->h_addr_list[0],remoteHost->h_length);
    client.sin_port = htons(port);  // "host to network short" converts byte order
    int                 status = connect(clientSock,(struct sockaddr*)&client,sizeof(client));
    if (status != 0) {
        //fgout << fgnl << "Unable to connect to " << hostname;
        return false;
    }
    //fgout << fgnl << "Sending data ... " << std::flush;
    // write() is same as send() with flag=0:
    int nBytes = write(clientSock,data.data(),int(data.size()));
    if (nBytes < int(data.size()))
        FGASSERT_FALSE1(toStr(nBytes));
    //fgout << "done" << std::flush;
    // close socket for sending to cause server's recv/read to return a zero
    // size data packet if server is waiting for more (ie to flush the stream).
    // An alternative design, if multiple back-and-forth is ever required, is to
    // send the amount of data before sending the data so the receiver knows when
    // to stop calling recv():
    shutdown(clientSock,1);
    if (getResponse) {
        response.clear();
        char    buff[1024];
        do {
            // read() same as recv() with flag=0:
            nBytes = read(clientSock,buff,sizeof(buff));
            FGASSERT(nBytes >= 0);
            if (nBytes > 0)
                response += std::string(buff,nBytes);
        }
        while (nBytes > 0);
        FGASSERT(nBytes == 0);
    }
    return true;
}

void sigchld_handler(int)
{
    while(waitpid(-1, NULL, WNOHANG) > 0)
    {}      // Avoid warning vs. just a semicolon
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void
runTcpServer(
    uint16              port,
    bool                respond,
    TcpHandlerFunc    handler,
    size_t              maxRecvBytes)
{
    struct sockaddr_storage clientAddress;
    int                 listenSockFd = -1;  // Avoid uninitialized warning
    struct addrinfo     hints,
                        *servinfo,
                        *p;
    struct sigaction    sa;
    int                 yes=1;
    std::memset(&hints, 0, sizeof hints);
    // On most unix systems, AF_UNSPEC choice will listen for either IPv4 or IPv6 incoming connections:
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int rv = getaddrinfo(NULL,toStr(port).c_str(),&hints,&servinfo);
    FGASSERT1(rv == 0,std::string(gai_strerror(rv)));

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((listenSockFd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1) {
            fgout << fgnl << "server_failed_socket_call " << std::flush;
            continue;
        }
        if (setsockopt(listenSockFd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) == -1)
            FGASSERT_FALSE;
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
        FGASSERT_FALSE;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        FGASSERT_FALSE;

    // Listen for client:
    int         dataSockFd;
    bool        handlerRetval;
    do {
        socklen_t   sz = sizeof(clientAddress);
        // Get incoming message socketFd. Will block until a message arrives since the
        // listen socket does not have the O_NONBLOCK option set.
        // The data socket is unique to the client IP:PORT, so multiple TCP connections can
        // take place simultaneously (not made use of here):
        dataSockFd = accept(listenSockFd,(struct sockaddr *)&clientAddress,&sz);
        FGASSERT(dataSockFd >= 0);
        // Set the timeout. Very important since the default is to never time out so in some
        // cases a broken connection causes 'recv' below to block forever:
        timeval         timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if (setsockopt(dataSockFd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) == -1)
            FGASSERT_FALSE;
        // Convert IP address to string:
        char                sbuf[INET6_ADDRSTRLEN];
        inet_ntop(
            clientAddress.ss_family,
            get_in_addr((struct sockaddr *)&clientAddress),
            sbuf,
            sizeof sbuf);
        std::string  ipAddr = std::string(sbuf);
        // Read incoming message:
        std::string 	dataBuff;
        int         	bytesRecvd;
        fgout << fgnl << "> " << std::flush;
        do {
            char        buffer[1024];
            // read() will return when either it has filled the buffer, copied over everything
            // from the socket input buffer (only if non-empty), or when the the connection
            // is closed by the client. Otherwise it will block (ie if input buffer empty).
            bytesRecvd = read(dataSockFd,buffer,sizeof(buffer));
            fgout << "." << std::flush;
            if (bytesRecvd > 0)
                dataBuff += std::string(buffer,bytesRecvd);
        }
        while ((bytesRecvd > 0) && (dataBuff.size() <= maxRecvBytes));
        if (bytesRecvd != 0) {
            fgout << "RECEIVE ERROR: ";
            close(dataSockFd);
            if (bytesRecvd > 0)
                fgout << "OVERSIZE MESSAGE IGNORED.";
            if (bytesRecvd < 0)
                fgout << "TCP READ ERROR: " << bytesRecvd;
            fgout << std::flush;
            continue;
        }
        fgout << ": " << std::flush;
        if (!respond)   // Handler can take arbitrarily long in this case so must close immediately:
            close(dataSockFd);
        std::string         response;
        try {
            handlerRetval = handler(ipAddr,dataBuff,response);
        }
        catch(FgException const & e) {
            fgout << "Handler exception (FG exception): " << e.no_tr_message();
        }
        catch(std::exception const & e) {
            fgout << "Handler exception (std::exception): " << e.what();
        }
        catch(...) {
            fgout << "Handler exception (unknown type)";
        }
        if (respond) {
            if (!response.empty()) {
                int     bytesSent = write(dataSockFd,response.data(),response.size());
                if (bytesSent != int(response.size()))
                    fgout << "TCP WRITE ERROR: " << bytesSent << " (of " << response.size() << "). ";
            }
            close(dataSockFd);
        }
        fgout << std::flush;
    } while (handlerRetval == true);
    if (listenSockFd >= 0)
        close(listenSockFd);
}

}

// */
