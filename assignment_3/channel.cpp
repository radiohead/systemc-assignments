#include "channel.hpp"

Channel::Channel(sc_module_name module_name) : sc_module(module_name) {
  SC_THREAD(dispatch);
  sensitive << this->clock.pos();

  this->current_initiator_id = -1;
  this->trace_file = sc_create_vcd_trace_file("trace_file");

  sc_trace(this->trace_file, this->current_packet, "CHANNEL_PACKET");
}

bool Channel::write(Packet packet) {
  this->initiator_ids.push_back(packet.initiator_id);

  wait(arbitration_complete);

  if (packet.initiator_id == this->current_initiator_id) {
    this->current_packet = packet;
    this->slave_port[packet.target_id]->write(packet);

    return true;
  }
  else {
    return false;
  }
}

void Channel::dispatch() {
  while(true) {
    wait();

    if (this->initiator_ids.size() > 0) {
      this->current_initiator_id = this->arbiter_port->arbitrate(this->initiator_ids);
      arbitration_complete.notify();
      this->initiator_ids.clear();
    }
  }
}
