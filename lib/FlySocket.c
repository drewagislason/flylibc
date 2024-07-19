/**************************************************************************************************
  FlySocket.c - Easy IPV4/IPv6 TCP and UTP sockets
  Copyright 2024 Drew Gislason  
  license: <https://mit-license.org>
*///***********************************************************************************************
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "Fly.h"
#include "FlyLog.h"
#include "FlySocket.h"

/*!
  @defgroup FlySocket   A simplified socket interface for any combo of TCP/UDP, IPv4/IPv6, server or client

  See: https://beej.us for a primer on C sockets

  Features:

  * Simplifies sockets for C applications, simply include "FLySocket.h"
  * Supports any combo of TCP/UDP, IPv4/IPv6, server or client
  * Supports multi-tasking or event loop, or simple blocking
  * See example: flychat.c and flychatserver.c
*/
#define FLY_SOCK_SANCHK         45454
#define FLY_SOCK_ADDR_SANCHK    45455

typedef struct
{
  unsigned                  sanchk;
  int                       sockFd;
  socklen_t                 addrLen;
  struct sockaddr_storage   sAddr;
} sFlySockAddr_t;           // see hFlySockAddr_t;

typedef struct
{
  unsigned                  sanchk;       // sanity check
  bool_t                    fServer;      // TRUE if server, FALSE if client
  bool_t                    fIpv6;        // TRUE if IPv6 (otherwise IPv4)
  bool_t                    fTcp;         // TRUE if TCP (otherwise UDP)
  bool_t                    fNonBlock;    // TRUE if non-blocking for send/receive/accept
  int                       errNum;       // last errno to a socket function
  sFlySockAddr_t            sAddr;
} sFlySock_t;               // see hFlySock_t

/*------------------------------------------------------------------------------------------------
  Get an addrinfo structure from the parameters. Returns the head of a linked list

  @param    szHost    Host, e.g. "localhost", "10.1.1.20", "::1" or "www.google.com"
  @param    szPort    e.g. "443", or "5000"
  @param    fIpv6     TRUE if IPv6, FALSE if IPv4
  @param    fTcp      TRUE if TCP, FALSE if UDP
  @param    fServer   TRUE if this will be a server
  @return   ptr to addrinfo head. Must free with freeaddrinfo()
-------------------------------------------------------------------------------------------------*/
static struct addrinfo * SockAddrInfoGet(const char *szHost, const char *szPort, bool_t fIpv6, bool_t fTcp, bool_t fServer)
{
  struct addrinfo   hints;
  struct addrinfo  *pAddrInfo = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = fIpv6 ? AF_INET6 : AF_INET;
  hints.ai_socktype = fTcp ? SOCK_STREAM : SOCK_DGRAM;
  if(fServer)
    hints.ai_flags = AI_PASSIVE;

  if(getaddrinfo(szHost, szPort, &hints, &pAddrInfo) != 0)
    pAddrInfo = NULL;

  return pAddrInfo;
}

