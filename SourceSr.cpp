#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>



#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"
#define MESLEN 128

struct client_type
{
	int id;
	SOCKET socket;

};

const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 5;

int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{
	std::string msg = "";
	char tempmsg[MESLEN] = "";
	recv(new_client.socket, tempmsg, MESLEN, 0);

	while (1)
	{
		memset(tempmsg, 0, MESLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, tempmsg, MESLEN, 0);

			if (iResult != SOCKET_ERROR)
			{
				if (strcmp("", tempmsg))
					msg = "Client #" + std::to_string(new_client.id) + ": " + tempmsg;

				std::cout << msg.c_str() << std::endl;


				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
						if (new_client.id != i)
							iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}
			}
			else
			{
				msg = "Client #" + std::to_string(new_client.id) + " Disconnected";

				std::cout << msg << std::endl;

				closesocket(new_client.socket);
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;


				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
						iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}

				break;
			}
		}
	}

	thread.detach();

	return 0;
}

int main()
{
	WSADATA wsaData;
	struct addrinfo hints;
	struct addrinfo *server = NULL;
	SOCKET server_socket = INVALID_SOCKET;
	std::string msg = "";
	std::vector<client_type> client(MAX_CLIENTS);
	int num_clients = 0;
	int temp_id = -1;
	std::thread my_thread[MAX_CLIENTS];

	
	WSAStartup(MAKEWORD(2, 2), &wsaData);


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	getaddrinfo(static_cast<LPSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &server);

	

	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
																					

	bind(server_socket, server->ai_addr, (int)server->ai_addrlen);

	listen(server_socket, SOMAXCONN);

	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client[i] = { -1, INVALID_SOCKET };
	}
	

	while (1)
	{

		SOCKET present = INVALID_SOCKET;
		present = accept(server_socket, NULL, NULL);

		if (present == INVALID_SOCKET) continue;


		num_clients = -1;


		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (client[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				client[i].socket = present;
				client[i].id = i;
				temp_id = i;
			}

			if (client[i].socket != INVALID_SOCKET)
				num_clients++;


		}

		if (temp_id != -1)
		{

			std::cout << "Client #" << client[temp_id].id << " Accepted" << std::endl;
			msg = std::to_string(client[temp_id].id);
			send(client[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			
			my_thread[temp_id] = std::thread(process_client, std::ref(client[temp_id]), std::ref(client), std::ref(my_thread[temp_id]));
		}
		else
		{
			msg = "Server is full";
			send(present, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	} 


	closesocket(server_socket);


	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(client[i].socket);
	}


	WSACleanup();
	std::cout << "Program has ended successfully" << std::endl;

	system("pause");
	return 0;
}