

#include "sm_enet_classes.h"

#include "core/io/marshalls.h"

void SMENetPacket::_bind_methods() {
  ClassDB::bind_method(D_METHOD("create", "flags"), &SMENetPacket::create, Flags::FLAG_RELIABLE);
  ClassDB::bind_method(D_METHOD("get_length"), &SMENetPacket::get_length);
  ClassDB::bind_method(D_METHOD("resize", "length"), &SMENetPacket::resize);
  ClassDB::bind_method(D_METHOD("get_flags"), &SMENetPacket::get_flags);

  ClassDB::bind_method(D_METHOD("get_data"), &SMENetPacket::get_data);
  ClassDB::bind_method(D_METHOD("set_data", "data"), &SMENetPacket::set_data);

  ClassDB::bind_method(D_METHOD("reset_data_pos", "pos"), &SMENetPacket::reset_data_pos, 0);
  ClassDB::bind_method(D_METHOD("is_data_pos_end"), &SMENetPacket::is_data_pos_end);
  ClassDB::bind_method(D_METHOD("put_string", "v"), &SMENetPacket::put_string);
  ClassDB::bind_method(D_METHOD("get_string"), &SMENetPacket::get_string);
  ClassDB::bind_method(D_METHOD("put_long", "v"), &SMENetPacket::put_long);
  ClassDB::bind_method(D_METHOD("get_long"), &SMENetPacket::get_long);
  ClassDB::bind_method(D_METHOD("put_int", "v"), &SMENetPacket::put_int);
  ClassDB::bind_method(D_METHOD("get_int"), &SMENetPacket::get_int);
  ClassDB::bind_method(D_METHOD("put_short", "v"), &SMENetPacket::put_short);
  ClassDB::bind_method(D_METHOD("get_short"), &SMENetPacket::get_short);
  ClassDB::bind_method(D_METHOD("put_byte", "v"), &SMENetPacket::put_byte);
  ClassDB::bind_method(D_METHOD("get_byte"), &SMENetPacket::get_byte);
  ClassDB::bind_method(D_METHOD("put_float", "v"), &SMENetPacket::put_float);
  ClassDB::bind_method(D_METHOD("get_float"), &SMENetPacket::get_float);
  ClassDB::bind_method(D_METHOD("put_var", "v", "full_objects"), &SMENetPacket::put_var, false);
  ClassDB::bind_method(D_METHOD("get_var", "allow_objects"), &SMENetPacket::get_var, false);

  ClassDB::bind_method(D_METHOD("put_data_as_var", "v", "full_objects"), &SMENetPacket::put_data_as_var, false);
  ClassDB::bind_method(D_METHOD("get_data_var", "allow_objects"), &SMENetPacket::get_data_var, false);

  BIND_ENUM_CONSTANT(FLAG_RELIABLE);
  BIND_ENUM_CONSTANT(FLAG_UNSEQUENCED);
  BIND_ENUM_CONSTANT(FLAG_NO_ALLOCATE);
  BIND_ENUM_CONSTANT(FLAG_UNRELIABLE_FRAGMENT);
  BIND_ENUM_CONSTANT(FLAG_SENT);
}

Error SMENetPacket::create(uint32_t flags) {
  ENetPacket *p = renet_packet_create(nullptr, 0, flags);
  ERR_FAIL_COND_V(p == nullptr, Error::FAILED);
  enet_packet = p;
  current_data_pos = 0;
  return OK;
}

size_t SMENetPacket::get_length() {
  ERR_FAIL_COND_V(enet_packet == nullptr, 0);
  return enet_packet->dataLength;
}
void SMENetPacket::resize(size_t length) {
  ERR_FAIL_COND(enet_packet == nullptr);
  renet_packet_resize(enet_packet, length);
}

uint32_t SMENetPacket::get_flags() {
  ERR_FAIL_COND_V(enet_packet == nullptr, 0);
  return enet_packet->flags;
}

