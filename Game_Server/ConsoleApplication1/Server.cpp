#include "pch.h"
#include <winsock2.h>
#include <thread>
#include <iostream>

using namespace std;

const string CLIENT_PREFIX =  "Client ";


Server::Server(int x, int y) {

	MAX_CLIENTS = x;
	MAX_SPECTATORS = y;

	//Initialize the client list
	for (int i = 0; i < MAX_CLIENTS + MAX_SPECTATORS + 1; i++)
	{
	
		clients.push_back({ -1, INVALID_SOCKET , 0 , 1});
	}
}

int Server::process_client(client_type &new_client, thread &thread)
{
	//ping client
	if (new_client.socket != INVALID_SOCKET) {
		ping(ref(new_client));

	}
	thread.detach();
	new_client.Detatched = 1;
	return 0;
}

int Server::process_spectator(client_type &new_spectator, thread &thread)
{
	string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";
	int temp_id;
	int Result;
	//temporarily disable spectator until reqeust accepted
	temp_id = new_spectator.id;
	new_spectator.id = -1;

	while (1) {
		memset(tempmsg, 0, DEFAULT_BUFLEN);
		if (new_spectator.socket != 0)
		{
			Result = recv(new_spectator.socket, tempmsg, DEFAULT_BUFLEN, 0);

			if (Result != SOCKET_ERROR)
			{
				if (strcmp("Y", tempmsg) || strcmp("y", tempmsg)) {
					new_spectator.id = temp_id;
					msg = to_string(new_spectator.id);
					send(new_spectator.socket, msg.c_str(), strlen(msg.c_str()), 0);
					break;
				}
			}
			else {
				closesocket(new_spectator.socket);
				
			}
			
			break;
		}
	}
	//ping client
	if (new_spectator.socket != INVALID_SOCKET && new_spectator.id != -1) {
		
		ping(ref(new_spectator));
	}
	thread.detach();
	new_spectator.socket = INVALID_SOCKET;
	new_spectator.Detatched = 1;

	return 0;
}

string Server::GetPlayerType(client_type &new_client) { 
	if (new_client.SPECTATOR == 0) {
		return " (Player) ";
	}
	else {
		return " (Spectator) ";
	}
}

string Server::GetCurrentSlots() {
	UpdateConCount();
	return " (" + to_string(num_clients) + " / " + to_string(MAX_CLIENTS) + ") ";
}

int Server::CommandListener(client_type &new_client, thread &thread) {
	char received_message[DEFAULT_BUFLEN];
	string message, prefix, msg;
	string delimiter = "/";
	int temp_id = -1;
	while (1)
	{
		memset(received_message, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, received_message, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR) {
				message = received_message;
				prefix = message.substr(0, message.find(delimiter));
				message.erase(0, message.find(delimiter) + delimiter.length());

				if (strcmp("Quit", prefix.c_str()) || strcmp("quit", prefix.c_str())) {
				
					temp_id = new_client.id;
					closesocket(new_client.socket);

					new_client.socket = INVALID_SOCKET;
					msg = CLIENT_PREFIX + '#' + to_string(temp_id) + GetPlayerType(new_client) + "has left the game." + GetCurrentSlots();

					Broadcast(msg);
					Output(msg);
				


					break;

				}else if ((strcmp("Join", prefix.c_str()) || strcmp("join", prefix.c_str())) && new_client.SPECTATOR == 1) {

					temp_id = new_client.id;
					closesocket(new_client.socket);
					new_client.socket = INVALID_SOCKET;
					msg = CLIENT_PREFIX + '#' + to_string(temp_id) + GetPlayerType(new_client) + "is attempting to switch from a spectator.";

					Broadcast(msg);
					Output(msg);



					break;

				}

			}
			else
			{
				break;
			}
		}
	}
	

	
	thread.detach();
	

	return 1;

}

