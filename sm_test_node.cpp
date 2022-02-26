#include "sm_test_node.h"

#include "core/variant.h"
#include "core/os/os.h"

#include <cstring>

void SMTestNode::_bind_methods() {
  ClassDB::bind_method(D_METHOD("server_send_msg", "msg"), &SMTestNode::server_send_msg);
  ClassDB::bind_method(D_METHOD("client_send_msg", "msg"), &SMTestNode::client_send_msg);
}

SMTestNode::SMTestNode() {

}

SMTestNode::~SMTestNode() {

}

String get_peer_address(ENetPeer *peer) {
  if (peer == nullptr) return "<Null>";
  Dictionary d;
  char host_name[256] = "<Error Host>";
  renet_address_get_host_ip(&peer->address, host_name, 256);
  d["host"] = host_name;
  d["port"] = peer->address.port;
  return String("{host}:{port}").format(d);
}

String _format(const String &f, Array &arr, const Variant &t) {
  arr.append(t);
  return f.format(arr);
}

template<typename... Args>
String _format(const String &f, Array &arr, const Variant &t, const Args &...args) {
  arr.append(t);
  return _format(f, arr, args...);
}

template<typename... Args>
String format(const String &f, const Args &...args) {
  return _format(f, Array(), args...);
}

void handle_host_service(ENetHost *host, const String &tag, bool is_server, SMTestNode &s) {
  ENetEvent event;

  int err = renet_host_service(host, &event, 0);
  if (err <= 0) {
    if (err < 0) {
      print_error(format("[{0}] Can\'t host service.", (Variant)tag));
    }
    return;
  }
  if (event.type == ENET_EVENT_TYPE_CONNECT) {
    print_line(format("[{0}] connected to host: {1}.", tag, get_peer_address(event.peer)));
    if (is_server) s.client_peer = event.peer;
    else s.server_peer = event.peer;
  } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
    print_line(format("[{0}] from {1} channel {2}({3}): \n{4}", tag, get_peer_address(event.peer), event.channelID, event.packet->dataLength, String::utf8((char*)event.packet->data, event.packet->dataLength)));
    renet_packet_destroy(event.packet);
  } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
    print_line(format("[{0}] {1} disconnected.", tag, get_peer_address(event.peer)));
  }
}

void SMTestNode::_notification(int n) {
  if (Engine::get_singleton()->is_editor_hint()) return;
  if (n == NOTIFICATION_ENTER_TREE) {
    print_line("SMTest Node Ready!");
    renet_address_set_host(&address, "localhost");
    address.port = 35578;
    server_peer = nullptr;
    client_peer = nullptr;

    server = renet_host_create(&address, 32, 2, 0, 0);
    ERR_FAIL_COND_MSG(server == nullptr, "Can\'t create server!");

    print_line(format("server listen on: {0}\n", address.port));
    set_physics_process_internal(true);

    client = renet_host_create(nullptr, 1, 2, 0, 0);
    ERR_FAIL_COND_MSG(client == nullptr, "Can\'t create client!");

    ENetPeer *peer = renet_host_connect(client, &address, 2, 0);
    ERR_FAIL_COND_MSG(peer == nullptr, "Can\'t create peer to connect server!");

  } else if (n == NOTIFICATION_INTERNAL_PHYSICS_PROCESS) {
    if (server == nullptr || client == nullptr) return;
    
    handle_host_service(server, "Server", true, *this);
    handle_host_service(client, "Client", false, *this);
  } else if (n == NOTIFICATION_EXIT_TREE) {
    if (server) {
      renet_host_destroy(server);
      server = nullptr;
    }
    if (client) {
      renet_host_destroy(client);
      client = nullptr;
    }
  }
}

void send_msg(ENetPeer *peer, const String &msg) {
  ERR_FAIL_COND(peer == nullptr);
  ENetPacket *packet = renet_packet_create(msg.utf8(), msg.utf8().length(), ENET_PACKET_FLAG_RELIABLE);
  renet_peer_send(peer, 0, packet);
}

void SMTestNode::server_send_msg(const String &msg) {
  send_msg(client_peer, msg);
}
void SMTestNode::client_send_msg(const String &msg) {
  send_msg(server_peer, msg);
}