PoolByteArray SMENetPacket::get_data() {
  PoolVector<uint8_t> res;
  if (enet_packet == nullptr) return res;
  res.resize(enet_packet->dataLength);
  auto w = res.write();
  memcpy(w.ptr(), enet_packet->data, enet_packet->dataLength);
  return res;
}
void SMENetPacket::set_data(const PoolByteArray &data) {
  if (enet_packet == nullptr) return;
  renet_packet_resize(enet_packet, data.size());
  memcpy(enet_packet->data, data.read().ptr(), data.size());
}

void SMENetPacket::reset_data_pos(size_t pos) {
  current_data_pos = pos;
}
bool SMENetPacket::is_data_pos_end() {
  return current_data_pos >= enet_packet->dataLength;
}
void SMENetPacket::put_string(const String &v) {
  if (enet_packet == nullptr) return;
  size_t len = v.utf8().size();
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  memcpy(enet_packet->data+current_data_pos, v.utf8().get_data(), len);
  current_data_pos += len;
}
String SMENetPacket::get_string() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return "";
  size_t len = 0;
  bool found_len = false;
  for (size_t i=current_data_pos; i<enet_packet->dataLength; ++i) {
    if (enet_packet->data[i] == '\0') {
      len = i-current_data_pos+1;
      found_len = true;
      break;
    }
  }
  if (!found_len) len = enet_packet->dataLength - current_data_pos;
  if (len == 0) return "";

  CharString s;
  s.resize(len);
  memcpy(s.ptrw(), enet_packet->data+current_data_pos, len);
  current_data_pos += len;

  String res;
  res.parse_utf8(s.ptr());

  return res;
}
void SMENetPacket::put_int(int32_t v) {
  if (enet_packet == nullptr) return;
  size_t len = 4;
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  for (size_t i=0; i<len; ++i)
    enet_packet->data[current_data_pos+i] = (v>>(8*i)) & 0xff;
  current_data_pos += len;
}
int32_t SMENetPacket::get_int() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return 0;
  size_t len = 4;
  ERR_FAIL_COND_V_MSG(current_data_pos+len > enet_packet->dataLength, 0, "Wrong packet length!");
  int32_t res = 0;
  for (size_t i=0; i<len; ++i) {
    res <<= 8;
    res |= enet_packet->data[current_data_pos+(len-i-1)];
  }
  current_data_pos += len;
  return res;
}
void SMENetPacket::put_long(int64_t v) {
  if (enet_packet == nullptr) return;
  size_t len = 8;
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  for (size_t i=0; i<len; ++i)
    enet_packet->data[current_data_pos+i] = (v>>(8*i)) & 0xff;
  current_data_pos += len;
}
int64_t SMENetPacket::get_long() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return 0;
  size_t len = 8;
  ERR_FAIL_COND_V_MSG(current_data_pos+len > enet_packet->dataLength, 0, "Wrong packet length!");
  int64_t res = 0;
  for (size_t i=0; i<len; ++i) {
    res <<= 8;
    res |= enet_packet->data[current_data_pos+(len-i-1)];
  }
  current_data_pos += len;
  return res;
}
void SMENetPacket::put_short(int16_t v) {
  if (enet_packet == nullptr) return;
  size_t len = 2;
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  for (size_t i=0; i<len; ++i)
    enet_packet->data[current_data_pos+i] = (v>>(8*i)) & 0xff;
  current_data_pos += len;
}
int16_t SMENetPacket::get_short() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return 0;
  size_t len = 2;
  ERR_FAIL_COND_V_MSG(current_data_pos+len > enet_packet->dataLength, 0, "Wrong packet length!");
  int16_t res = 0;
  for (size_t i=0; i<len; ++i) {
    res <<= 8;
    res |= enet_packet->data[current_data_pos+(len-i-1)];
  }
  current_data_pos += len;
  return res;
}
void SMENetPacket::put_byte(uint8_t v) {
  if (enet_packet == nullptr) return;
  if (current_data_pos + 1 > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + 1);
  }
  enet_packet->data[current_data_pos] = v;
  current_data_pos += 1;
}
uint8_t SMENetPacket::get_byte() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return 0;
  ERR_FAIL_COND_V_MSG(current_data_pos+1 > enet_packet->dataLength, 0, "Wrong packet length!");
  uint8_t res = enet_packet->data[current_data_pos];
  current_data_pos += 1;
  return res;
}
void SMENetPacket::put_float(double v) {
  if (enet_packet == nullptr) return;
  size_t len = sizeof(double);
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  auto pv = (uint64_t*)&v;
  for (size_t i=0; i<len; ++i)
    enet_packet->data[current_data_pos+i] = ((*pv)>>(8*i)) & 0xff;
  current_data_pos += len;
}
double SMENetPacket::get_float() {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return 0;
  size_t len = sizeof(double);
  ERR_FAIL_COND_V_MSG(current_data_pos+len > enet_packet->dataLength, 0, "Wrong packet length!");
  uint64_t res = 0;
  for (size_t i=0; i<len; ++i) {
    res <<= 8;
    res |= enet_packet->data[current_data_pos+(len-i-1)];
  }
  current_data_pos += len;
  return *((double*)&res);
}
void SMENetPacket::put_var(const Variant &v, bool full_objects) {
  if (enet_packet == nullptr) return;
  int len;
  Error err = encode_variant(v, nullptr, len, full_objects);
  ERR_FAIL_COND_MSG(err != OK, "Can\'t encode the variant! (Object or RID can\'t be encoded)");
  if (current_data_pos + len > enet_packet->dataLength) {
    renet_packet_resize(enet_packet, current_data_pos + len);
  }
  encode_variant(v, enet_packet->data+current_data_pos, len, full_objects);
  current_data_pos += len;
}
Variant SMENetPacket::get_var(bool allow_objects) {
  if (enet_packet == nullptr || enet_packet->dataLength <= current_data_pos) return Variant();
  int len;
  Variant res;
  Error err = decode_variant(res, enet_packet->data+current_data_pos, enet_packet->dataLength-current_data_pos, &len, allow_objects);
  ERR_FAIL_COND_V_MSG(err != OK, res, "Invalid variant bytes!");
  current_data_pos += len;
  return res;
}

