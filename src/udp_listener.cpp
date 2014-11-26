#include "udp_listener.h"

#include "events.h"

#include <logog.hpp>

namespace knight {

void UdpListener::Start(int port) {
  auto enet_callbacks = ENetCallbacks{knight_malloc, knight_free, knight_no_memory};
  auto result = enet_initialize_with_callbacks(ENET_VERSION, &enet_callbacks);
  XASSERT(result == 0, "An error occurred while initializing ENet");

  address_.host = ENET_HOST_ANY;
  address_.port = port;
  server_ = enet_host_create(&address_, 32, 2, 0, 0);

  XASSERT(server_ != nullptr, "An error occurred while trying to create an ENet server host");
}

void UdpListener::Stop() {
  enet_host_destroy(server_);
  enet_deinitialize();
}

bool UdpListener::Poll(foundation::Array<Event> &events) {
  using namespace knight::events;

  auto event = ENetEvent{};

  bool events_received = false;

  while (enet_host_service(server_, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        INFO("A new client connected from %x:%u.", 
                event.peer->address.host,
                event.peer->address.port);
        /* Store any relevant client information here. */
        event.peer->data = (void *)"Client";
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        {
          events_received = true;
          auto *event_header = GetEventHeader(event.packet->data);
          foundation::array::emplace_back(events, event_header, event.packet);
        }
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        INFO("%s disconnected.", event.peer->data);
        /* Reset the peer's client information. */
        event.peer->data = NULL;
      case ENET_EVENT_TYPE_NONE:
        break;
    }
  }

  return events_received;
}

} // namespace knight