/*!-----------------------------------------------------------------------------------------------
  Create a new socket. If server, this binds it to a port and starts listening.

  @param    szHost    host to connect to
  @param    szPort    port to connect to, can be service like 
  @param    type      e.g. FFLY_SOCK_IPV4_TCP
  @param    fServer   TRUE if this is binding to a port (a server)
  @return   handle to socket, or NULL if failed
*///-----------------------------------------------------------------------------------------------
hFlySock_t FlySockNew(const char *szHost, const char *szPort, flySockType_t type, bool_t fServer)
{
  sFlySock_t       *pSock;
  struct addrinfo  *pAddrInfo;
  int               sockFd      = -1;

  pSock = FlySockMemAlloc(sizeof(*pSock));
  if(pSock)
  {
    memset(pSock, 0, sizeof(*pSock));
    pSock->sanchk       = FLY_SOCK_SANCHK;
    pSock->fServer      = fServer;
    pSock->fIpv6        = FlySockIsIpv6(type);
    pSock->fTcp         = FlySockIsTcp(type);
    pSock->sAddr.sockFd = -1;
    pSock->sAddr.sanchk = FLY_SOCK_ADDR_SANCHK;
  }

  // open the socket
  if(pSock)
  {
    pAddrInfo = SockAddrInfoGet(szHost, szPort, pSock->fIpv6, pSock->fTcp, fServer);
    // if(pAddrInfo)
    //   printf("Got AddrInfo\n");
    // else
    //   printf("AddrInfo failed on host %s, port %s\n", szHost, szPort);

    if(pAddrInfo)
      sockFd = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);

    if(sockFd < 0)
    {
      printf("Socket Failed\n");
      pSock = FlySockFree(pSock);
    }
    else
    {
      // char      szHost[FLY_SOCK_ADDRSTRLEN];
      // unsigned  port;

      // printf("Got Socket %d\n", sockFd);

      // store the addr and len and sockFd for use in later functions
      // FlyAssert(pAddrInfo->ai_addrlen <= INET6_ADDRSTRLEN);
      memcpy(&pSock->sAddr.sAddr, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen);
      pSock->sAddr.addrLen = pAddrInfo->ai_addrlen;
      pSock->sAddr.sockFd = sockFd;

      // if(FlySockAddrHostGet(&pSock->sAddr, szHost, &port))
      //   printf("Host %s, port %u\n", szHost, port);
      // else
      //   printf("FlySockAddrHostGet() failed!\n");
    }
    // if(pAddrInfo)
    //   freeaddrinfo(pAddrInfo);
  }

  // intialize server
  if(pSock && pSock->fServer && sockFd > 0)
  {
    int yes = 1;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind socket to the port
    if(bind(sockFd, (struct sockaddr *)(&pSock->sAddr.sAddr), pSock->sAddr.addrLen) == -1)
    {
      // printf("Bind Failed\n");
      pSock = FlySockFree(pSock);
    }
    else
      // printf("Bind OK\n");

    // TCP server needs to listen
    if(pSock && pSock->fTcp)
    {
      if(listen(sockFd, FLY_SOCK_BACKLOG) == -1)
      {
        // printf("listen failed\n");
        pSock = FlySockFree(pSock);
      }
      // printf("TCP Server listening\n");
    }
  }

  // client initialization
  else if(pSock && pSock->fTcp)
  {
    if(connect(sockFd, (struct sockaddr *)&pSock->sAddr.sAddr, pSock->sAddr.addrLen) == -1)
    {
      printf("connect failed\n");
      FlySockFree(pSock);
      pSock = NULL;
    }
    // else
    //   printf("%s TCP client connected\n", pSock->fTcp ? "TCP" : "UDP");
  }

  if(!pSock)
    printf("No Socket\n");
  // else
  //   printf("pSock %p, fd %d\n", pSock, pSock->sAddr.sockFd);

  return  (hFlySock_t)pSock;
}

/*!-----------------------------------------------------------------------------------------------
  Is this type tcp?

  @param    type   see flySockType_t, e.g. FLY_SOCK_TYPE_IPV4_TCP
  @return   TRUE if TCP, FALSE if UDP
*///-----------------------------------------------------------------------------------------------
bool_t FlySockIsTcp(flySockType_t type)
{
  return ((type == FLY_SOCK_TYPE_IPV4_TCP) || (type == FLY_SOCK_TYPE_IPV6_TCP)) ? TRUE : FALSE;
}

/*!-----------------------------------------------------------------------------------------------
  Is this type IPv6?

  @param    type   see flySockType_t, e.g. FLY_SOCK_TYPE_IPV4_TCP
  @return   TRUE if IPv6, FALSE if IPv4
*///-----------------------------------------------------------------------------------------------
bool_t FlySockIsIpv6(flySockType_t type)
{
  return ((type == FLY_SOCK_TYPE_IPV6_UDP) || (type == FLY_SOCK_TYPE_IPV6_TCP)) ? TRUE : FALSE;
}

