#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Client
{
	public:
		Client(string, int, bool);
		~Client();

	private:

		void create();
		void getInput();
		bool send_request(string);
		bool get_response();
		int parseCommand(string);
		string processCommand(int, string, string);
		
		int port_;
		string host_;
		int server_;
		int buflen_;
		char* buf_;
		bool run_;
		bool debugFlag_;
};
