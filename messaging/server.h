#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <queue>

using namespace std;

class Server
{
	public:
		Server(int, bool);
		~Server();
		
		void handle(int, string&, char*);

	private:

		void create();
		void serve();
		int parseCommand(string);
		string getMessage(int, int, string&, char*);
		string get_request(int, string&, char*);
		bool send_response(int, string);

		int port_;
		int server_;
		int buflen_;
		int threadCount_;
		int maxQueueSize_;
		bool debugFlag_;
		vector<pthread_t*> threads_;
		queue<int> cliQue_;
		sem_t emptyQSlot_;
		sem_t filledQSlot_;
		sem_t queueLock_;
		sem_t poLock_;

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