/*!-----------------------------------------------------------------------------------------------
  Is this a valid socket handle?

  @param    hSock   A socket created with FlySockNew()
  @return   TRUE if valid
*///-----------------------------------------------------------------------------------------------
bool_t FlySockIsSock(hFlySock_t hSock)
{
  sFlySock_t    *pSock  = hSock;
  return (pSock && pSock->sanchk == FLY_SOCK_SANCHK) ? TRUE : FALSE;
}

/*!-----------------------------------------------------------------------------------------------
  Is this socket created with FlySockNew() a server?

  @param    hSock   A socket created with FlySockNew()
  @return   TRUE if a server
*///-----------------------------------------------------------------------------------------------
bool_t FlySockIsServer(hFlySock_t hSock)
{
  sFlySock_t    *pSock  = hSock;
  return (FlySockIsSock(hSock) && pSock->fServer) ? TRUE : FALSE;
}

/*!-----------------------------------------------------------------------------------------------
  Returns socket handle, or -1 if bad handle. Useful to set flags on the file handle that this
  framework doesn't support. Otherwise, don't use

  @param    hSock   A socket created with FlySockNew()
  @return   socket handle (int), or -1 if bad handle or socket
*///-----------------------------------------------------------------------------------------------
int FlySockFd(hFlySock_t hSock)
{
  sFlySock_t    *pSock  = hSock;
  int           sockFd  = -1;

  if(FlySockIsSock(hSock))
    sockFd = pSock->sAddr.sockFd;

  return sockFd;
}

/*!-----------------------------------------------------------------------------------------------
  Set to blocking or non-block. Call AFTER FlySockNew() and BEFORE FlySockAccept(). Any already
  accepted sockets won't be changed by this call.

  @param    hSock   A socket created with FlySockNew()
  @return   Returns the previous state of non-block
*///-----------------------------------------------------------------------------------------------
bool_t FlySockSetNonBlock(hFlySock_t hSock, bool_t fNonBlock)
{
  sFlySock_t    *pSock        = hSock;
  bool_t         fOldNonBlock = FALSE;

  if(FlySockIsSock(hSock))
  {
    fOldNonBlock = pSock->fNonBlock;
    pSock->fNonBlock = fNonBlock;

    // set the actual socket for this 
    if(pSock->sAddr.sockFd >= 0)
      fcntl(pSock->sAddr.sockFd, F_SETFL, fNonBlock ? O_NONBLOCK : 0);
  }

  return fOldNonBlock;
}

/*!-----------------------------------------------------------------------------------------------
  Get the host for this socket. On a server, the means the localhost/port. On a client, that
  means the destination host/port.

  @param    hSock   A socket created with FlySockNew()
  @return   Returns TRUE if worked, FALSE if bad parameter
*///-----------------------------------------------------------------------------------------------
bool_t FlySockHostGet(hFlySock_t hSock, char *pszHost, unsigned *pPort)
{
  sFlySock_t    *pSock   = hSock;
  bool_t         fWorked = FALSE;

  if(FlySockIsSock(hSock) && pszHost && pPort)
    fWorked = FlySockAddrHostGet(&pSock->sAddr, pszHost, pPort);

  return fWorked;
}

