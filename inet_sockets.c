//-----------------------------------------------------------------------------
//程序：
//      TCP套接字通信函数声明文件
//历史：
//      2014-9-26   李彦儒     1.0
//修改描述：
//

// To get NI_MAXHOST and NI_MAXSERV definitions from <netdb.h>
#define _BSD_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "inet_sockets.h"

//inetConnect()函数根据给定的socket type创建一个socket并将其连接到通过host和service指定
//的地址。这个函数可供需将自己的socket连接到一个服务器socket的TCP或UDP客户端使用。
//返回结果：新socket的文件描述符会作为函数结果返回。
int inetConnect(const char *host, const char *service, int type)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
    // Allow IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;
	
	s = getaddrinfo(host, service, &hints, &result);
	if(s != 0) {
		errno = ENOSYS;
		return -1;
	}
	
	// Walk through returned list until we find an address structure that
	// can be used to successfully connect a socket
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd == -1)
            // On error, try next address
            continue;
		
		if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		
		// Connect failed: close this socket and try next address
	}
	
	freeaddrinfo(result);
	
	return (rp == NULL) ? -1 : sfd;
}

static int inetPassiveSocket(const char *service, int type, socklen_t *addrlen,
									char doListen, int backlog)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, optval, s;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = type;
    // Allow IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
    // Use wildcard IP address
    hints.ai_flags = AI_PASSIVE;
	
	s = getaddrinfo(NULL, service, &hints, &result);
	if(s != 0)
		return -1;
	
	// Walk through returned list until we find an address structure
	// that can be used to successfully create and bind a socket
	optval = 1;
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd == -1)
            // On error, try next address
            continue;
		
		if(doListen) {
			if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
					sizeof(optval)) == -1) {
				close(sfd);
				freeaddrinfo(result);
				return -1;
			}
		}
		
		if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            // Success
            break;
		
		// bind() failed: close this socket and try next address
		close(sfd);
	}
	
	if(rp != NULL & doListen) {
		if(listen(sfd, backlog) == -1) {
			freeaddrinfo(result);
			return -1;
		}
	}
	
	if(rp != NULL && addrlen != NULL)
		*addrlen = rp->ai_addrlen;        // Return address structure size
	freeaddrinfo(result);
	
	return (rp == NULL) ? -1 : sfd;
}

//inetListen()函数创建一个监听流（SOCK_STREAM）socket，该socket会被绑定到由service
// 指定的TCP端口的通配IP地址上。这个函数被设计供TCP服务器使用。
//返回结果：新socket的文件描述符会作为函数结果返回。
// backlog参数指定了允许积压的未决连接数量（与listen()一样)
int inetListen(const char *service, int backlog, socklen_t *addrlen)
{
	return inetPassiveSocket(service, SOCK_STREAM, addrlen, TRUE, backlog);
}

// inetBind()函数根据给定的type创建一个socket并将其绑定到由service和type指定的端口的通配IP地址上。
//（socket type指定了该socket是一个TCP服务还是一个UDP服务器。）
// 这个函数被设计（主要）供UDP服务器和创建socket并将其绑定到某个具体地址上的客户端使用。
// 返回结果：新socket的文件描述符会作为函数结果返回
int inetBind(const char *service, int type, socklen_t *addrlen)
{
	return inetPassiveSocket(service, type, addrlen, FALSE, 0);
}

char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
							char *addrStr, int addrStrLen)
{
	char host[NI_MAXHOST], service[NI_MAXSERV];
	
	if(getnameinfo(addr, addrlen, host, NI_MAXHOST,
						service, NI_MAXSERV, NI_NUMERICSERV) == 0)
		snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
	else
		snprintf(addrStr, addrStrLen, "?(UNKNOWN?)");
    // Ensure result is null-terminated
    addrStr[addrStrLen - 1] = '\0';
	return addrStr;
}
