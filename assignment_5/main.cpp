// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// Initiator module generating generic payload transactions
struct Initiator: sc_module
{
  unsigned int number_of_targets;

  // TLM-2 socket
  tlm_utils::simple_initiator_socket<Initiator> socket;

  //Construct and name socket
  SC_CTOR(Initiator): socket("socket") {
    this->number_of_targets = 0;
    SC_THREAD(thread_process);
  }

  void thread_process()
  {
    // TLM-2 generic payload transaction
    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
    sc_time delay = sc_time(10, SC_NS);

    // Generate a random sequence of reads and writes
    for (int i = 0; i < 128; i += 4)
    {
      int data;
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(rand() % 2);
      if (cmd == tlm::TLM_WRITE_COMMAND) data = 0xFF000000 | i;

      trans->set_command( cmd );
      trans->set_address( rand() % this->number_of_targets );
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
      trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
      trans->set_dmi_allowed( false ); // Mandatory initial value
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

      // Blocking transport call
      socket->b_transport(*trans, delay);

      // Initiator obliged to check response status
      if ( trans->is_response_error() )
      {
        // *********************
        // Print response string
        // *********************
        char txt[100];
        sprintf(txt, "Error from b_transport, response status = %s",
                     trans->get_response_string().c_str());
        SC_REPORT_ERROR("TLM-2", txt);

      }

      cout << sc_time_stamp() << " @ trans = { " << (cmd ? 'W' : 'R') << ", " << hex << i
           << " } , data = " << hex << data << " at time " << sc_time_stamp()
           << " delay = " << delay << endl;
    }

    // ********************************************************
    // Use debug transaction interface to dump memory contents,
    // reusing same transaction object
    // ********************************************************
    trans->set_command( tlm::TLM_READ_COMMAND );
    trans->set_address(0);
    trans->set_read();
    trans->set_data_length(128);

    unsigned char* data = new unsigned char[128];
    trans->set_data_ptr(data);

    unsigned int n_bytes = socket->transport_dbg( *trans );

    for (unsigned int i = 0; i < n_bytes; i += 4)
    {
      cout << "mem[" << i << "] = "
           << *(reinterpret_cast<unsigned int*>( &data[i] )) << endl;
    }
  }
};


// Target module representing a simple memory
struct Target: sc_module
{
  // TLM-2 socket
  tlm_utils::simple_target_socket<Target> socket;

  enum { SIZE = 256 };

  SC_CTOR(Target): socket("socket")
  {
    // Register callbacks for incoming interface method calls
    socket.register_b_transport(this, &Target::b_transport);

    // Initialize memory with random data
    for (int i = 0; i < SIZE; i++)
      mem[i] = 0xAA000000 | (rand() % 256);
  }

  // TLM-2 blocking transport method
  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    // Obliged to check address range and check for unsupported features,
    //   i.e. byte enables, streaming, and bursts
    // Can ignore extensions

    // *********************************************
    // Generate the appropriate error response
    // *********************************************
    if (adr >= sc_dt::uint64(SIZE)) {
      trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
      return;
    }
    if (byt != 0) {
      trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
      return;
    }
    if (len > 4 || wid < len) {
      trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
      return;
    }

    // Obliged to implement read and write commands
    if ( cmd == tlm::TLM_READ_COMMAND )
      memcpy(ptr, &mem[adr], len);
    else if ( cmd == tlm::TLM_WRITE_COMMAND )
      memcpy(&mem[adr], ptr, len);

    // Illustrates that b_transport may block

    // Obliged to set response status to indicate successful completion
    trans.set_response_status( tlm::TLM_OK_RESPONSE );
  }

  // *********************************************
  // TLM-2 debug transport method
  // *********************************************
  virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address() / 4;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();

    // Calculate the number of bytes to be actually copied
    unsigned int num_bytes = (len < (SIZE - adr) * 4) ? len : (SIZE - adr) * 4;

    if ( cmd == tlm::TLM_READ_COMMAND )
      memcpy(ptr, &mem[adr], num_bytes);
    else if ( cmd == tlm::TLM_WRITE_COMMAND )
      memcpy(&mem[adr], ptr, num_bytes);

    return num_bytes;
  }
  int mem[SIZE];
};

template <unsigned int N_TARGETS, unsigned int N_INITIATORS>
struct Bus: sc_module
{
  tlm_utils::simple_target_socket_tagged<Bus>* target_sockets[N_INITIATORS];
  tlm_utils::simple_initiator_socket_tagged<Bus>* initiator_sockets[N_TARGETS];

  SC_CTOR(Bus)
  {
    for (unsigned int i = 0; i < N_INITIATORS; ++i)
    {
      char identifier[64];
      sprintf(identifier, "target_socket_%d", i);

      target_sockets[i] = new tlm_utils::simple_target_socket_tagged<Bus>(identifier);
      target_sockets[i]->register_b_transport(this, &Bus::b_transport, i);
    }

    for (unsigned int i = 0; i < N_TARGETS; ++i)
    {
      char identifier[64];
      sprintf(identifier, "initiator_socket_%d", i);
      initiator_sockets[i] = new tlm_utils::simple_initiator_socket_tagged<Bus>(identifier);
    }
  }

  virtual void b_transport(int socket_id, tlm::tlm_generic_payload& payload, sc_time& delay)
  {
    if (payload.get_address() <= (N_TARGETS - 1)) {
      (*initiator_sockets[payload.get_address()])->b_transport(payload, delay);
    }
    else {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
  }
};

SC_MODULE(Top)
{
  // Modules
  Initiator *initiator_1 = NULL;
  Initiator *initiator_2 = NULL;

  Target *target_1 = NULL;
  Target *target_2 = NULL;

  Bus<2, 2> *bus = NULL;

  SC_CTOR(Top) {
    // Instantiate components
    initiator_1 = new Initiator("initiator_1");
    initiator_2 = new Initiator("initiator_2");

    initiator_1->number_of_targets = 2;
    initiator_2->number_of_targets = 2;

    target_1 = new Target("target_1");
    target_2 = new Target("target_2");

    bus = new Bus<2, 2>("bus");

    initiator_1->socket.bind(*(bus->target_sockets[0]));
    initiator_2->socket.bind(*(bus->target_sockets[1]));

    (*bus->initiator_sockets[0]).bind(target_1->socket);
    (*bus->initiator_sockets[1]).bind(target_2->socket);
  }
};

int sc_main(int argc, char* argv[])
{
  Top top("top");
  sc_start();
  return 0;
}
