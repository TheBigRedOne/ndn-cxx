#include "optoflood.hpp"
#include "encoding/tlv-optoflood.hpp"
#include "encoding/encoder.hpp"
#include "encoding/block-helpers.hpp"
#include "tlv.hpp" // For tlv::ApplicationParameters

#include <vector>

namespace ndn {
namespace optoflood {

// Helper to build a Block with a non-negative integer payload
template<typename T>
static Block
makeIntegerBlock(uint32_t type, T value)
{
    Block block(type);
    size_t valueSize = ndn::encoding::sizeNonNegative(value);
    block.prepare(valueSize);
    ndn::encoding::writeNonNegative(block.value(), value, valueSize);
    return block;
}

Block
makeMobilityFlagBlock()
{
  return Block(tlv::optoflood::MobilityFlag);
}

Block
makeFloodIdBlock(uint64_t floodId)
{
  return makeIntegerBlock(tlv::optoflood::FloodId, floodId);
}

Block
makeNewFaceSeqBlock(uint32_t seq)
{
  return makeIntegerBlock(tlv::optoflood::NewFaceSeq, seq);
}

Block
makeTraceHintBlock(const std::vector<uint8_t>& hint)
{
  Block block(tlv::optoflood::TraceHint);
  if (!hint.empty()) {
    block.assign(hint.data(), hint.size());
  }
  return block;
}

// Visit MetaInfo to find a specific TLV type
template<typename T, typename OnFound>
static T
visitMetaInfo(const MetaInfo& metaInfo, uint32_t type, OnFound onFound)
{
  T result = std::nullopt;
  metaInfo.visit([&](const Block& block) {
    if (block.type() == type) {
      result = onFound(block);
      return false; // Stop visiting
    }
    return true; // Continue visiting
  });
  return result;
}

bool
hasMobilityFlag(const MetaInfo& metaInfo)
{
  return visitMetaInfo<bool>(metaInfo, tlv::optoflood::MobilityFlag,
                             [](const Block&) { return true; }).value_or(false);
}

std::optional<uint64_t>
getFloodId(const MetaInfo& metaInfo)
{
  return visitMetaInfo<std::optional<uint64_t>>(
    metaInfo, tlv::optoflood::FloodId,
    [](const Block& block) { return readNonNegativeInteger(block); });
}

std::optional<uint32_t>
getNewFaceSeq(const MetaInfo& metaInfo)
{
  return visitMetaInfo<std::optional<uint32_t>>(
    metaInfo, tlv::optoflood::NewFaceSeq,
    [](const Block& block) { return static_cast<uint32_t>(readNonNegativeInteger(block)); });
}

std::optional<std::vector<uint8_t>>
getTraceHint(const MetaInfo& metaInfo)
{
  return visitMetaInfo<std::optional<std::vector<uint8_t>>>(
    metaInfo, tlv::optoflood::TraceHint,
    [](const Block& block) { return std::vector<uint8_t>(block.value(), block.value_end()); });
}

Block
makeInterestFloodingParameters(const std::optional<std::vector<uint8_t>>& traceHint,
                               const std::optional<uint8_t>& hopLimit)
{
  ndn::encoding::Encoder appParamsEncoder;
  size_t appParamsSize = 0;

  ndn::encoding::Encoder floodRequestEncoder;
  size_t floodRequestSize = 0;

  if (hopLimit) {
      Block hopLimitBlock(tlv::HopLimit);
      hopLimitBlock.push_back(*hopLimit);
      floodRequestEncoder.prependBlock(hopLimitBlock);
      floodRequestSize += hopLimitBlock.wireEncode().size();
  }

  if (traceHint) {
      Block traceHintBlock = makeTraceHintBlock(*traceHint);
      floodRequestEncoder.prependBlock(traceHintBlock);
      floodRequestSize += traceHintBlock.wireEncode().size();
  }

  appParamsEncoder.prependBlock(tlv::optoflood::InterestFloodRequest,
                              floodRequestEncoder.begin(), floodRequestEncoder.end());
  appParamsSize += appParamsEncoder.size();

  return appParamsEncoder.prependBlock(tlv::ApplicationParameters);
}

bool
isInterestFloodRequested(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (!appParams.hasWire()) {
    return false;
  }
  try {
    appParams.parse();
    for (const auto& item : appParams.elements()) {
        if (item.type() == tlv::optoflood::InterestFloodRequest) {
            return true;
        }
    }
    return false;
  }
  catch (const tlv::Error&) {
    return false;
  }
}

} // namespace optoflood
} // namespace ndn
