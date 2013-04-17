
/* Client is used to ping server for a file name
 * Usage: ./Client -f filename -s serverIP
 */


#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "Defs.h"
#include "myUtils.h"


using namespace std;

void sendRequestToServer(string serverIP,string filename)
{
	//Socket variables for client
	int client_sockfd,n;
	struct sockaddr_in servaddr;
	socklen_t lensock;
		
	client_sockfd = socket(AF_INET,SOCK_DGRAM,0);

	if(client_sockfd == -1)
	{
		printf("Could not create socket \n");
		exit(EXIT_FAILURE);
	}
		
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(CLIENT_PORT);

	int bval = bind(client_sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	if(bval==-1)
	{
		printf("Bind failed\n");
		exit(EXIT_FAILURE);
	}


	
}




int main(int argc,char **argv)
{
	
	int command;
	
	cout<<"List of operations available are: "<<endl;
	cout<<"0 - put"<<endl;
	cout<<"1 - get"<<endl;
	cout<<"2 - exists"<<endl;
	cout<<"3 - ls"<<endl;
	cout<<"4 - delete"<<endl;

	while(1)
	{
		cout<<"Please enter the operation (0-4) you intend to do: "<<endl;
		cin >> command;

		switch(command)
		{
			case 0:
			{
				cout<< "You selected put"<<endl;
				cout<< "Please enter the file name"<<endl;
				
				string filename;
				cin>>filename;

				cout<< "Please enter the IP address of the server"<<endl;

				string serverIP;
				cin>>serverIP;


				
				break;
			}
			case 1:
			{
				cout<< "You selected get"<<endl;
				cout<< "Please enter the file name"<<endl;

				string filename;
				cin>>filename;

				break;
			}
			case 2:
			{
				cout<< "You selected exists"<<endl;
				cout<< "Please enter the file name"<<endl;

				string filename;
				cin>>filename;

				break;
			}
			case 3:
			{
				cout<< "You selected ls"<<endl;


				break;
			}
			case 4:
			{
				cout<< "You selected delete"<<endl;
				cout<< "Please enter the file name"<<endl;

				string filename;
				cin>>filename;


				break;
			}
			default:
			{
				cout<<"Please select an option from the available choices"<<endl;
			}
		}
	}

/*
	if(argc < 5)
	{
		printf("Usage: Client -f filename -s serverIP\n");
		exit(EXIT_FAILURE);
	}

	int iterator;

	string serverIP;

	string filename;

	for(iterator=0;iterator<argc;iterator++)
	{	
			string arg = argv[iterator];

			if(arg == "-f")
			{
					if(argv[iterator+1])
					{
						filename = argv[iterator+1];
						iterator = iterator+1;
					}
					else
					{
						printf("Please specify filename\n");
						exit(EXIT_FAILURE);
					}
			}

			if(arg == "-s")
			{
				if(argv[iterator+1])
				{
					serverIP = argv[iterator+1];
					iterator = iterator + 1;
				}
				else
				{
					printf("Please specify server IP\n");
					exit(EXIT_FAILURE);
				}
			}
		
	}
	
	printf("Queried server %s for file: %s \n",serverIP.c_str(),filename.c_str());
	
*/
}
