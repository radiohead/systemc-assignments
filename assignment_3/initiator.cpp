#include "initiator.hpp"
#include <fstream>

using namespace std;

Initiator::Initiator(sc_module_name module_name, string filename, int id, int target_id) : sc_module(module_name) {
  this->id = id;
  this->filename = filename;
  this->target_id = target_id;

  SC_THREAD(send_packet);
  sensitive << this->clock.pos();
}

void Initiator::send_packet() {
  ifstream input_file(this->filename.c_str());
  string file_line;

  if (!input_file.is_open()) {
    cerr << "Cannot open file [" << this->filename << "]" << endl;
  }

  while(getline(input_file, file_line)) {
    wait();

    Packet *packet = new Packet(this->id, this->target_id, atoi(file_line.c_str()));

    if (this->channel_port->write(*packet)) {
      cout << "PACKET [" << packet->data << "] " << "sent successfully" << endl;
    }
    else {
      cout << "PACKET [" << packet->data << "] " << "not sent" << endl;
    }

    delete packet;
  }

  input_file.close();
}
