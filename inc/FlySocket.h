/**************************************************************************************************
  FlySocket.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
*///***********************************************************************************************
#ifndef FLY_SOCKET_H
#define FLY_SOCKET_H

#include  "Fly.h"

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  #define EXTERN_C extern "C" {
#endif

#ifndef FLY_SOCK_BACKLOG
  #define FLY_SOCK_BACKLOG          8
#endif

#ifndef FLY_SOCK_DEF_WAIT
 #define FLY_SOCK_DEF_WAIT
#endif

// for embedded systems without heap
#ifndef FLY_SOCK_CFG_MEM_ALLOC
  #define FlySockMemAlloc(n)     malloc(n)
  #define FlySockMemFree(p)      free(p)
#endif

typedef enum
{
  FLY_SOCK_TYPE_IPV4_UDP = 0,
  FLY_SOCK_TYPE_IPV4_TCP,
  FLY_SOCK_TYPE_IPV6_UDP,
  FLY_SOCK_TYPE_IPV6_TCP
} flySockType_t;

#define FLY_SOCK_ADDRSTRLEN   46  // ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255

#define FLY_SOCK_CLIENT       FALSE
#define FLY_SOCK_SERVER       TRUE

// return codes (1-n is data)
#define FLY_SOCK_DISCONNECT  0
#define FLY_SOCK_ERR        -1
#define FLY_SOCK_NO_DATA    -2
#define FLY_SOCK_BAD_DATA   -3

typedef void   *hFlySock_t;        // handle to a single socket bound/connect to a host/port
typedef void   *hFlySockAddr_t;    // handle to an address, e.g. what's returned from FlySockAccept()

hFlySock_t      FlySockNew          (const char *szHost, const char *szPort, flySockType_t type, bool_t fServer);
bool_t          FlySockIsTcp        (flySockType_t type);
bool_t          FlySockIsIpv6       (flySockType_t type);
bool_t          FlySockIsSock       (hFlySock_t hSock);
bool_t          FlySockIsServer     (hFlySock_t hSock);
int             FlySockFd           (hFlySock_t hSock);
bool_t          FlySockHostGet      (hFlySock_t hAddr, char *pszHost, unsigned *pPort);
bool_t          FlySockSetNonBlock  (hFlySock_t hSock, bool_t fNonBlock);
hFlySockAddr_t  FlySockAccept       (hFlySock_t hSock, bool_t *pfNonBlock);
void           *FlySockFree         (hFlySock_t hSock);
int             FlySockErrno        (hFlySock_t hSock);

int             FlySockSend         (hFlySock_t hSock, hFlySockAddr_t hAddr, const uint8_t *pBuf, int bufLen);
int             FlySockReceive      (hFlySock_t hSock, hFlySockAddr_t hAddr, uint8_t *pBuf, int bufLen);

hFlySockAddr_t  FlySockAddrNew      (hFlySock_t hSock);
bool_t          FlySockAddrIsAddr   (hFlySockAddr_t hAddr);
void           *FlySockAddrFree     (hFlySockAddr_t hAddr);
bool_t          FlySockAddrHostGet  (hFlySockAddr_t hAddr, char *pszHost, unsigned *pPort);

bool_t          FlySockParseCmdline (int argc, const char *argv[], char **ppszHost, char **ppszPort, flySockType_t *pType);
flySockType_t   FlySockTypeOf       (bool_t fIpv6, bool_t fTcp);

#ifdef __cplusplus
  }
#endif

#endif // FLY_SOCKET_H
