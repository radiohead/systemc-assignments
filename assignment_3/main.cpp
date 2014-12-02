#include "main.hpp"

#include "target.hpp"
#include "arbiter.hpp"
#include "channel.hpp"
#include "initiator.hpp"

using namespace std;
using namespace sc_dt;
using namespace sc_core;

int sc_main(int argc, char *argv[])
{
  const sc_time CLOCK_PERIOD(50, SC_NS);
  sc_clock clk("clk", CLOCK_PERIOD, 0.5);

  Channel *bus_channel = new Channel("bus_channel");
  Arbiter *arbiter = new Arbiter("arbiter");

  Initiator *initiator_0 = new Initiator("initiator_0", "input_0", 0, 0);
  Initiator *initiator_1 = new Initiator("initiator_1", "input_1", 1, 1);
  Initiator *initiator_2 = new Initiator("initiator_2", "input_2", 2, 0);
  Initiator *initiator_3 = new Initiator("initiator_3", "input_3", 3, 1);

  Target *target_0 = new Target("target_0", 0);
  Target *target_1 = new Target("target_1", 1);

  bus_channel->clock(clk);
  bus_channel->arbiter_port(*arbiter);
  bus_channel->slave_port(*target_0);
  bus_channel->slave_port(*target_1);

  initiator_0->clock(clk);
  initiator_1->clock(clk);
  initiator_2->clock(clk);
  initiator_3->clock(clk);

  initiator_0->channel_port(*bus_channel);
  initiator_1->channel_port(*bus_channel);
  initiator_2->channel_port(*bus_channel);
  initiator_3->channel_port(*bus_channel);

  sc_start(6, SC_US);

  return 0;
}
