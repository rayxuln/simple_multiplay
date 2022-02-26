#include "register_types.h"

#include "enet/enet.h"

#include "core/variant.h"
#include "core/os/os.h"

#include "sm_test_node.h"
#include "sm_enet_classes.h"

void register_simple_multiplay_types() {
  ERR_FAIL_COND_MSG(renet_initialize() != 0, "An error occured while initializing Enet.");

  ClassDB::register_class<SMTestNode>();
  ClassDB::register_class<SMENetPeer>();
  ClassDB::register_class<SMENetPacket>();
  ClassDB::register_class<SMENetHost>();
}
void unregister_simple_multiplay_types() {
  renet_deinitialize();
}