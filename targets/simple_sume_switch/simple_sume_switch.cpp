#include <bm/bm_sim/parser.h>
#include <bm/bm_sim/tables.h>
#include <bm/bm_sim/logger.h>

#include <bm/bm_runtime/bm_runtime.h>

#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>

#include "simple_sume_switch.h"

packet_id_t SimpleSumeSwitch::packet_id = 0;

SimpleSumeSwitch::SimpleSumeSwitch(bool enable_swap)
  : Switch(enable_swap),
  my_transmit_fn([this](port_t port_num, packet_id_t pkt_id,
                        const char *buffer, int len) {
      _BM_UNUSED(pkt_id);
      this->transmit_fn(port_num, buffer, len);
  }) {

  add_required_field("sume_metadata_t", "dma_q_size");
  add_required_field("sume_metadata_t", "nf3_q_size");
  add_required_field("sume_metadata_t", "nf2_q_size");
  add_required_field("sume_metadata_t", "nf1_q_size");
  add_required_field("sume_metadata_t", "nf0_q_size");
  add_required_field("sume_metadata_t", "send_dig_to_cpu");
  add_required_field("sume_metadata_t", "dst_port");
  add_required_field("sume_metadata_t", "src_port");
  add_required_field("sume_metadata_t", "pkt_len");

  force_arith_header("sume_metadata_t");
  force_arith_header("user_metadata_t");
  force_arith_header("digest_data_t");
}

SimpleSumeSwitch::~SimpleSumeSwitch() {}

#define PACKET_LENGTH_REG_IDX 0
#define DROP_PORT 0

int
SimpleSumeSwitch::receive_(port_t port_num, const char *buffer, int len) {
  // we limit the packet buffer to original size + 512 bytes, which means we
  // cannot add more than 512 bytes of header data to the packet, which should
  // be more than enough
  std::cout << "RECIEVED A PACKET" << std::endl;

// https://github.com/p4lang/behavioral-model/blob/b826576c0d85c23ff9d59571fb70e44dc8475905/src/bm_sim/switch.cpp#L509
  auto packet = new_packet_ptr(port_num, packet_id++, len, // TODO: can I get rid of packet id?
                               bm::PacketBuffer(len + 512, buffer, len));

  std::cout << "new ptr" << std::endl;

  BMELOG(packet_in, *packet); // TODO: what is packet_in?

  PHV *phv = packet->get_phv();
  std::cout << "got phv" << std::endl;
  // many current P4 programs assume this
  // it is also part of the original P4 spec
  phv->reset_metadata();

  std::cout << "simple_sume_switch: meta reset" << std::endl;

  // TODO: crash is here!!

  phv->get_field("sume_metadata_t.src_port").set(port_num);

  std::cout << "simple_sume_switch: set port" << std::endl;
  // using packet register 0 to store length, this register will be updated for
  // each add_header / remove_header primitive call
  // TODO: is this still relevant??
  packet->set_register(PACKET_LENGTH_REG_IDX, len);
  phv->get_field("sume_metadata_t.pkt_len").set(len);

  std::cout << "simple_sume_switch: set len" << std::endl;

  Parser *parser = this->get_parser("parser");
  Pipeline *pipeline = this->get_pipeline("TopPipe");
  Deparser *deparser = this->get_deparser("deparser");

  port_t ingress_port = packet->get_ingress_port();
  (void) ingress_port;
  BMLOG_DEBUG_PKT(*packet, "Processing packet received on port {}",
                  ingress_port);

  std::cout << "simple_sume_switch: processing" << std::endl;

  auto p = packet.get();

  std::cout << "simple_sume_switch: got packet" << std::endl;

  parser->parse(p);

  std::cout << "simple_sume_switch: applying" << std::endl;

  pipeline->apply(packet.get());

  std::cout << "simple_sume_switch: reset exit" << std::endl;

  packet->reset_exit(); // TODO: do I need this?

  std::cout << "simple_sume_switch: setting len" << std::endl;

  phv->get_field("sume_metadata_t.pkt_len").set(packet->get_data_size());

  std::cout << "simple_sume_switch: setting dport" << std::endl;

  port_t egress_port = phv->get_field("sume_metadata_t.dst_port").get_int();
  packet->set_egress_port(egress_port);

  if (DROP_PORT == egress_port) { // drop packet
    std::cout << "dropping" << std::endl;
    BMLOG_DEBUG_PKT(*packet, "Dropping packet");
    return 0;
  }

  std::cout << "simple_sume_switch: deparsing" << std::endl;

  deparser->deparse(packet.get());

  // TODO: traffic management
  // TODO: prepend digest data if send to cpu
  /* send_dig_to_cpu - set the least significant bit of this field to send the digest_data to the CPU. If this bit is set and a packet is to be forwarded to the CPU then the digest_data will be prepended to the packet. Otherwise, only the digest_data will be sent over DMA. */

  BMELOG(packet_out, *packet);
  BMLOG_DEBUG_PKT(*packet, "Transmitting packet of size {} out of port {}",
                  packet->get_data_size(), packet->get_egress_port());
  std::cout << "transmitting" << std::endl;
  my_transmit_fn(packet->get_egress_port(), packet->get_packet_id(),
                 packet->data(), packet->get_data_size());

  std::cout << "done receving" << std::endl;

  return 0;
}

void
SimpleSumeSwitch::start_and_return_() {
  std::cout << "STARTED!" << std::endl;
}

void
SimpleSumeSwitch::reset_target_state_() {
  bm::Logger::get()->debug("Resetting simple_sume_switch target-specific state");
  get_component<McSimplePreLAG>()->reset_state();
}
