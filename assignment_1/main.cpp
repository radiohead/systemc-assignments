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
  sc_in<pkt> input_channel;

  SC_CTOR(Target)
  {
    SC_METHOD(receive_process);
    sensitive << input_channel;
  }

  void receive_process()
  {
    // Read and log the packet
    pkt packet = input_channel.read();
    cout << "RCV" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;
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

  SC_CTOR(Initiator)
  {
    // Define the process sensitive to positive clock edge
    SC_CTHREAD(send_process, input_clock.pos());
    // Reset id counter to 1
    this->id_counter = 1;
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
    while(getline(*this->input_file, file_line))
    {
      // Instantiate packet
      pkt packet(this->id_counter, atoi(file_line.c_str()));
      // Increment counter
      this->id_counter++;

      // Send packet
      output_channel.write(packet);
      // Log packet
      cout << "SND" << endl << "ID: " << packet.get_id() << " DATA: " << packet.get_data() << endl;
      // Wait for next clock
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
  // Open trace and input files
  sc_trace_file* trace_file;
  trace_file = sc_create_vcd_trace_file("trace_file");
  ifstream input_file("input");

  // Instantiate clock and communication
  const sc_time CLOCK_PERIOD(50, SC_NS);
  sc_clock clk("clk", CLOCK_PERIOD, 0.5);
  sc_signal<pkt> data_pipe;

  // Instantiate initiator
  // bind ports and signal
  // pass file stream handler
  Initiator initiator("Initiator");
  initiator.input_clock(clk);
  initiator.output_channel(data_pipe);
  initiator.set_file_stream(input_file);

  // Instantiate target
  // bin port
  Target target("target");
  target.input_channel(data_pipe);

  // Initiate channel monitoring
  sc_trace(trace_file, data_pipe, "input_channel");

  // Start simulation
  sc_start(6, SC_US);

  // Don't forget to clean up!
  input_file.close();
  sc_close_vcd_trace_file(trace_file);

  // Exit normally
  return 0;
}
