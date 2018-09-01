//#include <bm/SimpleSumeSwitch.h>
#include <bm/bm_runtime/bm_runtime.h>
//#include <bm/bm_sim/target_parser.h>

//#include <sstream>
//#include <string>
//#include <vector>

#include "simple_sume_switch.h"

/* Switch instance */

static SimpleSumeSwitch *simple_sume_switch;

int
main(int argc, char* argv[]) {
  simple_sume_switch = new SimpleSumeSwitch();
  int status = simple_sume_switch->init_from_command_line_options(argc, argv);
  if (status != 0) std::exit(status);

  // TODO: thrift
  int thrift_port = simple_sume_switch->get_runtime_port();
  bm_runtime::start_server(simple_sume_switch, thrift_port);

  simple_sume_switch->start_and_return();

  while (true) std::this_thread::sleep_for(std::chrono::seconds(100));

  return 0;
}
