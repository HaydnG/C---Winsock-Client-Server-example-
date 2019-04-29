
#include <string>
#include <vector>
#include <winsock2.h>
#include <thread>


#define IP_ADDRESS "192.168.0.88"
#define DEFAULT_PORT "3504"
#define DEFAULT_BUFLEN 512
//Server limit
#define MAX_CONNECTIONS 10

using namespace std;

int Output(string msg);

struct client_type {
	int id;
	SOCKET socket;
	int SPECTATOR;
};

class Server {
private:

public:
	string Game_mode;
	string Map;
	string Difficulty;
	int num_clients;
	int num_spectators;
	int MAX_CLIENTS;
	int MAX_SPECTATORS;

	SOCKET socket = INVALID_SOCKET;
	vector<client_type> clients;
	thread threads[MAX_CONNECTIONS *2];

	Server(int x, int y);

	int Start();
	int Broadcast(std::string msg);
	int ping(client_type &new_client);
	int process_client(client_type &new_client, thread &thread);
	int process_spectator(client_type &new_client, thread &thread);
	int GetFreeID();
	int CommandListener(client_type &new_client, thread &thread);
	string GetPlayerType(client_type &new_client);
	string GetCurrentSlots();
	void UpdateConCount();
	void Close();
};