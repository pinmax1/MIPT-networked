#include <enet/enet.h>
#include <iostream>
#include <string>
#include <vector>

void send_gameserver_address(ENetPeer* peer, const ENetAddress& gameserver_address) {
  std::string msg = "/connect " + std::to_string(gameserver_address.host) + ":" + std::to_string(gameserver_address.port);
  ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
}



int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10887;

  ENetAddress gameserver_address;

  enet_address_set_host(&gameserver_address, "localhost");
  gameserver_address.port = 10900;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  std::vector<ENetPeer*> clients;

  

  bool gameStarted = false;

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      std::string msg;
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        clients.push_back(event.peer);
        if(gameStarted) {
          send_gameserver_address(event.peer, gameserver_address);
        }
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        printf("Packet received '%s'\n", event.packet->data);
        msg = std::string(reinterpret_cast<char*>(event.packet->data));
        if(msg == "/s" && !gameStarted) {
          std::cout << "Game started!" << '\n';
          gameStarted = true;
          for(ENetPeer* peer : clients) {
            send_gameserver_address(peer, gameserver_address);
          }
        }
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}