int Server::ping(client_type &new_client) {
	int Result;
	string msg;
	int ping = 0;
	int temp_id = -1;;

	int threadID = MAX_CLIENTS + MAX_SPECTATORS + new_client.id;


	threads[threadID] = thread(&Server::CommandListener, this, ref(Server::clients[new_client.id]), ref(Server::threads[threadID]));


	msg = CLIENT_PREFIX + '#' + to_string(new_client.id) + " has joined as a" + GetPlayerType(new_client) + GetCurrentSlots();
	Broadcast(msg);
	Output(msg);
	Sleep(5 * 1000);
	while (1)
	{
		if (new_client.socket != INVALID_SOCKET)
		{
			
			msg = "ping/";
			Result = send(new_client.socket, msg.c_str(), strlen(msg.c_str()), 0);

			if (Result != SOCKET_ERROR)
			{
				ping = 0;
			}
			else
			{
				//Log server retrying connection
				msg = CLIENT_PREFIX + '#' + to_string(new_client.id) + GetPlayerType(new_client) + " not responding - Retry (" + to_string(ping) + ")";
				Output(msg);
				ping++;
				//Wait 30 seconds for client to respond, then disconnect
				if (ping == 6) {
					temp_id = new_client.id;
					closesocket(new_client.socket);
					new_client.socket = INVALID_SOCKET;
					msg = CLIENT_PREFIX + '#' + to_string(temp_id) + GetPlayerType(new_client) + "has lost connection..." + GetCurrentSlots();

					Broadcast(msg);
					Output(msg);
					
					break;
				}
			}
			Sleep(5 * 1000);
		}
		else {
			break;
		}
	}

	new_client.id = -1;

	return 1;
}

int Server::Broadcast(string msg) {

	string prefix = "text/";

	for (int i = 0; i < MAX_CLIENTS + MAX_SPECTATORS; i++)
	{
		if (clients[i].socket != INVALID_SOCKET) {
			send(clients[i].socket, (prefix + msg).c_str(), strlen(msg.c_str()) + strlen(prefix.c_str()), 0);	
		}
	}
	return 1;
}

void Server::UpdateConCount() {
	//Reset the number of clients
	string msg;
	num_clients = 0;
	num_spectators = 0;

	for (int i = 0; i < MAX_CLIENTS+MAX_SPECTATORS; i++)
	{
		if (clients[i].socket != INVALID_SOCKET && clients[i].SPECTATOR == 0)
			num_clients++;
		if (clients[i].socket != INVALID_SOCKET && clients[i].SPECTATOR == 1)
			num_spectators++;
	}

	Broadcast("There are " + to_string(MAX_CLIENTS - num_clients) + " availible slots left!");
}

int Server::GetFreeID() {
	int temp_id = -1;
	for (int i = 0; i < MAX_CLIENTS + MAX_SPECTATORS; i++)
	{
		if (clients[i].socket == INVALID_SOCKET && temp_id == -1 && clients[i].Detatched == 1)
		{
			temp_id = i;
			break;
		}
	
	}

	return temp_id;
}

int Server::Start() {
	int temp_id = -1;
	string msg = "";

	


	while (1)
	{
		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(socket, NULL, NULL);
		temp_id = -1;

		if (incoming != INVALID_SOCKET) {	


			//Update Client count
			UpdateConCount();
			if (num_clients < MAX_CLIENTS) {

				//Find ID for client
				temp_id = GetFreeID();
				clients[temp_id].socket = incoming;
				clients[temp_id].id = temp_id;
				clients[temp_id].SPECTATOR = 0;
				clients[temp_id].Detatched = 0;

				//Update Client count
				UpdateConCount();
			}

			if (temp_id != -1)
			{
				//Send the id to that client
				msg = to_string(clients[temp_id].id);
				send(clients[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

				//Create a thread process for that client
				threads[temp_id] = thread(&Server::process_client, this, ref(Server::clients[temp_id]), ref(Server::threads[temp_id]));
			}
			else
			{
				msg = "Server is full";
				send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
				Output(msg);


				//Update Client count
				UpdateConCount();
				if (num_spectators < MAX_SPECTATORS) {
					//Find ID for client
					temp_id = GetFreeID();
					clients[temp_id].socket = incoming;
					clients[temp_id].id = temp_id;
					clients[temp_id].SPECTATOR = 1;
					clients[temp_id].Detatched = 0;

					//Update Client count
					UpdateConCount();
				}

				if (temp_id != -1)
				{
					//Create a thread process for that client
					threads[temp_id] = thread(&Server::process_spectator,this, ref(Server::clients[temp_id]), ref(Server::threads[temp_id]));
				}
			}
		}
	} //end while

	Close();
}

void Server::Close() {
	//Close listening socket
	closesocket(socket);

	//Close client socket
	for (int i = 0; i < MAX_CLIENTS + MAX_SPECTATORS; i++)
	{
		threads[i].detach();
		closesocket(clients[i].socket);
	}
}