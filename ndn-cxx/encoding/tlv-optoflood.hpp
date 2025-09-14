#pragma once

#include "tlv.hpp"

namespace ndn {
namespace tlv {
namespace optoflood {

/**
 * @brief TLV types for OptoFlood protocol extensions.
 *
 * These types are used within the MetaInfo of Data packets
 * and ApplicationParameters of Interest packets to signal
 * and control the OptoFlood mobility mechanism.
 */
enum : uint32_t {
  /**
   * @brief A flag indicating that the packet is part of a mobility-related flood.
   *
   * This TLV has a zero-length value. Its presence alone acts as the flag.
   */
  MobilityFlag = 201,

  /**
   * @brief A unique identifier for a flooding event.
   *
   * Used by forwarders to deduplicate packets belonging to the same flood.
   * The value is a non-negative integer.
   */
  FloodId = 202,

  /**
   * @brief A sequence number associated with the producer's new face.
   *
   * Used to ensure consistency and prevent stale information when establishing
   * temporary forwarding entries.
   */
  NewFaceSeq = 203,

  /**
   * @brief A hint for guiding the flood towards the producer's last known location.
   *
   * The value contains trace information, such as recent Points of Attachment.
   */
  TraceHint = 204,

  /**
   * @brief A TLV block within ApplicationParameters to request Interest flooding.
   *
   * This block can contain other TLVs like TraceHint and HopLimit to guide the flood.
   */
  InterestFloodRequest = 205
};

} // namespace optoflood
} // namespace tlv
} // namespace ndn
