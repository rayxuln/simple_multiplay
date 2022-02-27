#ifndef _H_SM_ENET_CLASSES_H_
#define _H_SM_ENET_CLASSES_H_

#include "scene/main/node.h"

#include "enet/enet.h"

class SMENetPacket : public Reference {
  GDCLASS(SMENetPacket, Reference);

  ENetPacket *enet_packet;
  bool reliable;
  size_t current_data_pos;
protected:
  static void _bind_methods();

  
public:
  inline ENetPacket *get_enet_object() {
    return enet_packet;
  }
  inline void set_enet_object(ENetPacket *o) {
    enet_packet = o;
  }

  Error create(uint32_t flags);

  size_t get_length();
  void resize(size_t length);

  uint32_t get_flags();

  PoolByteArray get_data();
  void set_data(const PoolByteArray &data);

  void reset_data_pos(size_t pos);
  bool is_data_pos_end();
  void put_string(const String &v);
  String get_string();
  void put_long(int64_t v);
  int64_t get_long();
  void put_int(int32_t v);
  int32_t get_int();
  void put_short(int16_t v);
  int16_t get_short();
  void put_byte(uint8_t v);
  uint8_t get_byte();
  void put_float(double v);
  double get_float();
  void put_var(const Variant &v, bool full_objects);
  Variant get_var(bool allow_objects);

  void put_data_as_var(const Variant &v, bool full_objects);
  Variant get_data_var(bool allow_objects);

  enum Flags {
    FLAG_RELIABLE = ENET_PACKET_FLAG_RELIABLE,
    FLAG_UNSEQUENCED = ENET_PACKET_FLAG_UNSEQUENCED,
    FLAG_NO_ALLOCATE = ENET_PACKET_FLAG_NO_ALLOCATE,
    FLAG_UNRELIABLE_FRAGMENT = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT,
    FLAG_SENT = ENET_PACKET_FLAG_SENT,
  };

  SMENetPacket();
  virtual ~SMENetPacket();
};

class SMENetPeer : public Reference {
  GDCLASS(SMENetPeer, Reference);

  ENetPeer *enet_peer;
protected:
  static void _bind_methods();

public:
  enum PeerState {
    PEER_STATE_DISCONNECTED = ENET_PEER_STATE_DISCONNECTED,
    PEER_STATE_CONNECTING = ENET_PEER_STATE_CONNECTING,
    PEER_STATE_ACKNOWLEDGING_CONNECT = ENET_PEER_STATE_ACKNOWLEDGING_CONNECT,
    PEER_STATE_CONNECTION_PENDING = ENET_PEER_STATE_CONNECTION_PENDING,
    PEER_STATE_CONNECTION_SUCCEEDED = ENET_PEER_STATE_CONNECTION_SUCCEEDED,
    PEER_STATE_CONNECTED = ENET_PEER_STATE_CONNECTED,
    PEER_STATE_DISCONNECT_LATER = ENET_PEER_STATE_DISCONNECT_LATER,
    PEER_STATE_DISCONNECTING = ENET_PEER_STATE_DISCONNECTING,
    PEER_STATE_ACKNOWLEDGING_DISCONNECT = ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT,
    PEER_STATE_ZOMBIE = ENET_PEER_STATE_ZOMBIE
  };

  inline ENetPeer *get_enet_object() {
    return enet_peer;
  }
  inline void set_enet_object(ENetPeer *o) {
    enet_peer = o;
  }

  void disconnect_from_host(uint32_t data);
  void reset();

  Error send_packet(Ref<SMENetPacket> packet, uint8_t chanel_id);

  uint32_t get_rtt();
  String get_address();
  uint16_t get_port();
  uint32_t get_connect_id();
  int get_state();

  SMENetPeer();
  virtual ~SMENetPeer();
};

class SMENetHost : public Reference {
  GDCLASS(SMENetHost, Reference);

  ENetHost *enet_host;
protected:
  static void _bind_methods();

public:
  inline ENetHost *get_enet_object() {
    return enet_host;
  }
  inline void set_enet_object(ENetHost *o) {
    enet_host = o;
  }

  Error create_server(const String &address, uint8_t port, size_t peer_count, size_t channel_limits);
  Error create_client(size_t peer_count, size_t channel_limits);
  void destroy();

  Error host_service(uint32_t timeout);
  bool is_valid();

  Ref<SMENetPeer> connect_to_host(const String &address, uint8_t port, size_t channel_count, uint32_t data);

  void broadcast(Ref<SMENetPacket>, uint8_t channel_id);

  SMENetHost();
  virtual ~SMENetHost();
};

VARIANT_ENUM_CAST(SMENetPacket::Flags)
VARIANT_ENUM_CAST(SMENetPeer::PeerState)
#endif