// UDP server that use blocking sockets

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <tuple>
#include "conio.h"
//#include "constants.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27015	// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients

// Checks if ip address belongs to IPv4 address family
bool is_ipV4_address(sockaddr_in6 address);

int main()
{
    // Server address 
     sockaddr_in6  serverAddress;
	 sockaddr_in ipv4_server_Address;

	// Buffer we will use to send and receive clients' messages
    char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Initialize serverAddress structure used by bind function
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6; 			// set server address protocol family
    serverAddress.sin6_addr = in6addr_any;			// use all available addresses of server
    serverAddress.sin6_port = htons(SERVER_PORT);	// Set server port
	serverAddress.sin6_flowinfo = 0;				// flow info

    // Create a socket 
    SOCKET ipv6_listen_socket = socket(AF_INET6,      // IPv6 address famly
								 SOCK_STREAM,   // datagram socket
								 IPPROTO_TCP); // TCP umesto UDP


	SOCKET ipv6_client_socket;
	// Check if socket creation succeeded
    if (ipv6_listen_socket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
	
	// Disable receiving only IPv6 packets. We want to receive both IPv4 and IPv6 packets.
	char no[4] = {0};
	int ipv6_Result = setsockopt(ipv6_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)no, sizeof(no));
	int ipv4_Result;
	if (ipv6_Result == SOCKET_ERROR) 
			printf("failed with error: %u\n", WSAGetLastError());


    // Bind server address structure (type, port number and local address) to socket
    ipv6_Result = bind(ipv6_listen_socket,(SOCKADDR *)&serverAddress, sizeof(serverAddress));
	// Check if socket is succesfully binded to server datas

    if (ipv6_Result == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(ipv6_listen_socket);
        WSACleanup();
        return 1;
    }
	
	printf("Simple TCP server waiting for client messages.\n");
	ipv6_Result = listen(ipv6_listen_socket, SOMAXCONN);
	

	if (ipv6_Result == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ipv6_listen_socket);
        WSACleanup();
        return 1;
	}
	sockaddr_in6 clientAddress;
	memset(&clientAddress, 0, sizeof(clientAddress));
	int sockAddrLen = sizeof(clientAddress);
	ipv6_client_socket = accept(ipv6_listen_socket, (struct sockaddr *)&clientAddress, &sockAddrLen);
    if (ipv6_client_socket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ipv6_listen_socket);
        WSACleanup();
        return 1;
    }

	closesocket(ipv6_listen_socket);
    // Main server loop
    do
    {
		// Set whole buffer to zero
        memset(dataBuffer, 0, BUFFER_SIZE);

		// Receive client message
        ipv6_Result = recv(ipv6_client_socket, dataBuffer, BUFFER_SIZE, 0);
		
		// Check if message is succesfully received
		if (ipv6_Result == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			break;
		}
        char ipAddress[INET6_ADDRSTRLEN]; // INET6_ADDRSTRLEN 65 spaces for hexadecimal notation of IPv6
		
		// Copy client ip to local char[]
		inet_ntop(clientAddress.sin6_family, &clientAddress.sin6_addr, ipAddress, sizeof(ipAddress));
        
		// Convert port number from network byte order to host byte order
        unsigned short clientPort = ntohs(clientAddress.sin6_port);

		bool isIPv4 = is_ipV4_address(clientAddress); //true for IPv4 and false for IPv6

		if(isIPv4){
			char ipAddress1[15]; // 15 spaces for decimal notation (for example: "192.168.100.200") + '\0'
			struct in_addr *ipv4 = (struct in_addr*)&((char*)&clientAddress.sin6_addr.u)[12]; 
			
			// Copy client ip to local char[]
			strcpy_s(ipAddress1, sizeof(ipAddress1), inet_ntoa( *ipv4 ));
			printf("IPv4 Client connected from ip: %s, port: %d, sent: %s.\n---------------\n", ipAddress1, clientPort, dataBuffer);
		}else
			printf("IPv6 Client connected from ip: %s, port: %d, sent: %s.\n---------------\n", ipAddress, clientPort, dataBuffer);
		
		// Possible server-shutdown logic could be put here
    }while (ipv6_Result > 0);

    // Close server application
    ipv6_Result = closesocket(ipv6_client_socket);
    if (ipv6_Result == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
        return 1;
    }
	printf("Server successfully shut down.\n");
	
	// Close Winsock library
	WSACleanup();
	return 0;
}

bool is_ipV4_address(sockaddr_in6 address)
{
	char *check = (char*)&address.sin6_addr.u;

	for (int i = 0; i < 10; i++)
		if(check[i] != 0)
			return false;
		
	if(check[10] != -1 || check[11] != -1)
		return false;

	return true;
}
