Simple cross-platform client-server UDP networking library for both IPv4 *and* IPv6

# Usage:
1) `SimpleNet__StartServer(port, maxClients, eventCallback)` or `SimpleNet__StartClient(ip, port, eventCallback)`
2) `SimpleNet__Service()` -> Handle connections & received data
3) Send data via `SimpleNet__QueueSend(data, data_size)` -> Sends to server as client, broadcasts to all clients as server

Optionally disconnect client or stop server via `SimpleNet__Stop()`

# Example:
```c
#include <stdio.h>
#include <signal.h>

#include "SimpleNet.h"



void InterruptHandler() { SimpleNet__Stop(); exit(0); }



int main(int argc, char **argv) {

	if (argc != 2) return 1;

	if (argv[1][0] == 's') SimpleNet__StartServer(42069, 1);
	else if (argv[1][0] == 'c') {
		SimpleNet__StartClient("localhost", 42069);
		SimpleNet__QueueSend("Hello, server!", sizeof("Hello, server!"));
	}
	else return 1;

	signal(SIGINT, &InterruptHandler);

	for (;;) {
		SimpleNet__Event event;
		while (SimpleNet__Service(&event) > 0) {
			switch (event.type) {
			
				case SimpleNet__EVENT_CONNECT:

					puts("Client connected!");

					SimpleNet__QueueSend("Hello, client!", sizeof("Hello, client!"));

				break;



				case SimpleNet__EVENT_RECEIVE:

					puts("Got a message! :");

					puts(event.packet->data);

					SimpleNet__FreePacket(event.packet);

				break;



				case SimpleNet__EVENT_DISCONNECT:
				case SimpleNet__EVENT_DISCONNECT_TIMEOUT:

					puts("Client disconnected");

				break;

			}
		}
	}

}
```
