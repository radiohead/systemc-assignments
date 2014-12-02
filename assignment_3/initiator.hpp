#include "main.hpp"

class Initiator : public sc_module {
  int id;
  int target_id;
  std::string filename;

public:
  sc_in <bool> clock;
  sc_port <initiator_if> channel_port;

  SC_HAS_PROCESS(Initiator);

  Initiator(sc_module_name, std::string, int, int);
  void send_packet();
};