/*!-----------------------------------------------------------------------------------------------
  Only to be used on server sockets that were opened with FlySockNew().

  Returns a handle to a structure which contains address, port and socket file descriptor to be
  used with FlySockSend() or FlySockReceive().

  @param    hSock         A socket created with FlySockNew()
  @param    fNonBlock     Return value if TRUE means failure was due to nonblock behavior
  @return   handle to Addr or NULL if failed
*///-----------------------------------------------------------------------------------------------
hFlySockAddr_t FlySockAccept(hFlySock_t hSock, bool_t *pfNonBlock)
{
  sFlySock_t               *pSock  = hSock;
  sFlySockAddr_t           *pAddr  = NULL;
  struct sockaddr_storage   sAddr;
  socklen_t                 addrLen = sizeof(sAddr);
  int                       sockFd;

  if(pfNonBlock)
    *pfNonBlock = FALSE;

  if(FlySockIsServer(hSock))
  {
    // need the addr whether tcp or udp (UDP needs it for sendto() / recvfrom()
    if(pSock->fTcp)
    {
      // if we can't open the socket, may just be due to non-blocking
      memset(&sAddr, 0, sizeof(sAddr));
      sockFd = accept(pSock->sAddr.sockFd, (struct sockaddr *)(&sAddr), &addrLen);
      if(sockFd < 0)
      {
        pSock->errNum = errno;
        if(pfNonBlock && ((pSock->errNum == EAGAIN) || (pSock->errNum == EWOULDBLOCK)))
          *pfNonBlock = TRUE;
      }

      // worked
      else
      {
        // printf("TCP Accept fd %d, addrlen %u, family %u\n", sockFd, addrLen, sAddr.ss_family);
        pAddr = FlySockAddrNew(hSock);
        if(pAddr)
        {
          memcpy(&pAddr->sAddr, &sAddr, addrLen);
          pAddr->addrLen = addrLen;
          pAddr->sockFd = sockFd;

          if(pSock->fNonBlock)
            fcntl(sockFd, F_SETFL, O_NONBLOCK);
        }
      }
    }

    // UDP
    else
    {
      pAddr = FlySockAddrNew(hSock);
      if(pSock->fNonBlock)
        fcntl(pAddr->sockFd, F_SETFL, O_NONBLOCK);
    }
  }

  return pAddr;
}

/*!-----------------------------------------------------------------------------------------------
  Free a socket allocated by FlySockNew(). Returns NULL as a convenience for the caller:

      pSock = FlySockFree(pSock);

  @param    hSock     A socket created with FlySockNew()
  @returns  NULL
*///-----------------------------------------------------------------------------------------------
void * FlySockFree(hFlySock_t hSock)
{
  sFlySock_t    *pSock        = hSock;

  if(FlySockIsSock(hSock))
  {
    if(pSock->sAddr.sockFd > 0)
      close(pSock->sAddr.sockFd);

    memset(pSock, 0, sizeof(*pSock));
    FlySockMemFree(pSock);
  }

  return NULL;
}

/*!-----------------------------------------------------------------------------------------------
  Receive data on a socket. Assumes socket was opened as part of FlySockNew(), and the hAddr was
  returned from FlySockAccept()

  @param    hSock       The hSock returned from FlySockNew()
  @param    hAddr       The hAddr returned from FlySockAccept() or FlySockAddrNew()
  @param    pBuf        Buffer to hold received data
  @param    bufLen      size of buffer (1-n)
  @return   number of bytes received, or -1 on error, 0 if other closed
*///-----------------------------------------------------------------------------------------------
int FlySockReceive(hFlySock_t hSock, hFlySockAddr_t hAddr, uint8_t *pBuf, int bufLen)
{
  sFlySock_t       *pSock     = hSock;
  sFlySockAddr_t   *pAddr     = hAddr;
  int               sockFd    = -1;
  int               len       = -1;

  if(FlySockIsSock(hSock) && FlySockAddrIsAddr(hAddr))
  {
    // TCP: receiving from accepted client, FlySockAccept()
    if(pSock->fTcp)
      sockFd = pAddr->sockFd;
    // UDP: receiving from socket created during FlySockNew()
    else
      sockFd = pSock->sAddr.sockFd;

    if(sockFd >= 0 && pBuf && bufLen > 0)
    {
      if(pSock->fTcp)
        len = (int)recv(sockFd, pBuf, bufLen, 0);
      else
        len = (int)recvfrom(sockFd, pBuf, bufLen, 0, (struct sockaddr *)(&pAddr->sAddr), &pAddr->addrLen);
      if(len < 0)
        pSock->errNum = errno;
    }
  }

  return len;
}

