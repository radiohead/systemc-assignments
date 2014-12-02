#include "main.hpp"

class Target : public sc_module, public slave_if {
public:
  int id;

  SC_HAS_PROCESS(Target);
  Target(sc_module_name, int = 0);
  bool write(Packet packet);
};
