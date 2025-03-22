#pragma once
#include <cstdint>
#include <enet/enet.h>
#include "entity.h"
#include <string>
enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_STATE,
  E_SERVER_TO_CLIENT_SNAPSHOT,
  E_SERVER_TO_CLIENT_TELEPORT,
  E_SERVER_TO_CLIENT_POINTS
};

void send_join(ENetPeer *peer);
void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_set_controlled_entity(ENetPeer *peer, uint16_t eid);
void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y);
void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float size);
void send_teleport(ENetPeer* peer, float x, float y, float size);
void send_points(ENetPeer* peer, float points, int eid);

MessageType get_packet_type(ENetPacket *packet);

void deserialize_new_entity(ENetPacket *packet, Entity &ent);
void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid);
void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y);
void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float& size);
void deserialize_teleport(ENetPacket *packet, float &x, float &y, float& size);
void deserialize_points(ENetPacket* packet, float& points, int& eid);
