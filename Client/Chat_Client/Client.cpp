// Chat_Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define IP_ADDRESS "192.168.0.88"
#define DEFAULT_PORT "3504"

ofstream log_file;

struct client_type
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFLEN];
};

int process_client(client_type &new_client);
int join_server(client_type &client, addrinfo *result);
int Output(string msg);
int main();

int join_server(client_type &client, addrinfo *result) {
	struct addrinfo *ptr = NULL;
	string message;
	string sent_message = "";
	string response = "";
	int iResult = 0;

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		client.socket = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) {
			Output("socket() failed with error: " + WSAGetLastError());
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			Output("socket() failed with error: " + ptr->ai_family);
			closesocket(client.socket);
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (client.socket == INVALID_SOCKET) {
		Output("Unable to connect to server!");
		WSACleanup();
		system("pause");
		return 1;
	}

	Output("Successfully Connected");


	//Obtain id from server for this client;
	recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
	message = client.received_message;

	if (message != "Server is full")
	{
		client.id = atoi(client.received_message);

		thread my_thread(process_client, ref(client));

		while (1)
		{
			getline(cin, sent_message);
			iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (iResult <= 0)
			{
				log_file << "send() failed: " << WSAGetLastError() << endl;
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		}

		//Shutdown the connection since no more data will be sent
		my_thread.detach();
	}
	else {

		cout << "Would you like to become a spectator? [Y/N]" << endl;
		sent_message = "";

		while (1) {

			getline(cin, sent_message);

			if (sent_message == "Y" || sent_message == "y" || sent_message == "N" || sent_message == "n") {
				iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

				if (iResult <= 0)
				{
					log_file << "send() failed: " << WSAGetLastError() << endl;
					cout << "send() failed: " << WSAGetLastError() << endl;
				}
				else if (sent_message == "Y" || sent_message == "y") {
					recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
					client.id = atoi(client.received_message);

					thread my_thread(process_client, ref(client));

					cout << "Type 'quit' to leave or 'join' to join if a slot availible" << endl;
					while (1)
					{
						response = "";
						getline(cin, response);
						cout << response << endl;
						if (response == "QUIT" || response == "quit") {

							cout << "Closing game...." << endl;
							my_thread.detach();
							
							return 1;
						}
					}
					break;
					//Shutdown the connection since no more data will be sent
				}
			}
		}
	}
}

int process_client(client_type &new_client){
	string message;
	string prefix;
	string delimiter = "/";
	while (1)
	{
		memset(new_client.received_message, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR) {
				message = new_client.received_message;
				prefix = message.substr(0, message.find(delimiter));
				message.erase(0, message.find(delimiter) + delimiter.length());

				if (prefix == "text") {
					cout << message << endl;
				}

			}
			else
			{
				Output("Server not responding, closing.....");
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET) {
		log_file << "The server has disconnected" << endl;
		cout << "The server has disconnected" << endl;
	}
	return 0;
}

int main()
{
	WSAData wsa_data;
	struct addrinfo *result = NULL, hints;
	client_type client = { INVALID_SOCKET, -1, "" };
	int iResult = 0;
	log_file.open("client_log.txt");

	log_file << "Starting Client...\n";
	cout << "Starting Client...\n";

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (iResult != 0) {
		log_file << "WSAStartup() failed with error: " << iResult << endl;
		cout << "WSAStartup() failed with error: " << iResult << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	log_file << "Connecting...\n";
	cout << "Connecting...\n";

	// Resolve the server address and port
	iResult = getaddrinfo(static_cast<PCSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		log_file << "getaddrinfo() failed with error: " << iResult << endl;
		cout << "getaddrinfo() failed with error: " << iResult << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	join_server(ref(client), result);

	log_file << "Shutting down socket..." << endl;
	cout << "Shutting down socket..." << endl;
	iResult = shutdown(client.socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		log_file << "shutdown() failed with error: " << WSAGetLastError() << endl;
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}

int Output(string msg) {
	cout << msg << endl;
	log_file << msg << endl;
	return 1;
}
