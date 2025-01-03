// SimpleNet - https://github.com/GeeTwentyFive/SimpleNet
/*
MIT License

Copyright (c) 2024 Gee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _SIMPLE_NET_H
#define _SIMPLE_NET_H

#define ENET_IMPLEMENTATION
#include "external/enet.h"



#define SimpleNet__PEER_TIMEOUT 5000



typedef ENetEvent SimpleNet__Event;

#define SimpleNet__EVENT_CONNECT ENET_EVENT_TYPE_CONNECT
#define SimpleNet__EVENT_RECEIVE ENET_EVENT_TYPE_RECEIVE
#define SimpleNet__EVENT_DISCONNECT ENET_EVENT_TYPE_DISCONNECT
#define SimpleNet__EVENT_DISCONNECT_TIMEOUT ENET_EVENT_TYPE_DISCONNECT_TIMEOUT

#define SimpleNet__FreePacket enet_packet_destroy

typedef enum {
        SimpleNet__SUCCESS,
        SimpleNet__ERROR_ALREADY_RUNNING,
        SimpleNet__ERROR_FAILED_TO_INITIALIZE_ENET,
        SimpleNet__ERROR_FAILED_TO_CREATE_HOST,
        SimpleNet__ERROR_FAILED_TO_SET_ADDRESS,
        SimpleNet__ERROR_FAILED_TO_CONNECT_TO_SERVER,
        SimpleNet__ERROR_FAILED_TO_CREATE_PACKET,
        SimpleNet__ERROR_FAILED_TO_QUEUE_PACKET,
        SimpleNet__ERROR_SERVICE_FAILED
} SimpleNet__Error;

enum { SimpleNet__SERVER_SHUTDOWN = -1 };



int _SimpleNet__bInitialized = 0;
ENetAddress _SimpleNet__address = {0};
ENetHost *_SimpleNet__host = 0;
ENetPeer *_SimpleNet__peer = 0;



SimpleNet__Error SimpleNet__StartServer(unsigned short port, size_t maxClients) {

        if (!_SimpleNet__bInitialized) {
                if (enet_initialize() != 0) return SimpleNet__ERROR_FAILED_TO_INITIALIZE_ENET;
                _SimpleNet__bInitialized = 1;
        }



        _SimpleNet__address.host = ENET_HOST_ANY;
        _SimpleNet__address.port = port;
        _SimpleNet__host = enet_host_create(&_SimpleNet__address, maxClients, 1, 0, 0);
        if (_SimpleNet__host == NULL) return SimpleNet__ERROR_FAILED_TO_CREATE_HOST;



        return SimpleNet__SUCCESS;

}



SimpleNet__Error SimpleNet__StartClient(char *address, unsigned short port) {

        if (!_SimpleNet__bInitialized) {
                if (enet_initialize() != 0) return SimpleNet__ERROR_FAILED_TO_INITIALIZE_ENET;
                _SimpleNet__bInitialized = 1;
        }



        _SimpleNet__host = enet_host_create(0, 1, 1, 0, 0);
        if (_SimpleNet__host == NULL) return SimpleNet__ERROR_FAILED_TO_CREATE_HOST;

        if (enet_address_set_host(&_SimpleNet__address, address) != 0) return SimpleNet__ERROR_FAILED_TO_SET_ADDRESS;

        _SimpleNet__address.port = port;

        _SimpleNet__peer = enet_host_connect(_SimpleNet__host, &_SimpleNet__address, 1, 0);
        if (_SimpleNet__peer == NULL) return SimpleNet__ERROR_FAILED_TO_CONNECT_TO_SERVER;

        ENetEvent event = {};
        if (enet_host_service(_SimpleNet__host, &event, 5000) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT) {
                enet_peer_reset(_SimpleNet__peer);
                return SimpleNet__ERROR_FAILED_TO_CONNECT_TO_SERVER;
        }



        return SimpleNet__SUCCESS;

}



SimpleNet__Error SimpleNet__Send(ENetPeer *target, void *data, size_t data_size) {

        ENetPacket *packet = enet_packet_create(data, data_size, 0);
        if (packet == NULL) return SimpleNet__ERROR_FAILED_TO_CREATE_PACKET;

        if (target != 0) {
                if (enet_peer_send(target, 0, packet) != 0) return SimpleNet__ERROR_FAILED_TO_QUEUE_PACKET;
        } else if (_SimpleNet__peer != 0) {
                if (enet_peer_send(_SimpleNet__peer, 0, packet) != 0) return SimpleNet__ERROR_FAILED_TO_QUEUE_PACKET;
        } else {
                enet_host_broadcast(_SimpleNet__host, 0, packet);
        }

        enet_host_flush(_SimpleNet__host);

        return SimpleNet__SUCCESS;

}



int SimpleNet__Service(SimpleNet__Event *event_OUT) {
        int status = enet_host_service(_SimpleNet__host, event_OUT, 0);
        if (event_OUT->type == SimpleNet__EVENT_CONNECT) enet_peer_timeout(event_OUT->peer, SimpleNet__PEER_TIMEOUT, SimpleNet__PEER_TIMEOUT, SimpleNet__PEER_TIMEOUT);
        return status;
}



int _SimpleNet__shutdownPacket = SimpleNet__SERVER_SHUTDOWN;

void SimpleNet__Stop() {

        SimpleNet__QueueSend(0, &_SimpleNet__shutdownPacket, sizeof(int));
        enet_host_flush(_SimpleNet__host);

        if (_SimpleNet__peer != 0) {
                enet_peer_disconnect(_SimpleNet__peer, 0);
                enet_host_flush(_SimpleNet__host);

                enet_peer_reset(_SimpleNet__peer);
        }

        enet_host_destroy(_SimpleNet__host);

        _SimpleNet__peer = 0;

}



#endif // _SIMPLE_NET_H
