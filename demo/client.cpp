#include "monster_generated.h"
#include "event_header_generated.h"

#include <enet/enet.h>

#include <cstdio>

using namespace knight;
using namespace knight::events;

#define HOST "localhost"
#define PORT 1234

ENetHost *client;
ENetAddress address;
ENetPeer *peer;

void Disconnect();

int main() {
  setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

  int connected = 0;

  if (enet_initialize() != 0) {
    fprintf(stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  }
  atexit (enet_deinitialize);
  
  client = enet_host_create(nullptr /* create a client host */,
              1 /* only allow 1 outgoing connection */,
              2 /* allow up 2 channels to be used, 0 and 1 */,
              57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
              14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
  if (client == nullptr) {
    fprintf(stderr, 
             "An error occurred while trying to create an ENet client host.\n");
    exit(EXIT_FAILURE);
  }

  enet_address_set_host(&address, HOST);
  address.port = PORT;

  peer = enet_host_connect(client, &address, 2, 0);

  if (peer == nullptr) {
    printf("Could not connect to server\n");
    return 0;
  }

  ENetEvent event;

  if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {

    printf("Connection to %s succeeded.\n", HOST);
    connected++;

    flatbuffers::FlatBufferBuilder builder;

    auto monster_location = CreateMonster(builder, 10, 20);
    auto event_location = CreateEventHeader(builder, Event_Monster, monster_location.Union());

    FinishEventHeaderBuffer(builder, event_location);

    printf("Sending packet\n");
    ENetPacket * packet = enet_packet_create(builder.GetBufferPointer(), builder.GetSize(), ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);

    enet_host_flush(client);

    Disconnect();

  } else {
    enet_peer_reset(peer);
    printf("Could not connect to %s.\n", HOST);
  }

  enet_host_destroy(client);
}

void Disconnect() {
  ENetEvent event;
  enet_peer_disconnect(peer, 0);

  while (enet_host_service(client, & event, 3000) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy(event.packet);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Disconnection succeeded.\n");
        return;
    }
  }

  // Did not disconnect in time, force disconnect
  enet_peer_reset(peer);
}
