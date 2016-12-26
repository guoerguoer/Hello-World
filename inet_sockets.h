//-----------------------------------------------------------------------------
//程序：
//      TCP套接字通信函数声明文件
//历史：
//      2014-9-26   李彦儒     1.0
//修改描述：
//
#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H

#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// host参数是一个字符串，它包含一个主机名或一个数值地址（以IPv4的点分十进制表示或IPv6的十六进制字符串表示）。
//  或者也可以将host指定为NULL来表明使用环回地址。
// service参数是一个服务名或者是一个以十进制字符串表示的端口号。
// type 参数是socket的类型，其取值为SOCK_STREAM或SOCK_DGRAM

//inetConnect()函数根据给定的socket type创建一个socket并将其连接到通过host和service指定
//的地址。这个函数可供需将自己的socket连接到一个服务器socket的TCP或UDP客户端使用。
//返回结果：新socket的文件描述符会作为函数结果返回。
int inetConnect(const char *host, const char *service, int type);

//inetListen()函数创建一个监听流（SOCK_STREAM）socket，该socket会被绑定到由service
// 指定的TCP端口的通配IP地址上。这个函数被设计供TCP服务器使用。
//返回结果：新socket的文件描述符会作为函数结果返回。
// backlog参数指定了允许积压的未决连接数量（与listen()一样)
int inetListen(const char *service, int backlog, socklen_t *addrlen);

// inetBind()函数根据给定的type创建一个socket并将其绑定到由service和type指定的端口的通配IP地址上。
//（socket type指定了该socket是一个TCP服务还是一个UDP服务器。）
// 这个函数被设计（主要）供UDP服务器和创建socket并将其绑定到某个具体地址上的客户端使用。
// 返回结果：新socket的文件描述符会作为函数结果返回
int inetBind(const char *service, int type, socklen_t *addrlen);

// 在addr中给定了socket地址结构，其长度在addrlen中指定，
// 返回值：返回一个以null结尾的字符串，该字符串包含了对应的主机名和端口号，其形式如下：
// (hostname, port-number) 返回的字符串是存放在addrStr指向的缓冲器中的。
// addrlen指定了缓冲器的大小。如果返回的字符串超过了（addrStrLen - 1）字节，
// 那么它会被截断
// addrStrLen建议值：IS_ADDR_STR_LEN
char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
						char *addrStr, int addrStrLen);

#define IS_ADDR_STR_LEN 4096
								// Suggested length for string buffer that caller
								// should pass to inetAddressStr(). Must be greater
								// than (NI_MAXHOST + NI_MAXSERV + 4
#define TRUE 1
#define FALSE 0
#endif
