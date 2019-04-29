
#include <string>
#include <vector>
#include <winsock2.h>
#include <thread>


#define DEFAULT_BUFLEN 512
//Server limit
#define MAX_CONNECTIONS 20


using namespace std;


const vector<string> GameModes = { "Deathmatch", "Capture the flag", "Blood diamond" };
const vector<string> Difficulties = { "Easy", "Medium", "Hard", "Extreme!!!!" };
const vector<string> Maps = { "Wasteland", "Jungle", "Blizzard", "WaterWorld", "Easter Island" };


int Output(string msg);
int Log(string msg);

struct client_type {
	int id;
	SOCKET socket;
	int SPECTATOR;
	int Detatched;
};

class Server {
private:

public:
	int Game_mode;
	int Map;
	int Difficulty;
	int num_clients;
	int num_spectators;
	int MAX_CLIENTS;
	int MIN_CLIENTS;
	int MAX_SPECTATORS;

	//Game stats
	int gamelength = 0;
	int Disconnects = 0;
	int total_specs = 0;

	bool RUNNING = false;
	thread GameProcess;

	

	SOCKET socket = INVALID_SOCKET;
	vector<client_type> clients;
	thread threads[MAX_CONNECTIONS *2];

	Server(int max, int min);

	int Start();
	int Broadcast(string msg, bool Spec, bool Client);
	int ping(client_type &new_client);
	int process_client(client_type &new_client, thread &thread);
	int process_spectator(client_type &new_client, thread &thread);
	int GetFreeID();
	int CommandListener(client_type &new_client, thread &thread);
	int GameRuntime();
	string GetPlayerType(client_type &new_client);
	string GetCurrentSlots();
	void UpdateConCount();
	void Close();
};