void SMENetPacket::put_data_as_var(const Variant &v, bool full_objects) {
  if (enet_packet == nullptr) return;
  int len;
  Error err = encode_variant(v, nullptr, len, full_objects);
  ERR_FAIL_COND_MSG(err != OK, "Can\'t encode the variant! (Object or RID can\'t be encoded)");
  renet_packet_resize(enet_packet, len);
  encode_variant(v, enet_packet->data, len, full_objects);
}
Variant SMENetPacket::get_data_var(bool allow_objects) {
  if (enet_packet == nullptr || enet_packet->dataLength == 0) return Variant();
  Variant res;
  Error err = decode_variant(res, enet_packet->data, enet_packet->dataLength, nullptr, allow_objects);
  ERR_FAIL_COND_V_MSG(err != OK, res, "Invalid variant bytes!");
  return res;
}

SMENetPacket::SMENetPacket() {
  enet_packet = nullptr;
  current_data_pos = 0;
}
SMENetPacket::~SMENetPacket() {
  if (enet_packet != nullptr) {
    renet_packet_destroy(enet_packet);
    enet_packet = nullptr;
  }
}

void SMENetPeer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("disconnect_from_host", "data"), &SMENetPeer::disconnect_from_host, 0);
  ClassDB::bind_method(D_METHOD("reset"), &SMENetPeer::reset);
  ClassDB::bind_method(D_METHOD("send_packet", "packet", "chanel_id"), &SMENetPeer::send_packet, 0);
  ClassDB::bind_method(D_METHOD("get_rtt"), &SMENetPeer::get_rtt);
  ClassDB::bind_method(D_METHOD("get_address"), &SMENetPeer::get_address);
  ClassDB::bind_method(D_METHOD("get_port"), &SMENetPeer::get_port);
  ClassDB::bind_method(D_METHOD("get_connect_id"), &SMENetPeer::get_connect_id);
  ClassDB::bind_method(D_METHOD("get_state"), &SMENetPeer::get_state);

  BIND_ENUM_CONSTANT(PEER_STATE_DISCONNECTED);
  BIND_ENUM_CONSTANT(PEER_STATE_CONNECTING);
  BIND_ENUM_CONSTANT(PEER_STATE_ACKNOWLEDGING_CONNECT);
  BIND_ENUM_CONSTANT(PEER_STATE_CONNECTION_PENDING);
  BIND_ENUM_CONSTANT(PEER_STATE_CONNECTION_SUCCEEDED);
  BIND_ENUM_CONSTANT(PEER_STATE_CONNECTED);
  BIND_ENUM_CONSTANT(PEER_STATE_DISCONNECT_LATER);
  BIND_ENUM_CONSTANT(PEER_STATE_DISCONNECTING);
  BIND_ENUM_CONSTANT(PEER_STATE_ACKNOWLEDGING_DISCONNECT);
  BIND_ENUM_CONSTANT(PEER_STATE_ZOMBIE);
}

