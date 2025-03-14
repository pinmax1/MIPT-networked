#include "raylib.h"
#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include <string>

void send_fragmented_packet(ENetPeer *peer)
{
  const char *baseMsg = "Stay awhile and listen. ";
  const size_t msgLen = strlen(baseMsg);

  const size_t sendSize = 2500;
  char *hugeMessage = new char[sendSize];
  for (size_t i = 0; i < sendSize; ++i)
    hugeMessage[i] = baseMsg[i % msgLen];
  hugeMessage[sendSize-1] = '\0';

  ENetPacket *packet = enet_packet_create(hugeMessage, sendSize, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  delete[] hugeMessage;
}

void send_micro_packet(ENetPeer *peer)
{
  const char *msg = "dv/dt";
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}

void send_start_message(ENetPeer *peer) {
  const char* msg = "/s";
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
}

bool msg_starts_with(const std::string& msg, const std::string& prefix) {
  return msg.substr(0, prefix.size()) == prefix;
}

std::vector<std::string> split(std::string str, char delim)
{
    std::vector<std::string> result; 
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


void parse_list_players(const std::string& msgPlayersList, std::unordered_map<int,std::string>& idToName) {
  std::vector<std::string> splited = split(msgPlayersList, ' ');
  std::cout << "Players list: ";
  for(int i = 0; i < splited.size(); i += 2) {
    idToName[atoi(splited[i].c_str())] = splited[i+1];
    std::cout << splited[i+1] << " ";
  }
  std::cout << '\n';
}

void parse_new_player(const std::string& msgPlayersList, std::unordered_map<int,std::string>& idToName) {
  std::vector<std::string> splited = split(msgPlayersList, ' ');
  std::cout << "New player joined ";
  for(int i = 0; i < splited.size(); i += 2) {
    idToName[atoi(splited[i].c_str())] = splited[i+1];
    std::cout << splited[i+1] << " ";
  }
  std::cout << '\n';
}

void send_position(ENetPeer* peer, float x, float y) {
  std::string msg = "/pos " + std::to_string(x) + " " + std::to_string(y);
  ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}


void parse_position(const std::string& msgPos, std::unordered_map<int, std::pair<float,float>>& idToPos) {
  std::vector<std::string> splited = split(msgPos, ' ');
  idToPos[atoi(splited[2].c_str())] = {std::stof(splited[0]), std::stof(splited[1])};
  std::cout << "Player with id " << splited[2] << " on position " << splited[0] << " " << splited[1] << '\n';
}

void parse_ping(const std::string& msgPing, std::unordered_map<int, int>& idToPing) {
  std::vector<std::string> splited = split(msgPing, ' ');
  idToPing[atoi(splited[1].c_str())] = atoi(splited[0].c_str());
  std::cout << "Player with id " << splited[1] << " has ping " << splited[0] << '\n';
}
 
int main(int argc, const char **argv)
{
  int width = 800;
  int height = 600;
  InitWindow(width, height, "w2 MIPT networked");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10887;

  ENetPeer *lobbyPeer = enet_host_connect(client, &address, 2, 0);
  if (!lobbyPeer)
  {
    printf("Cannot connect to lobby");
    return 1;
  }

  ENetAddress gameserverAddress;
  ENetPeer* gameServerPeer;

  uint32_t timeStart = enet_time_get();
  uint32_t lastFragmentedSendTime = timeStart;
  uint32_t lastMicroSendTime = timeStart;
  bool connected = false;
  float posx = GetRandomValue(100, 1000);
  float posy = GetRandomValue(100, 500);
  float velx = 0.f;
  float vely = 0.f;
  bool gameserverConnected = false;
  std::unordered_map<int, std::string> idToName;
  std::unordered_map<int, int> idToPing;
  std::unordered_map<int, std::pair<float, float>> idToPos;
  while (!WindowShouldClose())
  {
    const float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0)
    {
      std::string msg;
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        
        msg = std::string(reinterpret_cast<char*>(event.packet->data));

        if(msg_starts_with(msg, "/connect")) {
          std::string address_str = msg.substr(9);
          size_t delim = address_str.find(":");
          gameserverAddress.host = atoi(address_str.substr(0, delim).c_str());
          gameserverAddress.port = atoi(address_str.substr(delim + 1).c_str());
          gameServerPeer = enet_host_connect(client, &gameserverAddress, 2, 0);
          gameserverConnected = true;
          if (!gameServerPeer)
          {
            printf("Cannot connect to game server");
            return 1;
          }
        }
        if(msg_starts_with(msg, "/playerlist")) {
          std::string msgPlayersList = msg.substr(strlen("/playerlist") + 1);
          parse_list_players(msgPlayersList, idToName);
        }

        if(msg_starts_with(msg, "/newplayer")) {
          std::string msgPlayersList = msg.substr(strlen("/newplayer") + 1);
          parse_new_player(msgPlayersList, idToName);
        }

        if(msg_starts_with(msg, "/pos")) {
          std::string msgPos = msg.substr(strlen("/pos") + 1);
          parse_position(msgPos, idToPos);
        }
        if(msg_starts_with(msg, "/ping")) {
          std::string msgPing = msg.substr(strlen("/ping") + 1);
          parse_ping(msgPing, idToPing);
        }

        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }

    bool enter = IsKeyDown(KEY_ENTER);
    if (connected)
    {
      // uint32_t curTime = enet_time_get();
      // if (curTime - lastFragmentedSendTime > 1000)
      // {
      //   lastFragmentedSendTime = curTime;
      // }
      // if (curTime - lastMicroSendTime > 100)
      // {
      //   lastMicroSendTime = curTime;
      // }
      if(enter) {
        uint32_t curTime = enet_time_get();
        if (curTime - lastFragmentedSendTime > 150)
        {
          send_start_message(lobbyPeer);
          lastFragmentedSendTime = curTime;
        }
      }
    }

    if(gameserverConnected) {
      send_position(gameServerPeer, posx, posy);
    }

    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);

    constexpr float accel = 30.f;
    velx += ((left ? -1.f : 0.f) + (right ? 1.f : 0.f)) * dt * accel;
    vely += ((up ? -1.f : 0.f) + (down ? 1.f : 0.f)) * dt * accel;
    posx += velx * dt;
    posy += vely * dt;
    velx *= 0.99f;
    vely *= 0.99f;

    BeginDrawing();
      ClearBackground(BLACK);
      if(!gameserverConnected) {
        DrawText(TextFormat("Current status: %s", "unknown"), 20, 20, 20, WHITE);
      } else {
        DrawText(TextFormat("Current status: %s", "game started"), 20, 20, 20, WHITE);
      }
      DrawText(TextFormat("My position: (%d, %d)", (int)posx, (int)posy), 20, 40, 20, WHITE);
      DrawText("List of players:", 20, 60, 20, WHITE);
      int i = 1;
      for(auto [id, name] : idToName) {
        DrawText((name + " " + std::to_string(idToPing[id])).c_str(), 20, 60 + i * 20, 20, WHITE);
        ++i;
      }
      if(gameserverConnected) {
        for(auto [id, pos] : idToPos) {
          DrawCircleV(Vector2{pos.first, pos.second}, 10.f, WHITE);
        }
      } else {
        DrawCircleV(Vector2{posx, posy}, 10.f, WHITE);
      }
    EndDrawing();
  }
  return 0;
}