#if 0
void PrintSin(void *p, sockLen_t addrLen)
{
  struct sockaddr_in  *p4 = p;
  struct sockaddr_in6 *p6 = p;
  unsigned port;

  if(p4 && p4->sin_family == AF_INET)
  {
    printf("addrLen %u, sizeof(sockaddr_in) %u\n", addrLen, sizeof(struct sockaddr_in));
      inet_ntop(pSockAddr->sa_family, pTheirAddr, pszHost, pAddr->addrLen);
    *pPort = ntohs(p4->sin_port);
  }
  else if(p6 && p6->sin5_family == AF_INET6)
  {
    printf("addrLen %u, sizeof(sockaddr_in6) %u\n", addrLen, sizeof(struct sockaddr_in6));
      inet_ntop(pSockAddr->sa_family, pTheirAddr, pszHost, pAddr->addrLen);
    *pPort = ntohs(p6->sin6_port);

  }
  else
  {
    printf("unknown family p %p\n", p);
  }
}
#endif

/*!-----------------------------------------------------------------------------------------------
  Send data on a socket. Assumes socket was opened as part of FlySockNew(), and the hAddr was
  returned from FlySockAccept()

  @param    hSock       The hSock returned from FlySockNew()
  @param    hAddr       The hAddr returned from FlySockAccept() or FlytSockAddrNew()
  @param    pBuf        Buffer to send
  @param    bufLen      Length of buffer (1-n)
  @return   number of bytes sent or -1 if error, 0 if other side closed
*///-----------------------------------------------------------------------------------------------
int FlySockSend(hFlySock_t hSock, hFlySockAddr_t hAddr, const uint8_t *pBuf, int bufLen)
{
  sFlySock_t       *pSock     = hSock;
  sFlySockAddr_t   *pAddr     = hAddr;
  struct sockaddr  *p         = NULL;
  socklen_t         addrLen   = 0;
  int               sockFd    = -1;
  int               len       = -1;

  if(FlySockIsSock(hSock) && FlySockAddrIsAddr(hAddr))
  {
    // printf("server %u, tcp %u, ipv6 %u, pSock %p, pAddr %p\n", pSock->fServer, pSock->fTcp,
    //   pSock->fIpv6, pSock, pAddr);

    // server
    if(pSock->fServer)
    {
      // TCP: sending to accepted client
      if(pSock->fTcp)
        sockFd = pAddr->sockFd;

      // UDP: sending to passed-in client, presumably what we just received from
      else
      {
        sockFd = pAddr->sockFd;
        p = (struct sockaddr *)(&pAddr->sAddr);
        addrLen = pAddr->addrLen;
        // printf("UDP server: addrlen %u, family %u, pBuf %p, bufLen %d, sockFd %d\n", addrLen,
        //   pAddr->sAddr.ss_family, pBuf, bufLen, sockFd);
      }
    }

    // client
    else
    {
      if(pSock->fTcp)
        sockFd = pSock->sAddr.sockFd;

      else
      {
        sockFd = pSock->sAddr.sockFd;
        p = (struct sockaddr *)(&pSock->sAddr.sAddr);
        addrLen = pSock->sAddr.addrLen;
        // printf("UDP client: addrlen %u, family %u, pBuf %p, bufLen %d, sockFd %d\n", addrLen,
        //   pSock->sAddr.sAddr.ss_family, pBuf, bufLen, sockFd);
      }
    }

    if(sockFd >= 0 && pBuf && bufLen > 0)
    {
      if(pSock->fTcp)
        len = (int)send(sockFd, pBuf, bufLen, 0);
      else
      {
        // PrintSin(p, addrLen);
        len = (int)sendto(sockFd, pBuf, bufLen, 0, p, addrLen);
      }
      if(len < 0)
      {
        pSock->errNum = errno;
        // printf("errNum %d %s\n", pSock->errNum, strerror(pSock->errNum));
      }
    }
  }

  return len;
}

