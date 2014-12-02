#ifndef MAIN_HPP
  #define MAIN_HPP

  #include "systemc.h"

  class Packet {
  public:
    int data;
    int initiator_id;
    int target_id;

    Packet() {
      this->initiator_id = -1;
      this->target_id = -1;
      this->data = -1;
    }

    Packet(int initiator_id, int target_id, int data) {
      this->initiator_id = initiator_id;
      this->target_id = target_id;
      this->data = data;
    }

    inline Packet& operator =(const Packet& right) {
      this->initiator_id = right.initiator_id;
      this->target_id = right.target_id;
      this->data = right.data;

      return *this;
    }

    inline bool operator ==(const Packet& right) {
      return this->initiator_id == right.initiator_id &&
             this->target_id == right.target_id &&
             this->data == right.data;
    }

    inline friend std::ostream& operator <<(std::ostream& output, const Packet& packet) {
      output << "Packet:<" << packet.initiator_id << "," << packet.target_id << "," << packet.data << ">";
      return output;
    }

    inline friend void sc_trace(sc_trace_file *trace_file, const Packet& packet, const std::string& packet_name) {
      sc_trace(trace_file, packet.initiator_id, packet_name + ".id");
      sc_trace(trace_file, packet.target_id, packet_name + ".id");
      sc_trace(trace_file, packet.data, packet_name + ".data");
    }
  };

  class initiator_if: virtual public sc_interface {
  public:
    virtual bool write(Packet packet) = 0;
  };

  class slave_if: virtual public sc_interface {
  public:
    virtual bool write(Packet packet) = 0;
  };

  class arbiter_if: virtual public sc_interface {
  public:
    virtual int arbitrate(std::vector<int> ids) = 0;
  };
#endif
