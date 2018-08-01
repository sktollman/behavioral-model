/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Antonin Bas (antonin@barefootnetworks.com)
 *
 */

#include <gtest/gtest.h>

#include <bm/bm_apps/packet_pipe.h>

#include <boost/filesystem.hpp>

#include <string>
#include <memory>
#include <vector>

#include "simple_sume_switch.h"

#include "utils.h"

namespace fs = boost::filesystem;

using bm::MatchErrorCode;
using bm::ActionData;
using bm::MatchKeyParam;
using bm::entry_handle_t;

namespace {

void
packet_handler(int port_num, const char *buffer, int len, void *cookie) {
  static_cast<SimpleSumeSwitch *>(cookie)->receive(port_num, buffer, len);
}

}  // namespace

class SimpleSwitch_PacketRedirectP4 : public ::testing::Test {
 protected:
  static constexpr size_t kMaxBufSize = 512;

  static constexpr bm::device_id_t device_id{0};

  SimpleSwitch_PacketRedirectP4()
      : packet_inject(packet_in_addr){}//,
        // events(event_logger_addr) { }

  // Per-test-case set-up.
  // We make the switch a shared resource for all tests. This is mainly because
  // the simple_switch target detaches threads
  static void SetUpTestCase() {
    // bm::Logger::set_logger_console();
// #ifdef BMELOG_ON
//     auto event_transport = bm::TransportIface::make_nanomsg(event_logger_addr);
//     event_transport->open();
//     bm::EventLogger::init(std::move(event_transport));
// #endif
    // std::cout<< "setting up!" << std::endl;
    test_switch = new SimpleSumeSwitch(8);  // 8 ports
    // std::cout<< "created" << std::endl;

    // load JSON
    fs::path json_path = fs::path(testdata_dir) / fs::path(test_json);
    test_switch->init_objects(json_path.string());
    // std::cout<< "json loaded" << std::endl;

    // packet in - packet out
    test_switch->set_dev_mgr_packet_in(device_id, packet_in_addr, nullptr);
    // std::cout<< "set packet in" << std::endl;
    test_switch->Switch::start();  // there is a start member in SimpleSwitch
    // std::cout<< "started" << std::endl;
    test_switch->set_packet_handler(packet_handler, // TODO: do I actually need this??
                                    static_cast<void *>(test_switch));
    test_switch->start_and_return();
    // std::cout<< "started and returned" << std::endl;
  }

  // Per-test-case tear-down.
  static void TearDownTestCase() {
    delete test_switch;
  }

//TODO: I think I need a packet handler...
  virtual void SetUp() {
    packet_inject.start();
    auto cb = std::bind(&PacketInReceiver::receive, &receiver,
                        std::placeholders::_1, std::placeholders::_2,
                        std::placeholders::_3, std::placeholders::_4);
    packet_inject.set_packet_receiver(cb, nullptr);
  }

  virtual void TearDown() {
    // kind of experimental, so reserved for testing
    test_switch->reset_state();
  }

  // bool check_event_table_hit(const NNEventListener::NNEvent &event,
  //                            const std::string &name) {
  //   return (event.type == NNEventListener::TABLE_HIT) &&
  //       (event.id == test_switch->get_table_id(name));
  // }

  // bool check_event_table_miss(const NNEventListener::NNEvent &event,
  //                             const std::string &name) {
  //   return (event.type == NNEventListener::TABLE_MISS) &&
  //       (event.id == test_switch->get_table_id(name));
  // }

  // bool check_event_action_execute(const NNEventListener::NNEvent &event,
  //                                 const std::string &t_name,
  //                                 const std::string &a_name) {
  //   return (event.type == NNEventListener::ACTION_EXECUTE) &&
  //       (event.id == test_switch->get_action_id(t_name, a_name));
  // }

 protected:
  // static const std::string event_logger_addr;
  static const std::string packet_in_addr;
  static SimpleSumeSwitch *test_switch;
  bm_apps::PacketInject packet_inject;
  PacketInReceiver receiver{};
  // NNEventListener events;

 private:
  static const std::string testdata_dir;
  static const std::string test_json;
};

