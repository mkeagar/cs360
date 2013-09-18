#include "client.h"

Client::Client(string host, int port, bool debug)
{
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    debugFlag_ = debug;

    // connect to the server and run getInput()
    create();
    getInput();
}

Client::~Client()
{
	delete(buf_);
}

void Client::create()
{
    struct sockaddr_in server_addr;

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry)
	{
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_)
	{
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
	{
        perror("connect");
        exit(-1);
    }
}

void Client::getInput()
{
	string line = "";
    string commandMessage = "";
    string message = "";
    string sendToServer = "";
    bool doneTyping = false;
    int commandNum = -1;
    
    // runs until the quit command is called
    while (run_)
    {	
    	message = "";
    	line = "";
    	commandMessage = "";
    	sendToServer = "";
    	doneTyping = false;
    	commandNum = -1;
        cout << "% ";
        getline(cin, commandMessage);
        commandNum = parseCommand(commandMessage);
        
        switch (commandNum)
        {
        	case 1:			// send
        	{
        		if (debugFlag_)
        			cout << "[DEBUG] The user entered the send command.  Now we get the rest of the message." << endl;
        		
        		cout << "- Type your message. End with a blank line -" << endl;
        		while (!doneTyping)
        		{
					getline(cin, line);
					if (line == "")	// We've reached the end of the message.
						doneTyping = true;
					else				// Append the current line.
					{
						line += "\n";
						message += line;
					}
        		}
        		
        		if (debugFlag_)
        		{
        			cout << "[DEBUG] the message to send is: " << message << endl;
				}
				
				sendToServer = processCommand(commandNum, commandMessage, message);
				if (debugFlag_)
				{
					cout << "[DEBUG] sendToServer: " << sendToServer << endl;
				}
				
				bool success = send_request(sendToServer);
				
				//break if an error occurred
				if (!success)
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] send_request() was not successful." << endl;
					}
					continue;
				}
				  
				// get a response
				string response = get_response();
				
				if (debugFlag_)
				{
					cout << "[DEBUG] server response is: \n" << response << endl;
				}	
				
				// break if an error occurred
				if (response.empty())
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] get_response() was not successful." << endl;
					}
					continue;
				}
				
				if (response != "OK\n")
				{
					cout << response;
				}
				break;
			}
        	
        	case 2:			// list
        	{
        		if (debugFlag_)
        		{
        			cout << "[DEBUG] The user requested the list of messages for a user." << endl;
        		}
        			
        		sendToServer = processCommand(commandNum, commandMessage, message);
				if (debugFlag_)
				{
					cout << "[DEBUG] sendToServer: " << sendToServer << endl;
				}
				bool success = send_request(sendToServer);
				//break if an error occurred
				if (!success)
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] send_request() was not successful." << endl;
					}
					continue;
				}
				  
				// get a response
				string response = get_response();
				if (debugFlag_)
				{
					cout << "[DEBUG] server response is: \n" << response << endl;
				}	
				// break if an error occurred
				if (response.empty())
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] get_response() was not successful." << endl;
					}
					continue;
				}
				
				int numItems = 0;
				int index = 0;
				string fWord = "";
				stringstream rs(response);
				rs >> fWord >> numItems;
				
				if (fWord == "error")
				{
					cout << response;
					break;
				}
				
				for (index = 0; index < numItems; index++)
				{
					response += get_response();
				}
				cout << response;
        		break;	
        	}

        	case 3:			// read
        	{
        		if (debugFlag_)
        		{
        			cout << "[DEBUG] The user requested to read a message for a user." << endl;
        		}
        			
        		sendToServer = processCommand(commandNum, commandMessage, message);
				if (debugFlag_)
				{
					cout << "[DEBUG] sendToServer: " << sendToServer << endl;
				}
				bool success = send_request(sendToServer);
				//break if an error occurred
				if (!success)
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] send_request() was not successful." << endl;
					}
					continue;
				}
				  
				// get a response
				string response = get_response();
				if (debugFlag_)
				{
					cout << "[DEBUG] server response is: \n" << response << endl;
				}	
				// break if an error occurred
				if (response.empty())
				{
					if (debugFlag_)
					{
						cout << "[DEBUG] get_response() was not successful." << endl;
					}
					continue;
				}
				
				int length = 0;
				string fWord = "";
				string subject = "";
				stringstream rstream (response);
				rstream >> fWord >> subject >> length;
				
				if (fWord == "error")
				{
					cout << response;
					break;
				}
								
				response += getMessage(length);		
				cout << response;
				//if response doesn't have a newline character at the end, add one on
				if (response.at(response.length()-1) != '\n')
				{
					cout << endl;
				}
        		break;
        	}	
        		
			case 4:			// quit
				if (debugFlag_)
				{
					cout << "[DEBUG] The client will now quit." << endl;
				}
				run_ = false;
				break;
				
			default:
				cout << "That is not a valid command." << endl;
				if (debugFlag_)
				{
					cout << "[DEBUG] The user has typed an invalid command." << endl;
				}
        
        }
    }
    close(server_);
}

int Client::parseCommand(string commandMessage)
{
	stringstream ss (commandMessage);
	string command = "";
	ss >> command;
	
	if (command == "send")
		return 1;
	else if (command == "list")
		return 2;
	else if (command == "read")
		return 3;
	else if (command == "quit")
		return 4;
	else
		return -1;
}

string Client::processCommand(int commandNum, string commandMessage, string message)
{	
	string command = "";
	string user = "";
	string subject = "";
	string index = "";
	int length;
	stringstream ss (commandMessage);
	stringstream requestMessage("");
	
	switch (commandNum)
	{
		case 1:				// sending a message, so let's put it together.
			ss >> command >> user >> subject;
			length = message.length();
			requestMessage << "put " << user << " " << subject << " " << length << endl << message;
			break;
			
		case 2:
			ss >> command >> user;
			requestMessage << command << " " << user << endl;
			break;
			
		case 3:
			ss >> command >> user >> index;
			requestMessage << "get " << user << " " << index << endl;
			break;
			
		default:
			if (debugFlag_)
				cout << "You should never see this debug statement." << endl;
			break;
	}
	
	return requestMessage.str();
}


bool Client::send_request(string request)
{
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0)
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


string Client::get_response()
{
	string response = cache_;
    // read until we get a newline
    while (response.find("\n") == string::npos)
	{
        int nread = recv(server_,buf_,1024,0);
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
        response.append(buf_,nread);
    }
	// a better client would cut off anything after the newline and
	// save it in a cache
	cache_ = response.substr(response.find("\n")+1);
	return response.substr(0, response.find("\n")+1);
}

string Client::getMessage(int numBytes)
{
	string message = cache_;
	
    // read until we get the number of bytes we wanted
    while (message.length() < numBytes)
	{
        int nread = recv(server_,buf_,1024,0);
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
        message.append(buf_,nread);
    }
	
	if (debugFlag_)
	{
		cout << "[DEBUG] getMessage() - state of cache_ before request substring copy: \n" << cache_ << endl;
	}
	cache_ = message.substr(numBytes);
	if (debugFlag_)
	{
		cout << "[DEBUG] getMessage() - state of cache_ after request substring copy: \n" << cache_ << endl;
	}
    return message.substr(0, numBytes+1);
}
