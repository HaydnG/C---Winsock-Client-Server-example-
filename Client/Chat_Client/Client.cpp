
#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <algorithm>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
string IP;
string PORT;

ofstream log_file;

struct client_type
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFLEN];
	int SPECTATOR;
};

int process_client(client_type &new_client);
int join_server();
int Output(string msg);
int CommandDispatcher(client_type &new_client);
int ReadConfig();
int GetOption();
int main();

int GetOption() {

	int option = 0;
	while (!(cin >> option)) {
		cin.clear();
		cin.ignore(256, '\n');

	}
	return option;

}

int ReadConfig() {
	ifstream File("ip_config.txt");

	if (File.is_open()) {
		string Line;
		while (getline(File, Line)) {
			Line.erase(std::remove_if(Line.begin(), Line.end(), isspace), Line.end());
			if (Line[0] == '#' || Line.empty())
				continue;

			int delimiterPos = Line.find("=");
			string name = Line.substr(0, delimiterPos);
			string value = Line.substr(delimiterPos + 1);

			if (name == "ip" || name == "IP") {
				IP = value;
			}
			else if (name == "port" || name == "PORT") {
				PORT = value;
			}
		}

		File.close();
	}
	else {
		cerr << "Couldn't open config file for reading.\n";
	}

	return 1;
}

int join_server() {
	struct addrinfo *result = NULL, hints;
	client_type client = { INVALID_SOCKET, -1, "" , 0 };
	string message;
	string sent_message = "";
	string response = "";
	int Result = 0, CommandID = 8;

	ReadConfig();

	while (CommandID == 8) {
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		// Resolve the server address and port
		Result = getaddrinfo(static_cast<PCSTR>(IP.c_str()), PORT.c_str(), &hints, &result);
		if (Result != 0) {
			log_file << "getaddrinfo() failed with error: " << Result << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// Attempt to connect to an address until one succeeds
		for (result = result; result != NULL; result = result->ai_next) {

			// Create a SOCKET for connecting to server
			client.socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (client.socket == INVALID_SOCKET) {
				log_file << "socket() failed with error : " << WSAGetLastError() << endl;
				WSACleanup();
				system("pause");
				return 1;
			}

			// Connect to server.
			Result = connect(client.socket, result->ai_addr, (int)result->ai_addrlen);
			if (Result == SOCKET_ERROR) {
				log_file << "socket() failed with error : " << result->ai_family << endl;
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
			return 1;
		}
		else {
			cout << endl;
			Output("Successfully connect to (" + IP + ":" + PORT + ")");
			cout << endl;
		}

		//Obtain id from server for this client;
		recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
		message = client.received_message;


		if (message == "Server is full - NO spectator slots") {
			cout << message << endl;
			cout << endl;
			break;
		}else if (message != "Server is full")
		{
			client.id = atoi(client.received_message);
			client.SPECTATOR = 0;

			thread runtime(process_client, ref(client));

			cout << "Type 'quit' to leave." << endl << endl;

			CommandID = CommandDispatcher(ref(client));
			runtime.detach();
		}else {
			cout << message << endl;
			cout << endl;
			cout << "Would you like to become a spectator? [Y/N]" << endl;
			sent_message = "";

			while (1) {

				getline(cin, sent_message);

				if (sent_message == "Y" || sent_message == "y" || sent_message == "N" || sent_message == "n") {
					Result = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

					if (Result <= 0)
					{
						log_file << "send() failed: " << WSAGetLastError() << endl;
						cout << "send() failed: " << WSAGetLastError() << endl;
					}
					else if (sent_message == "Y" || sent_message == "y") {
						recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
						client.id = atoi(client.received_message);
						client.SPECTATOR = 1;

						thread runtime(process_client, ref(client));

						cout << endl;
						cout << "Type 'quit' to leave or 'join' to join if a slot availible." << endl << endl;


						CommandID = CommandDispatcher(ref(client));
						runtime.detach();
						break;
					}
				}
			}
		}

	}


	log_file << "Shutting down socket..." << endl;
	cout << "Shutting down socket..." << endl;
	Result = shutdown(client.socket, SD_SEND);
	if (Result == SOCKET_ERROR) {
		log_file << "shutdown() failed with error: " << WSAGetLastError() << endl;
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);


	return 1;
}

int CommandDispatcher(client_type &client) {
	string response;

	while (1)
	{
		response = "";
		getline(cin, response);
		if (response == "quit" || response == "Quit") {

			send(client.socket, response.c_str(), strlen(response.c_str()), 0);

			cout << "Closing game...." << endl;

			return 1;
		}

		if (client.SPECTATOR == 1) {
		
			if (response == "join" || response == "Join") {

				send(client.socket, response.c_str(), strlen(response.c_str()), 0);

				cout << "Attempting to join game..." << endl;
				cout << endl;
				cout << endl;

				client.id = -1;
				client.socket = INVALID_SOCKET;

				return 8;
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

		if (new_client.socket != INVALID_SOCKET)
		{
			int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

			if (new_client.socket != INVALID_SOCKET) {

				if (iResult != SOCKET_ERROR) {
					message = new_client.received_message;
					prefix = message.substr(0, message.find(delimiter));
					message.erase(0, message.find(delimiter) + delimiter.length());

					if (prefix == "text") {
						cout << message << endl;
						log_file << message << endl;
					}

				}
				else
				{
					Output("Server not responding, closing.....");
					break;
				}

			}
			else
			{
				break;
			}
		}
		else
		{
			break;
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
	int iResult = 0;
	int CommandID = 8;
	int option = 0;
	log_file.open("client_log.txt");


	cout << "Select an option to proceed.." << endl;
	cout << "0) Join" << endl;
	cout << "1) Quit" << endl;
	do {
		cout << "Option: ";
		option = GetOption();
	} while (option < 0 || option >  1);

	cout << endl << endl;

	if (option == 0) {

		log_file << "Starting Client...\n";
		cout << "Starting Client...\n";

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (iResult != 0) {
			log_file << "WSAStartup() failed with error: " << iResult << endl;
			cout << "WSAStartup() failed with error: " << iResult << endl;
			return 1;
		}



		log_file << "Connecting...\n";
		cout << "Connecting...\n";
		join_server();

	}
	else {
	
	}
	
	WSACleanup();
	system("pause");
	return 0;
}

int Output(string msg) {
	cout << msg << endl;
	log_file << msg << endl;
	return 1;
}
