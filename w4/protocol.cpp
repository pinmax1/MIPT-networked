#include "protocol.h"
#include <cstring> // memcpy
#include "bitstream.h"
void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  *packet->data = E_CLIENT_TO_SERVER_JOIN;

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity),
                                                   ENET_PACKET_FLAG_RELIABLE);
  uint8_t *ptr = packet->data;
  *ptr = E_SERVER_TO_CLIENT_NEW_ENTITY; ptr += sizeof(uint8_t);
  memcpy(ptr, &ent, sizeof(Entity)); ptr += sizeof(Entity);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                                   ENET_PACKET_FLAG_RELIABLE);
  uint8_t *ptr = packet->data;
  *ptr = E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY; ptr += sizeof(uint8_t);
  memcpy(ptr, &eid, sizeof(uint16_t)); ptr += sizeof(uint16_t);

  enet_peer_send(peer, 0, packet);
}

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y)
{
  Bitstream bs;
  bs.Write(E_CLIENT_TO_SERVER_STATE);
  bs.Write(eid);
  bs.Write(x);
  bs.Write(y);
  ENetPacket *packet = enet_packet_create(nullptr, bs.Size(),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);
  memcpy(packet->data, bs.Data(), bs.Size());

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float size)
{
  Bitstream bs;
  bs.Write(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.Write(eid);
  bs.Write(x);
  bs.Write(y);
  bs.Write(size);
  ENetPacket *packet = enet_packet_create(nullptr, bs.Size(),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);
  memcpy(packet->data, bs.Data(), bs.Size());
  enet_peer_send(peer, 1, packet);
}

void send_teleport(ENetPeer* peer, float x, float y, float size) {
  Bitstream bs;
  bs.Write(E_SERVER_TO_CLIENT_TELEPORT);
  bs.Write(x);
  bs.Write(y);
  bs.Write(size);
  ENetPacket *packet = enet_packet_create(nullptr, bs.Size(),
                                                   ENET_PACKET_FLAG_RELIABLE);
  memcpy(packet->data, bs.Data(), bs.Size());
  enet_peer_send(peer, 0, packet);
}

void send_points(ENetPeer* peer, float points, int eid) {
  Bitstream bs;
  bs.Write(E_SERVER_TO_CLIENT_POINTS);
  bs.Write(points);
  bs.Write(eid);
  ENetPacket *packet = enet_packet_create(nullptr, bs.Size(),
                                                   ENET_PACKET_FLAG_RELIABLE);
  memcpy(packet->data, bs.Data(), bs.Size());
  enet_peer_send(peer, 0, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  ent = *(Entity*)(ptr); ptr += sizeof(Entity);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  eid = *(uint16_t*)(ptr); ptr += sizeof(uint16_t);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  Bitstream bs(packet->data, packet->dataLength);
  bs.Skip<uint8_t>();
  bs.Read(eid);
  bs.Read(x);
  bs.Read(y);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float& size)
{
  Bitstream bs(packet->data, packet->dataLength);
  bs.Skip<uint8_t>();
  bs.Read(eid);
  bs.Read(x);
  bs.Read(y);
  bs.Read(size);
}

void deserialize_teleport(ENetPacket *packet, float &x, float &y, float& size)
{
  Bitstream bs(packet->data, packet->dataLength);
  bs.Skip<uint8_t>();
  bs.Read(x);
  bs.Read(y);
  bs.Read(size);
}

void deserialize_points(ENetPacket* packet, float& points, int& eid) {
  Bitstream bs(packet->data, packet->dataLength);
  bs.Skip<uint8_t>();
  bs.Read(points);
  bs.Read(eid);
}

