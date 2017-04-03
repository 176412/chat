#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define MESLEN 128          
#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"

struct client_type
{
	SOCKET socket;
	int id;
	char received_message[MESLEN];
	string name;
};

int process_client(client_type &new_client)
{
	while (1)
	{
		memset(new_client.received_message, 0, MESLEN);

		if (new_client.socket != 0)
		{
			int res = recv(new_client.socket, new_client.received_message, MESLEN, 0);

			if (res != SOCKET_ERROR)
				cout << new_client.received_message << endl;
			else
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected" << endl;

	return 0;
}

int main()
{
	WSAData wsa_data;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "" };
	int res = 0;
	string message;

	cout << "Starting Client...\n";

	res = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (res != 0) {
		cout << "WSAStartup() failed with error: " << res << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	cout << "Connecting...\n";


	res = getaddrinfo(static_cast<LPSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (res != 0) {
		cout << "getaddrinfo() failed with error: " << res << endl;
		WSACleanup();
		system("pause");
		return 1;
	}


	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {


		client.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) {
			cout << "socket() failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}


		res = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (res == SOCKET_ERROR) {
			closesocket(client.socket);
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (client.socket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Successfully Connected" << endl;


	recv(client.socket, client.received_message, MESLEN, 0);
	message = client.received_message;

	if (message != "Server is full")
	{
		client.id = atoi(client.received_message);


		thread my_thread(process_client, client);

		

		while (1)
		{
			cout << client.name << ":";
			getline(cin, sent_message);

			res = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (res <= 0)
			{
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		}


		my_thread.detach();
	}
	else
		cout << client.received_message << endl;


	res = shutdown(client.socket, SD_SEND);
	if (res == SOCKET_ERROR) {
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}