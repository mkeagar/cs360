#include "server.h"

void* threadCommander(void* input);

// structure to hold data passed to a thread
typedef struct threadData_
{	
	Server* server;
    int client;
  	char* buffer;
  	string cache;   
} threadData;

Server::Server(int port, bool debug)
{
    // setup variables
    port_ = port;
    buflen_ = 1024;
    threadCount_ = 10;
    debugFlag_ = debug;

    // create and run the server
    create();
    serve();
}

Server::~Server(){}

void Server::create()
{
	if (debugFlag_)
	{
		cout << "[DEBUG] Creating server..." << endl;
	}
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_)
	{
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
	{
        perror("bind");
        exit(-1);
    }

      // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0)
	{
        perror("listen");
        exit(-1);
    }
    
    char host_[128] = "";
    gethostname(host_, sizeof(host_));
    
    cout << "Server running on " << host_ << "\nServer listening on port " << port_ << endl;
    
    /**** create array of threads ****/
    int i = 0;
    for (i = 0; i < threadCount_; i++)
    {
    	threadData* data = new threadData;
    	data->server = this;
    	data->buffer = new char[buflen_+1];
    	data->cache = "";
    	
    	pthread_t* thread = new pthread_t;
    	pthread_create(thread, NULL, &threadCommander, (void*) data);
    }
}

void Server::serve()
{
	if (debugFlag_)
	{
		cout << "[DEBUG] Server preparing to serve..." << endl;
	}
    // setup client
    int client = 0;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0)
	{
		// wait queue empty
		// get queue lock
		// put client in queue
		// signal queue lock
		/**** signal queue counting semaphore ****/	
    }
    close(server_);
}

void* threadCommander(void* input)
{
	threadData* data;
	data = (threadData*) input;
	
	while (true)
	{
		// wait on queue counting semaphore
		// wait on queue lock
		// get the client int out of the queue
		// signal the queue lock
		// signal the queue counting semaphore
		data->server->handle(data->client, data->cache, data->buffer);
		close(data->client);
	}
}

