#pragma once

#include "encoding/block.hpp"
#include "meta-info.hpp"
#include "interest.hpp"

#include <optional>
#include <vector>

namespace ndn {
namespace optoflood {

// All functions are implemented in optoflood.cpp

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
 * @brief Creates a Block containing ApplicationParameters for Interest flooding.
 *
 * @param traceHint An optional byte vector for the TraceHint.
 * @param hopLimit An optional hop limit for the flood.
 * @return A Block containing the encoded ApplicationParameters.
 */
Block
makeInterestFloodingParameters(const std::optional<uint8_t>& hopLimit);

/**
 * @brief Checks if an Interest's ApplicationParameters contain a flood request.
 */
bool
isInterestFloodRequested(const Interest& interest);

/**
 * @brief Extracts requested HopLimit for flooding from ApplicationParameters, if present.
 */
std::optional<uint8_t>
getFloodHopLimit(const Interest& interest);


} // namespace optoflood
} // namespace ndn
