#include "pch.h"
#include <thread>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mstcpip.h"  
#include <fstream>
#include <string>
#include <windows.h>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")
using namespace std;

ofstream log_file;
int total_clients;
int total_spectators;

const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 3;
const int MAX_SPECTATORS = 2;



//Function Prototypes


int main();


int main()
{
	WSADATA wsaData;
	addrinfo hints;
	addrinfo *server = NULL;
	
	Server GameServer(3, 2);
	GameServer.Difficulty = 2;
	GameServer.Game_mode = 4;
	GameServer.Map = 3;


	log_file.open("server_log.txt");

	//Initialize Winsock
	Output("Intializing Winsock...");
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Setup hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//Setup Server
	Output("Setting up server...");
	getaddrinfo(static_cast<PCSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &server);

	//Create a listening socket for connecting to server
	Output("Creating server socket...");
	GameServer.socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Setup socket options
	setsockopt(GameServer.socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(GameServer.socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	//Assign an address to the server socket.
	Output("Binding socket...");
	int tmp = ::bind(GameServer.socket, server->ai_addr, (int)server->ai_addrlen);
	cout << tmp << endl;

	//Listen for incoming connections.
	Output("Listening...");
	listen(GameServer.socket, SOMAXCONN);

	//Start server
	Output("Starting server...");


	GameServer.Start();

	


	//Clean up Winsock
	WSACleanup();
	Output("Program has ended successfully");

	system("pause");
	return 0;
}

int Output(string msg) {
	cout << msg << endl;
	log_file << msg << endl;
	return 1;
}


