#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "client.h"

using namespace std;

int main(int argc, char **argv)
{
    int option;

    // setup default arguments
    int port = 3000;
    string server = "localhost";

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"s:p:")) != -1)
	{
        switch (option)
		{
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                server = optarg;
                break;
            default:
                cout << "client [-s server] [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    Client client = Client(server, port);
}

