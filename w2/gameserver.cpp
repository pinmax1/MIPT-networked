#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

bool msg_starts_with(const std::string& msg, const std::string& prefix) {
    return msg.substr(0, prefix.size()) == prefix;
}

std::vector<std::string_view> split(const std::string_view& str, char delim)
{
    std::vector<std::string_view> result; 
    auto left = str.begin(); 
    for (auto it = left; it != str.end(); ++it) 
    { 
        if (*it == delim) 
        { 
            result.emplace_back(&*left, it - left); 
            left = it + 1; 
        } 
    } 
    if (left != str.end()) 
        result.emplace_back(&*left, str.end() - left); 
    return result; 
}


void send_pos(const std::string& msgPos, int id, ENetHost* server) {
    std::string msg = "/pos " + msgPos + " " + std::to_string(id);
    ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(server, 1, packet);
}

void send_ping(ENetPeer* peer, ENetHost* server, int id) {
    std::string msg = "/ping " + std::to_string(peer->roundTripTime) + " " + std::to_string(id);
    ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(server, 1, packet);
}



int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  enet_address_set_host(&address, "localhost");
  address.port = 10900;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  std::vector<ENetPeer*> clients;

  std::unordered_map<ENetPeer*, int> peersToId;
  std::unordered_map<int, std::pair<float, float>> idToPos;

  int cur_id = 0;

  const std::vector<std::string> names = {
    "Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Hank",
    "Ivy", "Jack", "Kate", "Leo", "Mia", "Nathan", "Olivia", "Paul",
    "Quinn", "Rachel", "Sam", "Tina", "Ulysses", "Vera", "Walter", "Xena",
    "Yvonne", "Zack", "Amelia", "Benjamin", "Clara", "Daniel", "Emma", "Felix"
  };



  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }
  uint32_t timeStart = enet_time_get();
  uint32_t lastSendTime = timeStart;
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      std::string msg;
      ENetPacket* packet;
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        clients.push_back(event.peer);
        peersToId[event.peer] = cur_id;
        msg = "/playerlist ";
        for(int i = 0; i < cur_id; ++i) {
            msg += std::to_string(i) + " " + names[i] + " ";
        }

        packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event.peer, 0, packet);
        

        msg = "/newplayer " + std::to_string(cur_id) + " " + names[cur_id];
        packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(server, 0, packet);
        ++cur_id;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        printf("Packet received '%s'\n", event.packet->data);
        msg = std::string(reinterpret_cast<char*>(event.packet->data));
        if(msg_starts_with(msg, "/pos")) {
            std::string msgPos = msg.substr(strlen("/pos") + 1);
            std::cout << "Player " << names[peersToId[event.peer]] << " on position " << msgPos << '\n';
            send_pos(msgPos, peersToId[event.peer], server);

        }
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    uint32_t time = enet_time_get();
    if(time - lastSendTime > 150) {
        lastSendTime = time;
        for(ENetPeer* client : clients) {
            send_ping(client, server, peersToId[client]);
        }
    }

  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
