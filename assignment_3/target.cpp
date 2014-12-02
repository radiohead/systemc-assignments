#include "target.hpp"

Target::Target(sc_module_name module_name, int id) : sc_module(module_name), id(id) { }

bool Target::write(Packet packet) {
  cout << "Received packet from [" << packet.initiator_id << "]" << endl;

  return true;
}
