// UDP client that uses blocking sockets

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <tuple>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "0:0:0:0:0:0:0:1"	// IPv6 address of server in localhost
#define SERVER_PORT 27015					// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512						// Size of buffer that will be used for sending and receiving messages to client


int main()
{
	FILE *file;
	int offset_bytes, send_length, send_times, i = 0; //variables for choosing file segment to send
    // Server address structure
    sockaddr_in6 serverAddress;

    // Size of server address structure
	int sockAddrLen = sizeof(serverAddress);

	// Buffer that will be used for sending and receiving messages to client
    char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is used to receive details of the Windows Sockets implementation
    WSADATA wsaData;
    
	// Initialize windows sockets for this process
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    
	// Check if library is succesfully initialized
	if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

   // Initialize memory for address structure
    memset((char*)&serverAddress, 0, sizeof(serverAddress));		
    
	
	 // Initialize address structure of server
	serverAddress.sin6_family = AF_INET6;								// IPv6 address famly
    inet_pton(AF_INET6, SERVER_IP_ADDRESS, &serverAddress.sin6_addr);	// Set server IP address using string
    serverAddress.sin6_port = htons(SERVER_PORT);						// Set server port
	serverAddress.sin6_flowinfo = 0;									// flow info
	 

	// Create a socket
    SOCKET clientSocket = socket(AF_INET6,      // IPv6 address famly
								 SOCK_STREAM, 
								 IPPROTO_TCP);

	// Check if socket creation succeeded
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

	iResult = connect (clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server\n");
		WSACleanup();
		return 1;
	}

	if ( (file = fopen("LoremIpsum.txt", "r")) == NULL)
	{
		printf("Error opening file\n");
		return 1;
	}
	printf("Input offset in bytes:\t");
	scanf("%d", &offset_bytes);
	printf("Input sending duration(length):\t");
	scanf("%d", &send_length);
	send_times = send_length/BUFFER_SIZE + (((send_length % BUFFER_SIZE) != 0) ? 1 : 0); //number of times client sends message
	fseek(file, offset_bytes, SEEK_SET);
	while (i < send_times)
	{
		if (fgets(dataBuffer, BUFFER_SIZE, file) == NULL)
			break;
		// Send message to server
		printf("%s\n", dataBuffer);
		iResult = send(clientSocket, dataBuffer, BUFFER_SIZE, 0);

		// Check if message is succesfully sent. If not, close client application
		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		i++;
	}

	fclose(file);
	// Close client application
    iResult = closesocket(clientSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
        return 1;
    }

	// Close Winsock library
    WSACleanup();

	// Client has succesfully sent a message
    return 0;
}
