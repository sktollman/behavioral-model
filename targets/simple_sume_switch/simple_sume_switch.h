#ifndef SIMPLE_SUME_SWITCH_H_
#define SIMPLE_SUME_SWITCH_H_

#include <bm/bm_sim/queue.h>
#include <bm/bm_sim/queueing.h>
#include <bm/bm_sim/packet.h>
#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_sim/simple_pre_lag.h>

#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <functional>

using ts_res = std::chrono::microseconds;
using std::chrono::duration_cast;
using ticks = std::chrono::nanoseconds;

using bm::Switch;
using bm::Queue;
using bm::Packet;
using bm::PHV;
using bm::Parser;
using bm::Deparser;
using bm::Pipeline;
using bm::McSimplePreLAG;
using bm::Field;
using bm::FieldList;
using bm::packet_id_t;
using bm::p4object_id_t;
// TODO: remove unnecessary imports

class SimpleSumeSwitch : public Switch {
 public:
  // by default, swapping is off
  explicit SimpleSumeSwitch(bool enable_swap = false);

  ~SimpleSumeSwitch();

  int receive_(port_t port_num, const char *buffer, int len) override;

  void start_and_return_() override;

  void reset_target_state_() override;

  // returns the packet id of most recently received packet. Not thread-safe.
  static packet_id_t get_packet_id() {
    return (packet_id-1);
  }

 private:
  static packet_id_t packet_id;
  std::shared_ptr<McSimplePreLAG> pre; // TODO: is this necessary?
};

#endif  // SIMPLE_SUME_SWITCH_H_
