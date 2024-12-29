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

#include <pthread.h>

#define ENET_IMPLEMENTATION
#include "external/enet.h"



typedef void (*SimpleNet__EventCallback)(ENetEvent*);



int _SimpleNet__bInitialized = 0;
ENetAddress _SimpleNet__address = {0};
ENetHost *_SimpleNet__host = 0;
ENetPeer *_SimpleNet__peer = 0;
pthread_t _SimpleNet__threadID = 0;
SimpleNet__EventCallback _SimpleNet__eventCallback = 0;



typedef enum {
        SIMPLENET__SUCCESS,
        SIMPLENET__ERROR_FAILED_TO_INITIALIZE_ENET,
        SIMPLENET__ERROR_FAILED_TO_CREATE_HOST,
        SIMPLENET__ERROR_FAILED_TO_SET_ADDRESS,
        SIMPLENET__ERROR_FAILED_TO_CONNECT_TO_SERVER,
        SIMPLENET__ERROR_FAILED_TO_CREATE_PACKET,
        SIMPLENET__ERROR_FAILED_TO_QUEUE_PACKET,
        SIMPLENET__ERROR_FAILED_TO_START_THREAD
} SimpleNet__Error;







void* _SimpleNet__Thread(void*) {

        ENetEvent event;
        while (enet_host_service(_SimpleNet__host, &event, 1) >= 0) {

                if (event.type != ENET_EVENT_TYPE_NONE) _SimpleNet__eventCallback(&event);

        }

}







SimpleNet__Error SimpleNet__StartServer(unsigned short port, size_t maxClients, SimpleNet__EventCallback eventCallback) {

        _SimpleNet__eventCallback = eventCallback;

        if (!_SimpleNet__bInitialized) {
                if (enet_initialize() != 0) return SIMPLENET__ERROR_FAILED_TO_INITIALIZE_ENET;
                _SimpleNet__bInitialized = 1;
        }



        _SimpleNet__address.host = ENET_HOST_ANY;
        _SimpleNet__address.port = port;
        _SimpleNet__host = enet_host_create(&_SimpleNet__address, maxClients, 1, 0, 0);
        if (_SimpleNet__host == NULL) return SIMPLENET__ERROR_FAILED_TO_CREATE_HOST;

        if (pthread_create(&_SimpleNet__threadID, 0, &_SimpleNet__Thread, 0) != 0) return SIMPLENET__ERROR_FAILED_TO_START_THREAD;



        return SIMPLENET__SUCCESS;

}



SimpleNet__Error SimpleNet__StartClient(char *address, unsigned short port, SimpleNet__EventCallback eventCallback) {

        _SimpleNet__eventCallback = eventCallback;

        if (!_SimpleNet__bInitialized) {
                if (enet_initialize() != 0) return SIMPLENET__ERROR_FAILED_TO_INITIALIZE_ENET;
                _SimpleNet__bInitialized = 1;
        }



        _SimpleNet__host = enet_host_create(0, 1, 1, 0, 0);
        if (_SimpleNet__host == NULL) return SIMPLENET__ERROR_FAILED_TO_CREATE_HOST;

        if (enet_address_set_host(&_SimpleNet__address, address) != 0) return SIMPLENET__ERROR_FAILED_TO_SET_ADDRESS;

        _SimpleNet__address.port = port;

        _SimpleNet__peer = enet_host_connect(_SimpleNet__host, &_SimpleNet__address, 1, 0);
        if (_SimpleNet__peer == NULL) return SIMPLENET__ERROR_FAILED_TO_CONNECT_TO_SERVER;

        ENetEvent event = {0};
        if (enet_host_service(_SimpleNet__host, &event, 5000) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT) {
                enet_peer_reset(_SimpleNet__peer);
                return SIMPLENET__ERROR_FAILED_TO_CONNECT_TO_SERVER;
        }

        if (pthread_create(&_SimpleNet__threadID, 0, &_SimpleNet__Thread, 0) != 0) return SIMPLENET__ERROR_FAILED_TO_START_THREAD;



        return SIMPLENET__SUCCESS;

}



int SimpleNet__Send(void *data, size_t data_size) {

        ENetPacket *packet = enet_packet_create(data, data_size, 0);
        if (packet == NULL) return SIMPLENET__ERROR_FAILED_TO_CREATE_PACKET;

        if (_SimpleNet__peer != 0) {
                if (enet_peer_send(_SimpleNet__peer, 0, packet) != 0) return SIMPLENET__ERROR_FAILED_TO_QUEUE_PACKET;
        } else {
                enet_host_broadcast(_SimpleNet__host, 0, packet);
        }

        return SIMPLENET__SUCCESS;

}



void SimpleNet__Stop() {

        if (_SimpleNet__peer != 0) {
                enet_peer_disconnect(_SimpleNet__peer, 0);
                enet_host_flush(_SimpleNet__host);

                enet_peer_reset(_SimpleNet__peer);
        }

        pthread_cancel(_SimpleNet__threadID);

        enet_host_destroy(_SimpleNet__host);

        _SimpleNet__peer = 0;

}



#endif // _SIMPLE_NET_H