void Server::handle(int client, string& cache, char* buffer)
{
	if (debugFlag_)
	{
		cout << "[DEBUG] entering handle() method" << endl;
	}
	string response = ""; 
    // loop to handle all requests
    while (true)
	{
		//empty the response string
		response = "";
		
        // get a request
        string request = get_request(client, cache, buffer);
        // break if client is done or an error occurred
        if (request.empty())
            break;
		
		// if we didn't get an error, parse the command
        int commandNum = parseCommand(request);
        
        switch(commandNum)
        {
        	case 1:		// "put" command, so store the message
        	{
        		if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> put" << endl;
				
				string command = "";
				string user = "";
				string subject = "";
				string message = "";
				int length = 0;
				stringstream ss (request);
	
				if (ss >> command >> user >> subject >> length)
				{
					if (length > 0)
					{
						message = getMessage(client, length, cache, buffer);
						
						if (debugFlag_)
						{
							cout << "[DEBUG] Message is: " << message << endl;
						}
						// wait on map lock
						map<string, vector<Message> >::iterator itr = this->postoffice.find(user);
						if (itr == postoffice.end())               //We didn't find user in the postoffice
						{
							postoffice[user] = vector<Message>();
						}
						postoffice[user].push_back(Message(subject, message));
						// signal map lock
						response = "OK\n";	
					}
					else
						response = "error INVALID MESSAGE LENGTH\n";
				}
				else
					response = "error INVALID PUT REQUEST\n";
					
        		break;
        	}
        		
    		case 2:		// "list" command, so tell them what's in the postoffice
    		{
    			if (debugFlag_)
    			{
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> list" << endl;
        		}
        			
        		string command = "";
				string user = "";
				stringstream ss (request);
				stringstream rs;
	
				if (ss >> command >> user)
				{
						// wait on map lock
						map<string, vector<Message> >::iterator itr = this->postoffice.find(user);
						if (itr == postoffice.end())
						{
							rs << "list 0\n";
						}	
						else
						{
							rs << "list " << postoffice[user].size() << "\n";
							
							int i = 0;
							for (i = 0; i < postoffice[user].size(); i++)
							{
								rs << (i+1) << " " << postoffice[user].at(i).sub << "\n";
							}
						}
						// signal map lock
						response = rs.str();
						
						if (debugFlag_)
						{
							cout << "[DEBUG] Sending Response: \n" << response << "[DEBUG] End response" << endl;
						}
				}
				else
				{
					response = "error INVALID LIST REQUEST\n";
        		}	
    			break;
    		}
    			
			case 3:		// "get" command, send them what they want
			{
				if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> get" << endl;
    			
				string command = "";
				string user = "";
				int index = 0;
				stringstream ss (request);
				stringstream rs;
	
				if (ss >> command >> user >> index)
				{
						//wait on map lock
						map<string, vector<Message> >::iterator itr = this->postoffice.find(user);
						if (itr == postoffice.end())
						{
							rs << "error USER \"" << user << "\" NOT FOUND\n";
						}
						else if (postoffice[user].size() < index || index <= 0)
						{
							rs << "error NO MESSAGE AT INDEX " << index << endl;
						}
						else
						{
							rs << "message " << postoffice[user].at(index-1).sub << " "  << postoffice[user].at(index-1).msg.length() << "\n" << postoffice[user].at(index-1).msg;
						}
						// signal map lock
						response = rs.str();
				}
				else
				{
					response = "error INVALID GET REQUEST\n";
        		}
        			
    			break;
			}
			
			case 4:		// "reset" command, empty the postoffice
				if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> reset" << endl;
        		// wait on map lock
				postoffice.clear();
				// signal map lock
				response = "OK\n";
				break;
				
			default:	// default behavior!
				if (debugFlag_)
				{
					cout << "[DEBUG] There was an error! You should not see this message!" << endl;
				}
				response = "error SERVER ERROR\n";
				break;
        }

		// send response
		bool success = send_response(client,response);
		
		// break if an error occurred
		if (!success)
		{	
			if (debugFlag_)
				cout << "[DEBUG] There was an error when sending the response" << endl;
			break;	
		}
    }
}

int Server::parseCommand(string commandMessage)
{
	if (debugFlag_)
	{
		cout << "[DEBUG] processing parseCommand()" << endl;
	}
	stringstream ss (commandMessage);
	string command = "";
	ss >> command;
	
	if (command == "put")
		return 1;
	else if (command == "list")
		return 2;
	else if (command == "get")
		return 3;
	else if (command == "reset")
		return 4;
	else
		return -1;
}

string Server::getMessage(int client, int numBytes, string& cache, char* buffer)
{
	if (debugFlag_)
	{
		cout << "[DEBUG] processing getMessage()" << endl;
	}
	
	string request = cache;
    // read until we get the number of bytes we wanted
    while (request.length() < numBytes)
	{
        int nread = recv(client,buffer,buflen_,0);
        if (nread < 0)
		{
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        }
		else if (nread == 0)
		{
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buffer,nread);
    }
	cache = request.substr(numBytes);
    return request.substr(0, numBytes);
}

string Server::get_request(int client, string& cache, char* buffer)
{
	if (debugFlag_)
	{
		cout << "[DEBUG] processing get_request()" << endl;
	}
    string request = cache;
    // read until we get a newline
    while (request.find("\n") == string::npos)
	{
        int nread = recv(client,buffer,buflen_,0);
        if (nread < 0)
		{
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        }
		else if (nread == 0)
		{
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buffer,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
	cache = request.substr(request.find("\n")+1);
    return request.substr(0, request.find("\n"));
}

bool Server::send_response(int client, string response)
{
	if (debugFlag_)
	{
		cout << "[DEBUG] processing send_response()" << endl;
	}
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft)
	{
        if ((nwritten = send(client, ptr, nleft, 0)) < 0)
		{
            if (errno == EINTR)
			{
                // the socket call was interrupted -- try again
                continue;
            }
			else
			{
                // an error occurred, so break out
                perror("write");
                return false;
            }
        }
		else if (nwritten == 0)
		{
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}