/*!-----------------------------------------------------------------------------------------------
  Return last errno. Note a successful call will set this to 0.

  @param    hSock       The hSock returned from FlySockNew()
  @return   last errno
*///-----------------------------------------------------------------------------------------------
int FLySockErrno(hFlySock_t hSock)
{
  sFlySock_t       *pSock   = hSock;
  int               errNum  = EINVAL;

  if(FlySockIsSock(hSock))
    errNum = pSock->errNum;

  return errNum;
}


/*!-----------------------------------------------------------------------------------------------
  Create a new socket address which contains storage for both address and file handle. Copies the
  address already stored in the socket.

  @return   handle to new socket address
*///-----------------------------------------------------------------------------------------------
hFlySockAddr_t FlySockAddrNew(hFlySock_t hSock)
{
  sFlySock_t       *pSock = hSock;
  sFlySockAddr_t   *pAddr = NULL;

  if(FlySockIsSock(hSock))
  {
    pAddr = FlySockMemAlloc(sizeof(*pAddr));
    if(pAddr)
      memcpy(pAddr, &pSock->sAddr, sizeof(*pAddr));
  }

  return pAddr;
}

/*!-----------------------------------------------------------------------------------------------
  Create a new socket address which ontains storage for both address and file handle

  @param    handle created by FlySockAddrNew()
  @return   TRUE if it's a valid handle, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlySockAddrIsAddr(hFlySockAddr_t hAddr)
{
  sFlySockAddr_t  *pAddr = hAddr;

  return (pAddr && (pAddr->sanchk == FLY_SOCK_ADDR_SANCHK)) ? TRUE : FALSE;
}

/*!-----------------------------------------------------------------------------------------------
  Free the socket address structure

  @param    handle created by FlySockAddrNew()
  @return   TRUE if it's a valid handle, FALSE if not
*///-----------------------------------------------------------------------------------------------
void * FlySockAddrFree(hFlySockAddr_t hAddr)
{
  sFlySockAddr_t  *pAddr = hAddr;

  if(FlySockAddrIsAddr(hAddr))
  {
    if(pAddr->sockFd > 0)
      close(pAddr->sockFd);
 
    memset(pAddr, 0, sizeof(*pAddr));
    FlySockMemFree(pAddr);
  }

  return NULL;
}

/*!-----------------------------------------------------------------------------------------------
  Free the socket address structure

  @param    handle created by FlySockAddrNew()
  @return   TRUE if it's a valid handle, FALSE if not
*///-----------------------------------------------------------------------------------------------
bool_t FlySockAddrHostGet(hFlySockAddr_t hAddr, char *pszHost, unsigned *pPort)
{
  bool_t                fWorked = FALSE;
  struct sockaddr      *pSockAddr;
  struct sockaddr_in   *pSockAddrIpv4;
  struct sockaddr_in6  *pSockAddrIpv6;
  void                 *pTheirHost;

  sFlySockAddr_t  *pAddr = hAddr;
  if(FlySockAddrIsAddr(pAddr))
  {
    pSockAddr = (struct sockaddr *)&pAddr->sAddr;

    // debugging
    {
      // char  *szFamily;
      // if(pSockAddr->sa_family == AF_INET)
      //   szFamily = "AF_NET";
      // else if(pSockAddr->sa_family == AF_INET6)
      //   szFamily = "AF_NET6";
      // else
      //   szFamily = "AF_??";
      // printf("%s, addrLen %u\n", szFamily, pAddr->addrLen);
    }

    if(pSockAddr->sa_family == AF_INET)
    {
      pSockAddrIpv4 = (struct sockaddr_in *)pSockAddr;
      *pPort = ntohs(pSockAddrIpv4->sin_port);
      pTheirHost = &(pSockAddrIpv4->sin_addr);
      fWorked = TRUE;
    }
    else if(pSockAddr->sa_family == AF_INET6)
    {
      pSockAddrIpv6 = (struct sockaddr_in6 *)pSockAddr;
      *pPort = ntohs(pSockAddrIpv6->sin6_port);
      pTheirHost = &(pSockAddrIpv6->sin6_addr);
      fWorked = TRUE;
    }
    if(fWorked)
    {
      inet_ntop(pSockAddr->sa_family, pTheirHost, pszHost, pAddr->addrLen);

      // debugging
      // printf("szHost %s, port %u\n", pszHost, *pPort);
    }
  }

  return fWorked;  
}

