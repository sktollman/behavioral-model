#ifndef _SUME_SWITCH_P4_
#define _SUME_SWITCH_P4_

// File "simple_sume_switch.p4"
// NetFPGA SUME P4 Switch declaration
// core library needed for packet_in definition
#include <core.p4>

// one-hot encoded: {DMA, NF3, DMA, NF2, DMA, NF1, DMA, NF0}
typedef bit<8> port_t;

/* standard sume switch metadata */
struct sume_metadata_t {
    bit<16> dma_q_size; // measured in 32-byte words
    bit<16> nf3_q_size; // measured in 32-byte words
    bit<16> nf2_q_size; // measured in 32-byte words
    bit<16> nf1_q_size; // measured in 32-byte words
    bit<16> nf0_q_size; // measured in 32-byte words
    bit<8> send_dig_to_cpu; // send digest_data to CPU
    bit<8> drop;
    port_t dst_port; // one-hot encoded: {DMA, NF3, DMA, NF2, DMA, NF1, DMA, NF0}
    port_t src_port; // one-hot encoded: {DMA, NF3, DMA, NF2, DMA, NF1, DMA, NF0}
    bit<16> pkt_len; // unsigned int
}

/**
 * Programmable parser.
 * @param b input packet
 * @param <H> type of headers; defined by user
 * @param parsedHeaders headers constructed by parser
 * @param <M> type of metadata; defined by user
 * @param metadata; metadata constructed by parser
 * @param sume_metadata; standard metadata for the sume switch
 */
parser Parser<H, M, D>(packet_in b,
                       out H parsedHeaders,
                       out M user_metadata,
                       out D digest_data,
                       inout sume_metadata_t sume_metadata);

/**
 * Match-action pipeline
 * @param <H> type of input and output headers
 * @param parsedHeaders; headers received from the parser and sent to the deparser
 * @param <M> type of input and output user metadata
 * @param user_metadata; metadata defined by the user
 * @param sume_metadata; standard metadata for the sume switch
 */
control Pipe<H, M, D>(inout H parsedHeaders,
                      inout M user_metadata,
                      inout D digest_data,
                      inout sume_metadata_t sume_metadata);

/**
 * Switch deparser.
 * @param b output packet
 * @param <H> type of headers; defined by user
 * @param parsedHeaders headers for output packet
 * @param <M> type of metadata; defined by user
 * @param user_metadata; defined by user
 * @param sume_metadata; standard metadata for the sume switch
 */
control Deparser<H, M, D>(packet_out b,
                          in H parsedHeaders,
                          in M user_metadata,
                          inout D digest_data,
                          inout sume_metadata_t sume_metadata);

/**
 * Top-level package declaration - must be instantiated by user.
 * The arguments to the package indicate blocks that
 * must be instantiated by the user.
 * @param <H> user-defined type of the headers processed.
 */
package SimpleSumeSwitch<H, M, D>(Parser<H, M, D> p,
                                  Pipe<H, M, D> map,
                                  Deparser<H, M, D> d);

#endif  /* _SUME_SWITCH_P4_ */

