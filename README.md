Simple cross-platform client-server UDP networking library for both IPv4 *and* IPv6

# Usage:
1) `SimpleNet__StartServer(port, maxClients, eventCallback)` or `SimpleNet__StartClient(ip, port, eventCallback)`
2) Handle connections & received data in passed event callback function
3) Send data via `SimpleNet__Send(data, data_size)` -> Sends to server as client, broadcasts to all clients as server

Optionally disconnect client or stop server via `SimpleNet__Stop()`

# Example:
```c
#include <stdio.h>

#include "SimpleNet.h"



void NetworkEventsCallback(ENetEvent *event) {

	puts("Event!");

	switch (event->type) {

		case ENET_EVENT_TYPE_CONNECT:

			puts("Client connected!");

			SimpleNet__Send("Hello, client!", sizeof("Hello, client!"));

		break;



		case ENET_EVENT_TYPE_RECEIVE:

			puts("Got a message! :");

			puts(event->packet->data);

			enet_packet_destroy(event->packet);

		break;



		case ENET_EVENT_TYPE_DISCONNECT:

			puts("Client disconnected");

		break;

	}

}



int main(int argc, char **argv) {

	if (argc != 2) return 1;

	if (argv[1][0] == 's') SimpleNet__StartServer(42069, 1, &NetworkEventsCallback);
	else if (argv[1][0] == 'c') {
		SimpleNet__StartClient("localhost", 42069, &NetworkEventsCallback);
		SimpleNet__Send("Hello, server!", sizeof("Hello, server!"));
	}
	else return 1;



	getchar();



	SimpleNet__Stop();



	return 0;

}
```
