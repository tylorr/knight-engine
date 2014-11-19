#pragma once

#include "common.h"

#include <enet/enet.h>

namespace knight {

namespace events { class EventHeader; }

class UdpListener {
 public:
  UdpListener() : address_(), server_(nullptr) { }

  void Start(int port);
  void Stop();
  void Poll();

 private:
  ENetAddress address_;
  ENetHost *server_;
};

} // namespace knight