// In theory, I could be using an 'inproc' transport here. However, I observe a
// high number of packet drops when switching to 'inproc', which is obviosuly
// causing the tests to fail. PUB/SUB is not a reliable protocol and therefore
// packet drops are to be expected when the phblisher is faster than the
// consummer. However, I do not believe my consummer is that slow and I never
// observe the drops with 'ipc'
// const std::string SimpleSwitch_PacketRedirectP4::event_logger_addr =
//     "ipc:///tmp/test_events_abc123";
const std::string SimpleSwitch_PacketRedirectP4::packet_in_addr =
"inproc://packets";
    // "ipc:///tmp/test_packet_in_abc1234";

SimpleSumeSwitch *SimpleSwitch_PacketRedirectP4::test_switch = nullptr;

const std::string SimpleSwitch_PacketRedirectP4::testdata_dir = TESTDATADIR;
const std::string SimpleSwitch_PacketRedirectP4::test_json =
    "add_sume_2.json";

TEST_F(SimpleSwitch_PacketRedirectP4, Recirculate) {
  // std::cout<< "starting test" << std::endl;
  static constexpr int port_in = 1;
  static constexpr int port_out_1 = 2;
  static constexpr int port_out_2 = 3;

  // std::vector<MatchKeyParam> match_key_1;
  // match_key_1.emplace_back(MatchKeyParam::Type::EXACT, std::string("\x06"));
  // match_key_1.emplace_back(MatchKeyParam::Type::EXACT, std::string("\x00", 1));
  // ActionData data_1;
  // data_1.push_back_action_data(port_out_1);
  // entry_handle_t h_1;
  // ASSERT_EQ(MatchErrorCode::SUCCESS,
  //           test_switch->mt_add_entry(0, "t_ingress_1", match_key_1,
  //                                     "_set_port", std::move(data_1), &h_1));

  // std::vector<MatchKeyParam> match_key_2;
  // match_key_2.emplace_back(MatchKeyParam::Type::EXACT, std::string("\x06"));
  // match_key_2.emplace_back(MatchKeyParam::Type::EXACT, std::string("\x01", 1));
  // ActionData data_2;
  // data_2.push_back_action_data(port_out_2);
  // entry_handle_t h_2;
  // ASSERT_EQ(MatchErrorCode::SUCCESS,
  //           test_switch->mt_add_entry(0, "t_ingress_1", match_key_2,
  //                                     "_set_port", std::move(data_2), &h_2));

  // std::vector<MatchKeyParam> match_key_3;
  // match_key_3.emplace_back(MatchKeyParam::Type::EXACT, std::string("\x06"));
  // // only PKT_INSTANCE_TYPE_NORMAL (= 0)
  // match_key_3.emplace_back(MatchKeyParam::Type::TERNARY,
  //                          std::string(4, '\x00'), std::string(4, '\xff'));
  // ActionData data_3;
  // entry_handle_t h_3;
  // ASSERT_EQ(MatchErrorCode::SUCCESS,
  //           test_switch->mt_add_entry(0, "t_egress", match_key_3,
  //                                     "_recirculate", std::move(data_3),
  //                                     &h_3, 1));

  // recirc packet needs to be larger because of remove_header call
  const char pkt[] = {
    '\x00', '\x00', '\x00', '\x00', // source eth
    '\x00', '\x00', '\x00', '\x00', // dst eth
    '\x12', '\x12', // ethertype
    '\x00', '\x00', '\x00', '\x01', // op1
    '\x00', // opcode
    '\x00', '\x00', '\x00', '\x01', // op2
  };
  std::cout << "0" << std::endl;
  packet_inject.send(port_in, pkt, sizeof(pkt));
  std::cout << "1" << std::endl;
  char recv_buffer[kMaxBufSize];
  int recv_port = -1;
  receiver.read(recv_buffer, sizeof(pkt), &recv_port);
  std::cout << "2" << std::endl;
  std::cout << "RECV BUFFER: " << recv_buffer << std::endl;
  printf("RECV BUFFER: %s\n", recv_buffer);
  ASSERT_EQ(port_out_2, recv_port);
}