/*------------------------------------------------------------------------------------------------
  Return type based on boolean flags (e.g. FLY_SOCK_TYPE_IPV6_TCP)

  @param  fIpv6       TRUE if IPv6, FALSE if IPv4
  @param  fTcp        TRUE if TCP, FALSE if UDP
  @return type
-------------------------------------------------------------------------------------------------*/
flySockType_t FlySockTypeOf(bool_t fIpv6, bool_t fTcp)
{
  flySockType_t type;

  if(fIpv6)
  {
    if(fTcp)
      type = FLY_SOCK_TYPE_IPV6_TCP;
    else
      type = FLY_SOCK_TYPE_IPV6_UDP;
  }
  else
  {
    if(fTcp)
      type = FLY_SOCK_TYPE_IPV4_TCP;
    else
      type = FLY_SOCK_TYPE_IPV4_UDP;
  }

  return type;
}

/*------------------------------------------------------------------------------------------------
  Usage = [host] [-pPort] [-t] [-6]

  @param  argc        from main()
  @param  argv        from main()
  @param  ppszHost    ptr to string to receive host (e.g. "localhost" or "10.1.1.8" or "::1")
  @param  ppszPort    ptr to string to receive port (e.g. "5000")
  @param  type        e.g. FLY_SOCK_TYPE_IPV4_UDP
  @return TRUE if parsed, FALSE if error or help
-------------------------------------------------------------------------------------------------*/
bool_t FlySockParseCmdline(int argc, const char *argv[], char **ppszHost, char **ppszPort, flySockType_t *pType)
{
  int                 i;
  const char          szHelp[] =
  "%s [host] [-p#] [-6] [-t]\n"
  "\n"
  "host   a website name, ipv6 (e.g. ::1) or ipv4 address (e.g. 127.0.0.1)\n"
  "-p#    e.g. -p5000\n"
  "-6     IPv6 (default is IPv4)\n"
  "-t     TCP (default is UDP)\n"
  "\n";
  bool_t    fParsed = TRUE;
  bool_t    fIpv6   = FALSE;
  bool_t    fTcp    = FALSE;

  // parse cmdline
  for(i = 1; i < argc; ++i)
  {
    if(argv[i][0] == '-')
    {
      if(strcmp(argv[i], "--help") == 0)
      {
        printf(szHelp, argv[0]);
        fParsed = FALSE;
        break;
      }

      else if(strncmp(argv[i], "-p", 2) == 0)
        *ppszPort = (char *)(&argv[i][2]);

      else if(strcmp(argv[i], "-6") == 0)
        fIpv6 = TRUE;

      else if(strcmp(argv[i], "-t") == 0)
        fTcp = TRUE;

      else
      {
        printf("bad parameter '%s'\n", argv[i]);
        fParsed = FALSE;
        break;
      }
    }
    else
      *ppszHost = (char *)(argv[i]);
  }

  if(fParsed)
    *pType = FlySockTypeOf(fIpv6, fTcp);

  return fParsed;
}
