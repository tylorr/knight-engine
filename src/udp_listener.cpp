#include "udp_listener.h"

#include "event_header_generated.h"
#include "monster_generated.h"

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

void UdpListener::Poll() {
  using namespace knight::events;

  auto event = ENetEvent{};

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
          auto event_header = GetEventHeader(event.packet->data);
          auto event_type = event_header->event_type();

          if (event_type == Event_Monster) {
            auto monster = reinterpret_cast<const Monster *>(event_header->event());

            INFO("Received monster event mana: %d foo: %d", monster->mana(), monster->foo());
          }

          /* Clean up the packet now that we're done using it. */
          enet_packet_destroy(event.packet);
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
}

} // namespace knight
