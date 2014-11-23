#pragma once

#include "common.h"

#include <enet/enet.h>
#include <array.h>

namespace knight {

namespace events { class EventHeader; }

class Event {
 public:
  const events::EventHeader *header;

  Event() : header{nullptr}, packet_{nullptr} { }
  Event(const events::EventHeader *event_header, ENetPacket *packet)
    : header{event_header}, 
      packet_{packet} { }

  ~Event() {
    enet_packet_destroy(packet_);
  }

 private:
  ENetPacket *packet_;
};

class UdpListener {
 public:
  UdpListener() : address_(), server_(nullptr) { }

  void Start(int port);
  void Stop();
  bool Poll(foundation::Array<Event> &events);

 private:
  ENetAddress address_;
  ENetHost *server_;
};

} // namespace knight
