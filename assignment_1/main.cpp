#include <iostream>
#include <fstream>

#include "systemc"

using namespace std;
using namespace sc_dt;
using namespace sc_core;

class pkt
{
  sc_int<32> id;
  sc_int<64> data;

public:
  pkt()
  {
    this->id = -1;
    this->data = -1;
  }

  pkt(sc_int<32> id, sc_int<64> data)
  {
    this->id = id;
    this->data = data;
  }

  sc_int<32> get_id()
  {
    return this->id;
  }

  sc_int<64> get_data()
  {
    return this->data;
  }

  inline pkt& operator =(const pkt& right)
  {
    this->id = right.id;
    this->data = right.data;

    return *this;
  }

  inline bool operator ==(const pkt& right)
  {
    return this->id == right.id && this->data == right.data;
  }

  inline friend std::ostream& operator <<(std::ostream& output, const pkt& packet)
  {
    output << "pkt:<" << packet.id << "," << packet.data << ">";
    return output;
  }

  inline friend void sc_trace(sc_trace_file *trace_file, const pkt& packet, const std::string& packet_name)
  {
    sc_trace(trace_file, packet.id, packet_name + ".id");
    sc_trace(trace_file, packet.data, packet_name + ".data");
  }
};

SC_MODULE(Target)
{
  sc_in<pkt> input_channel;

  SC_CTOR(Target)
  {
    SC_METHOD(receive_process);
  }

  void receive_process()
  {
    next_trigger(50, SC_NS);
    pkt packet = input_channel.read();
    cout << "RCV" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;
  }
};

SC_MODULE(Initiator)
{
  sc_in<bool> input_clock;
  sc_out<pkt> output_channel;

  SC_CTOR(Initiator)
  {
    SC_CTHREAD(send_process, input_clock.pos());

    this->id_counter = 1;
  }

  void send_process()
  {
    if (!this->input_file->is_open()) {
      cerr << "Cannot read from file!" << endl;
      return;
    }

    string file_line;

    while(getline(*this->input_file, file_line))
    {
      pkt packet(this->id_counter, atoi(file_line.c_str()));
      this->id_counter++;

      output_channel.write(packet);
      cout << "SND" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;
      wait();
    }
  }

  void set_file_stream(ifstream& file)
  {
    this->input_file = &file;
  }

private:
  ifstream *input_file;
  sc_int<32> id_counter;
};

int sc_main(int argc, char *argv[])
{
  sc_trace_file* trace_file;
  trace_file = sc_create_vcd_trace_file("trace_file");
  ifstream input_file("input");

  const sc_time CLOCK_PERIOD(50, SC_NS);
  sc_clock clk("clk", CLOCK_PERIOD, 0.5);
  sc_signal<pkt> data_pipe;

  Initiator initiator("Initiator");
  initiator.input_clock(clk);
  initiator.output_channel(data_pipe);
  initiator.set_file_stream(input_file);

  Target target("target");
  target.input_channel(data_pipe);

  sc_trace(trace_file, data_pipe, "input_channel");
  sc_start(6, SC_US);

  input_file.close();
  sc_close_vcd_trace_file(trace_file);

  return 0;
}
