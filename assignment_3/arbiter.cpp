#include "arbiter.hpp"

Arbiter::Arbiter(sc_module_name module_name) : sc_module(module_name) { }

int Arbiter::arbitrate(std::vector <int> initiator_ids) {
  int random = rand() % initiator_ids.size();
  return initiator_ids[random];
}
