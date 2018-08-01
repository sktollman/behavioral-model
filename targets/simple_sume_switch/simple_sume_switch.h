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
  using mirror_id_t = int;

  using TransmitFn = std::function<void(port_t, packet_id_t,
                                         const char *, int)>;

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

  void set_transmit_fn(TransmitFn fn) {
    my_transmit_fn = std::move(fn);
  }

  // TODO: what is mirroring???
  int mirroring_mapping_add(mirror_id_t mirror_id, port_t egress_port) {
    mirroring_map[mirror_id] = egress_port;
    return 0;
  }

  int mirroring_mapping_delete(mirror_id_t mirror_id) {
    return mirroring_map.erase(mirror_id);
  }

  bool mirroring_mapping_get(mirror_id_t mirror_id, port_t *port) const {
    return get_mirroring_mapping(mirror_id, port);
  }

 private:
  static packet_id_t packet_id;
  std::shared_ptr<McSimplePreLAG> pre; // TODO: is this necessary?

  TransmitFn my_transmit_fn;
  std::unordered_map<mirror_id_t, port_t> mirroring_map;

  bool get_mirroring_mapping(mirror_id_t mirror_id, port_t *port) const {
    const auto it = mirroring_map.find(mirror_id);
    if (it != mirroring_map.end()) {
      *port = it->second;
      return true;
    }
    return false;
  }
};

#endif  // SIMPLE_SUME_SWITCH_H_