void SMENetPeer::disconnect_from_host(uint32_t data) {
  ERR_FAIL_COND(enet_peer == nullptr);
  renet_peer_disconnect(enet_peer, data);
}
void SMENetPeer::reset() {
  ERR_FAIL_COND(enet_peer == nullptr);
  renet_peer_reset(enet_peer);
}

Error SMENetPeer::send_packet(Ref<SMENetPacket> packet, uint8_t chanel_id) {
  ERR_FAIL_COND_V(enet_peer == nullptr, Error::FAILED);
  ERR_FAIL_COND_V(packet.is_null(), Error::FAILED);
  auto res = renet_peer_send(enet_peer, chanel_id, packet->get_enet_object()); // hand it over to the enet
  packet->set_enet_object(nullptr);
  ERR_FAIL_COND_V(res < 0, Error::FAILED);
  return OK;
}

uint32_t SMENetPeer::get_rtt() {
  ERR_FAIL_COND_V(enet_peer == nullptr, 0);
  return enet_peer->roundTripTime;
}

String SMENetPeer::get_address() {
  ERR_FAIL_COND_V(enet_peer == nullptr, "");
  CharString s;
  s.resize(2048);
  renet_address_get_host_ip(&enet_peer->address, s.ptrw(), 2048);
  return String(s);
}
uint16_t SMENetPeer::get_port() {
  ERR_FAIL_COND_V(enet_peer == nullptr, 0);
  return enet_peer->address.port;
}
uint32_t SMENetPeer::get_connect_id() {
  ERR_FAIL_COND_V(enet_peer == nullptr, 0);
  return enet_peer->connectID;
}
int SMENetPeer::get_state() {
  ERR_FAIL_COND_V(enet_peer == nullptr, 0);
  return enet_peer->state;
}

SMENetPeer::SMENetPeer() {
  enet_peer = nullptr;
}
SMENetPeer::~SMENetPeer() {
  enet_peer = nullptr;
}

void SMENetHost::_bind_methods() {
  ClassDB::bind_method(D_METHOD("create_server", "address", "port", "peer_count", "channel_limits"), &SMENetHost::create_server, 32, 2);
  ClassDB::bind_method(D_METHOD("create_client", "peer_count", "channel_limits"), &SMENetHost::create_client, 32, 2);
  ClassDB::bind_method(D_METHOD("destroy"), &SMENetHost::destroy);
  ClassDB::bind_method(D_METHOD("host_service", "timeout"), &SMENetHost::host_service, 0);
  ClassDB::bind_method(D_METHOD("connect_to_host", "address", "port", "channel_count", "data"), &SMENetHost::connect_to_host, 2, 0);
  ClassDB::bind_method(D_METHOD("broadcast", "packet", "channel_id"), &SMENetHost::broadcast, 0);
  ClassDB::bind_method(D_METHOD("is_valid"), &SMENetHost::is_valid);

  ADD_SIGNAL(MethodInfo("connected", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_TYPE_STRING, "SMENetPeer")));
  ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_TYPE_STRING, "SMENetPeer")));
  ADD_SIGNAL(MethodInfo("received",
          PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_TYPE_STRING, "SMENetPeer"),
          PropertyInfo(Variant::OBJECT, "packet", PROPERTY_HINT_TYPE_STRING, "SMENetPacket"),
          PropertyInfo(Variant::INT, "channel_id")
          ));
}

