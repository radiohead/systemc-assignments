#include <iostream>
#include <fstream>

#include "systemc"

using namespace std;
using namespace sc_dt;
using namespace sc_core;

// Define packet format to send between the initiator and target
class pkt
{
  // Use SystemC integer types
  // since they are platform-agnostic
  sc_int<32> id;
  sc_int<64> data;

public:
  // Default constructor
  // Necessary for the packet
  // to be used in signal pipes
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

  // Define accessors
  // Since the attributes are private
  sc_int<32> get_id()
  {
    return this->id;
  }

  sc_int<64> get_data()
  {
    return this->data;
  }

  // The following functions are necessary
  // To be able to send/receive instances of pkt
  // In signals and also for sc_trace
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

// Define module target
SC_MODULE(Target)
{
  // The input channel
  // That accepts pkt objects
  sc_port<sc_signal_in_if<pkt>, 0> input_channel;

  SC_CTOR(Target)
  {
    SC_METHOD(receive_process);
  }

  void receive_process()
  {
    // Wait for the clock
    // to trigger the initiator
    next_trigger(50, SC_NS);

    for (int i = 0; i != input_channel.size(); ++i)
    {
      // Read and log the packet
      pkt packet = input_channel[i]->read();
      cout << "RCV" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;
    }
  }
};

// Initiator module
SC_MODULE(Initiator)
{
  // Clock signal attribute
  sc_in<bool> input_clock;
  // Output channel able to send
  // pkt objects
  sc_out<pkt> output_channel;
  sc_port<sc_mutex_if> arbiter_channel;

  SC_CTOR(Initiator)
  {
    // Define the process sensitive to positive clock edge
    SC_CTHREAD(send_process, input_clock.pos());
  }

  void send_process()
  {
    // The file isn't open
    // so there's no point in trying to read from it
    if (!this->input_file->is_open()) {
      cerr << "Cannot read from file!" << endl;
      return;
    }

    string file_line;

    // Read lines from file
    while(getline(*this->input_file, file_line)) {
      if (arbiter_channel->trylock() != -1) {
        // Instantiate packet
        pkt packet(this->id, atoi(file_line.c_str()));

        // Send packet
        output_channel.write(packet);
        // Log packet
        cout << "SND" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;

        arbiter_channel->unlock();
      }

      // Wait for next clock
      wait();
    }
  }

  void set_file_stream(ifstream& file)
  {
    this->input_file = &file;
  }

  void set_id(sc_int<32> id)
  {
    this->id = id;
  }

private:
  ifstream *input_file;
  sc_int<32> id_counter;
  sc_int<32> id;
};

int sc_main(int argc, char *argv[])
{
  // Trace file
  sc_trace_file* trace_file;
  trace_file = sc_create_vcd_trace_file("trace_file");

  // Input files
  ifstream input_file_0("input_1");
  ifstream input_file_1("input_2");

  // Instantiate clock and communication
  const sc_time CLOCK_PERIOD(50, SC_NS);
  sc_clock clk("clk", CLOCK_PERIOD, 0.5);
  sc_mutex arbiter_mutex;

  Target *target = new Target("Target");
  Initiator *initiator_0 = new Initiator("Initiator_0");
  Initiator *initiator_1 = new Initiator("Initiator_1");

  sc_signal<pkt> data_pipe_0;
  sc_signal<pkt> data_pipe_1;

  sc_int<32> id_0(0);
  sc_int<32> id_1(1);

  initiator_0->input_clock(clk);
  initiator_0->arbiter_channel(arbiter_mutex);
  initiator_0->output_channel(data_pipe_0);
  initiator_0->set_file_stream(input_file_0);
  initiator_0->set_id(id_0);

  initiator_1->input_clock(clk);
  initiator_1->arbiter_channel(arbiter_mutex);
  initiator_1->output_channel(data_pipe_1);
  initiator_1->set_file_stream(input_file_1);
  initiator_0->set_id(id_1);

  target->input_channel(data_pipe_0);
  target->input_channel(data_pipe_1);

  sc_trace(trace_file, data_pipe_0, "input_channel_0");
  sc_trace(trace_file, data_pipe_1, "input_channel_1");

  // Start simulation
  sc_start(6, SC_US);

  // Don't forget to clean up!
  input_file_0.close();
  input_file_1.close();
  sc_close_vcd_trace_file(trace_file);

  return 0;
}
