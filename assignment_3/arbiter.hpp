#include "main.hpp"

class Arbiter : public sc_module, public arbiter_if {
public:
  SC_HAS_PROCESS(Arbiter);
  Arbiter(sc_module_name);

  int arbitrate(std::vector <int>);
};
