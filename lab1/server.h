#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

using namespace std;

class Server
{
	public:
		Server(int, bool);
		~Server();

	private:

		void create();
		void serve();
		void handle(int);
		int parseCommand(string);
		string getMessage(int, int);
		string get_request(int);
		bool send_response(int, string);

		int port_;
		int server_;
		int buflen_;
		string cache_;
		char* buf_;
		bool debugFlag_;
		

		//	Message class for storing messages
    	class Message
		{
    		public:
				Message(string s, string m)
        		{
					this->sub = s;
	        	    this->msg = m;
	        	}
	
		        string sub;
		        string msg;
    	};

		// map to store the messages on the server
		map<string, vector<Message> > postoffice;
};
