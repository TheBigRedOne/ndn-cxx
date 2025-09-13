#pragma once

#include "block.hpp"
#include "meta-info.hpp"
#include "interest.hpp"

#include <optional>
#include <vector>

namespace ndn {
namespace optoflood {

/**
 * @brief Creates a Block containing a zero-length MobilityFlag TLV.
 */
Block
makeMobilityFlagBlock();

/**
 * @brief Creates a Block containing a FloodId TLV.
 * @param floodId the unique identifier for the flooding event.
 */
Block
makeFloodIdBlock(uint64_t floodId);

/**
 * @brief Creates a Block containing a NewFaceSeq TLV.
 * @param seq the sequence number of the new face.
 */
Block
makeNewFaceSeqBlock(uint32_t seq);

/**
 * @brief Creates a Block containing a TraceHint TLV.
 * @param hint a byte vector representing the trace hint.
 */
Block
makeTraceHintBlock(const std::vector<uint8_t>& hint);

/**
 * @brief Checks if the MetaInfo contains a MobilityFlag.
 */
bool
hasMobilityFlag(const MetaInfo& metaInfo);

/**
 * @brief Extracts the FloodId from MetaInfo, if present.
 */
std::optional<uint64_t>
getFloodId(const MetaInfo& metaInfo);

/**
 * @brief Extracts the NewFaceSeq from MetaInfo, if present.
 */
std::optional<uint32_t>
getNewFaceSeq(const MetaInfo& metaInfo);

/**
 * @brief Extracts the TraceHint from MetaInfo as a byte vector, if present.
 */
std::optional<std::vector<uint8_t>>
getTraceHint(const MetaInfo& metaInfo);

} // namespace optoflood
} // namespace ndn