Error SMENetHost::create_server(const String &address, uint8_t port, size_t peer_count, size_t channel_limits) {
  ERR_FAIL_COND_V_MSG(enet_host != nullptr, Error::FAILED, "You need to destroy the host in order to create a new one!");

  ENetAddress enet_address;
  if (address != "*") {
    int res;
    if (address.is_valid_ip_address()) {
      res = renet_address_set_host_ip(&enet_address, address.utf8());
    } else {
      res = renet_address_set_host(&enet_address, address.utf8());
    }
    ERR_FAIL_COND_V_MSG(res < 0, Error::FAILED, "The address is wrong!");
  } else {
    enet_address.host = ENET_HOST_ANY;
  }
  enet_address.port = port;
  
  enet_host = renet_host_create(&enet_address, peer_count, channel_limits, 0, 0);
  if (enet_host == nullptr) return Error::FAILED;

  return OK;
}
Error SMENetHost::create_client(size_t peer_count, size_t channel_limits) {
  ERR_FAIL_COND_V_MSG(enet_host != nullptr, Error::FAILED, "You need to destroy the host in order to create a new one!");
  enet_host = renet_host_create(nullptr, peer_count, channel_limits, 0, 0);
  return OK;
}
void SMENetHost::destroy() {
  ERR_FAIL_COND(enet_host == nullptr);
  renet_host_destroy(enet_host);
  enet_host = nullptr;
}

Error SMENetHost::host_service(uint32_t timeout) {
  ERR_FAIL_COND_V(enet_host == nullptr, Error::FAILED);
  ENetEvent event;
  int res = renet_host_service(enet_host, &event, timeout);
  ERR_FAIL_COND_V_MSG(res < 0, Error::FAILED, "Something is wrong when hosting the service!(The host need to created as a server or connecting to a server as a client!)");

  Ref<SMENetPeer> peer(memnew(SMENetPeer));
  peer->set_enet_object(event.peer);

  switch (event.type)
  {
  case ENET_EVENT_TYPE_CONNECT:
    {
      emit_signal("connected", peer);
      break;
    }
  case ENET_EVENT_TYPE_DISCONNECT:
    {
      emit_signal("disconnected", peer);
      break;
    }
  case ENET_EVENT_TYPE_RECEIVE:
    {
      Ref<SMENetPacket> packet(memnew(SMENetPacket));
      packet->set_enet_object(event.packet);
      emit_signal("received", peer, packet, event.channelID);
      break;
    }
  }

  return Error::OK;
}

Ref<SMENetPeer> SMENetHost::connect_to_host(const String &address, uint8_t port, size_t channel_count, uint32_t data) {
  ERR_FAIL_COND_V(enet_host == nullptr, nullptr);
  ENetAddress enet_address;
  int res;
  if (address.is_valid_ip_address()) {
    res = renet_address_set_host_ip(&enet_address, address.utf8());
  } else {
    res = renet_address_set_host(&enet_address, address.utf8());
  }
  ERR_FAIL_COND_V_MSG(res < 0, nullptr, "The address is wrong!");
  enet_address.port = port;

  ENetPeer *enet_peer = renet_host_connect(enet_host, &enet_address, channel_count, data);
  ERR_FAIL_COND_V_MSG(enet_peer == nullptr, nullptr, "Failed to connect to the host.");

  Ref<SMENetPeer> peer(memnew(SMENetPeer));
  peer->set_enet_object(enet_peer);
  return peer;
}

bool SMENetHost::is_valid() {
  return enet_host != nullptr;
}

void SMENetHost::broadcast(Ref<SMENetPacket> packet, uint8_t channel_id) {
  ERR_FAIL_COND(enet_host == nullptr);
  ERR_FAIL_COND(packet->get_enet_object() == nullptr);
  renet_host_broadcast(enet_host, channel_id, packet->get_enet_object());
  packet->set_enet_object(nullptr);
}

SMENetHost::SMENetHost() {
  enet_host = nullptr;
}
SMENetHost::~SMENetHost() {
  if (enet_host != nullptr) {
    renet_host_destroy(enet_host);
    enet_host = nullptr;
  }
}


