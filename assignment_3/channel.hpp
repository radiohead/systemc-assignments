#include "main.hpp"

class Channel: public sc_module, public initiator_if {
public:
  sc_in <bool> clock;
  sc_port <slave_if, 0> slave_port;
  sc_port <arbiter_if> arbiter_port;
  sc_event arbitration_complete;

  std::vector <int> initiator_ids;
  int current_initiator_id;

  sc_trace_file* trace_file;
  Packet current_packet;

  SC_HAS_PROCESS(Channel);
  Channel(sc_module_name);

  bool write(Packet packet);
  void dispatch(void);
};
