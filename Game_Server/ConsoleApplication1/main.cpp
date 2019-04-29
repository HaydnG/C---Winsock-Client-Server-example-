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
#include <algorithm>

#pragma comment (lib, "Ws2_32.lib")
using namespace std;

ofstream log_file;
int total_clients;
int total_spectators;

const char OPTION_VALUE = 1;

string IP;
string PORT;

//Function Prototypes

int main();
Server SetupServer();


int GetOption();
int ReadConfig();

int ReadConfig() {
	ifstream File("ip_config.txt");

	if (File.is_open()) {
		string Line;
		while (getline(File, Line)) {
			Line.erase(std::remove_if(Line.begin(), Line.end(), isspace),Line.end());
			if (Line[0] == '#' || Line.empty())
				continue;

			int delimiterPos = Line.find("=");
			string name = Line.substr(0, delimiterPos);
			string value = Line.substr(delimiterPos + 1);

			if (name == "ip" || name == "IP") {
				IP = value;
			}
			else if(name == "port" || name == "PORT"){
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



int GetOption() {

	int option = 0;
	while (!(cin >> option)){
		cin.clear();
		cin.ignore(256, '\n');

	}
	return option;

}

Server SetupServer() {
	int option = 0, maxc = 3, minc = 2;

	cout << "Maximum number of players" << " (SERVER MAX #" << MAX_CONNECTIONS << "): ";
	maxc = GetOption();

	cout << "Minimum number of players (MIN 1): ";
	minc = GetOption();

	Server GameServer(maxc, minc);
	cout << endl;

	//Gamemdoe select
	cout << "Please select the gamemode." << endl;
	
	for (int i = 0; i < GameModes.size(); i++) {
		cout << to_string(i) << ") " << GameModes[i] << endl;
	}
	do {
		cout << "Option: ";
		option = GetOption();
	} while (option < 0 || option >  GameModes.size());
	GameServer.Game_mode = option;
	cout << endl;

	//Map select
	cout << "Please select the map." << endl;

	for (int i = 0; i < Maps.size(); i++) {
		cout << to_string(i) << ") " << Maps[i] << endl;
	}
	do {
		cout << "Option: ";
		option = GetOption();
	} while (option < 0 || option >  Maps.size());
	GameServer.Map = option;
	cout << endl;

	//Difficulty select
	cout << "Please select the diffuculty." << endl;
	for (int i = 0; i < Difficulties.size(); i++) {
		cout << to_string(i) << ") " << Difficulties[i] << endl;
	}
	do {
		cout << "Option: ";
		option = GetOption();
	} while (option < 0 || option >  Difficulties.size());
	GameServer.Difficulty = option;
	cout << endl;
	cout << endl;

	return GameServer;
}

int main()
{
	WSADATA wsaData;
	addrinfo hints;
	addrinfo *server = NULL;
	
	log_file.open("server_log.txt");

	ReadConfig();

	//Setup server configurations
	Server GameServer = SetupServer();
	string msg = " (Max_clients: " + to_string(GameServer.MAX_CLIENTS) + ", Min_clients : " + to_string(GameServer.MIN_CLIENTS) + ", Max_Spectators : " + to_string(GameServer.MAX_SPECTATORS) + ")";

	cout << endl;

	Output("### Starting server ###" + msg);
	Output("Gamemode <" + GameModes[GameServer.Game_mode] + ">" + " Map<" + Maps[GameServer.Map] + "> Difficulty<" + Difficulties[GameServer.Difficulty] + ">");
	cout << endl;
	cout << endl;

	

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
	getaddrinfo(static_cast<PCSTR>(IP.c_str()), PORT.c_str(), &hints, &server);

	//Create a listening socket for connecting to server
	Output("Creating server socket...");
	GameServer.socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Setup socket options
	setsockopt(GameServer.socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(GameServer.socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	//Assign an address to the server socket.
	Output("Binding socket...");
	int tmp = ::bind(GameServer.socket, server->ai_addr, (int)server->ai_addrlen);

	//Listen for incoming connections.
	Output("Listening...");
	listen(GameServer.socket, SOMAXCONN);

	//Start server
	cout << endl;
	Output("Server ready... at address (" + IP+":"+PORT+")");
	cout << endl;



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

int Log(string msg) {
	log_file << msg << endl;
	return 1;
}


