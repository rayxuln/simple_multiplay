#ifndef _H_SM_TEST_NODE_
#define _H_SM_TEST_NODE_

#include "scene/main/node.h"

#include "enet/enet.h"

class SMTestNode : public Node {
  GDCLASS(SMTestNode, Node);

  ENetAddress address;
  ENetHost *server;
  ENetHost *client;
protected:
  static void _bind_methods();
  void _notification(int);

public:
  ENetPeer *server_peer, *client_peer;

  void server_send_msg(const String &);
  void client_send_msg(const String &);

  SMTestNode();
  virtual ~SMTestNode();
};



#endif
