#include "server.h"

Server::Server(int port, bool debug)
{
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    debugFlag_ = debug;
    cache_ = "";
    errMsg_ = "";

    // create and run the server
    create();
    serve();
}

Server::~Server()
{
    delete buf_;
}

void Server::create()
{
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
}

void Server::serve()
{
    // setup client
    int client = 0;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0)
	{

        handle(client);
        close(client);
    }
    
    close(server_);

}

void Server::handle(int client)
{
	string response = ""; 
    // loop to handle all requests
    while (1)
	{
		//empty the response string
		response = "";
		
        // get a request
        string request = get_request(client);    
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
						message = getMessage(client, length);
						map<string, vector<Message> >::iterator itr = this->postoffice.find(user);
						if (itr == postoffice.end())               //We didn't find user in the postoffice
							postoffice[user] = vector<Message>();
				
						postoffice[user].push_back(Message(subject, message));
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
    			if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> list" << endl;
    			break;
    			
			case 3:		// "get" command, send them what they want
				if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> get" << endl;
				break;
			
			case 4:		// "reset" command, empty the postoffice
				if (debugFlag_)
        			cout << "[DEBUG] RECEIVED CLIENT REQUEST -> reset" << endl;
				break;
				
			default:	// default behavior!
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

string Server::getMessage(int client, int numBytes)
{
	string request = cache_;
	
    // read until we get a newline
    while (request.length() < numBytes)
	{
        int nread = recv(client,buf_,1024,0);
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
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
	if (numBytes < request.length())	
		cache_ = request.substr(numBytes);
	
    return request.substr(0, numBytes);
}

string Server::get_request(int client)
{
    string request = cache_;
    // read until we get a newline
    while (request.find("\n") == string::npos)
	{
        int nread = recv(client,buf_,1024,0);
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
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
	cache_ = request.substr(request.find("\n")+1);
    return request.substr(0, request.find("\n"));
}

bool Server::send_response(int client, string response)
{
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
