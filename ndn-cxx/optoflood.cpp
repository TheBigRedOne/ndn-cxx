#include "optoflood.hpp"
#include "encoding/tlv-optoflood.hpp"
#include "encoding/encoder.hpp"
#include "encoding/block-helpers.hpp"
#include <ndn-cxx/lp/tags.hpp> // For HopLimitTag

#include <vector>

namespace ndn {
namespace optoflood {

Block
makeMobilityFlagBlock()
{
  return Block(tlv::optoflood::MobilityFlag);
}

Block
makeFloodIdBlock(uint64_t floodId)
{
  Block block(tlv::optoflood::FloodId);
  ndn::encoding::prependNonNegativeInteger(block, floodId);
  return block;
}

Block
makeNewFaceSeqBlock(uint32_t seq)
{
  Block block(tlv::optoflood::NewFaceSeq);
  ndn::encoding::prependNonNegativeInteger(block, seq);
  return block;
}

Block
makeTraceHintBlock(const std::vector<uint8_t>& hint)
{
  Block block(tlv::optoflood::TraceHint);
  block.assign(hint.begin(), hint.end());
  return block;
}

bool
hasMobilityFlag(const MetaInfo& metaInfo)
{
  return metaInfo.has(tlv::optoflood::MobilityFlag);
}

std::optional<uint64_t>
getFloodId(const MetaInfo& metaInfo)
{
  const auto& block = metaInfo.get(tlv::optoflood::FloodId);
  if (block.empty()) {
    return std::nullopt;
  }
  return readNonNegativeInteger(block);
}

std::optional<uint32_t>
getNewFaceSeq(const MetaInfo& metaInfo)
{
  const auto& block = metaInfo.get(tlv::optoflood::NewFaceSeq);
  if (block.empty()) {
    return std::nullopt;
  }
  return static_cast<uint32_t>(readNonNegativeInteger(block));
}

std::optional<std::vector<uint8_t>>
getTraceHint(const MetaInfo& metaInfo)
{
  const auto& block = metaInfo.get(tlv::optoflood::TraceHint);
  if (block.empty()) {
    return std::nullopt;
  }
  return std::vector<uint8_t>(block.begin(), block.end());
}

// --- Interest Packet Helpers ---

Block
makeInterestFloodingParameters(const std::optional<std::vector<uint8_t>>& traceHint,
                               const std::optional<uint8_t>& hopLimit)
{
  Encoder encoder;
  encoder.prependBlock(tlv::ApplicationParameters);
  size_t appParamsLength = 0;

  Block floodRequestBlock(tlv::optoflood::InterestFloodRequest);
  Encoder floodRequestEncoder;
  size_t floodRequestLength = 0;

  if (traceHint) {
    floodRequestEncoder.prependBlock(makeTraceHintBlock(*traceHint));
    floodRequestLength += floodRequestEncoder.size();
  }
  if (hopLimit) {
    Block hopLimitBlock(tlv::lp::HopLimit);
    hopLimitBlock.push_back(*hopLimit);
    floodRequestEncoder.prependBlock(hopLimitBlock);
    floodRequestLength += floodRequestEncoder.size();
  }

  floodRequestBlock.encodeHeader(floodRequestLength);
  floodRequestBlock.insert(floodRequestBlock.end(),
                           floodRequestEncoder.begin(), floodRequestEncoder.end());

  encoder.prependBlock(floodRequestBlock);
  appParamsLength += encoder.size();
  
  encoder.prependHeader(appParamsLength);

  return encoder.getBlock();
}

bool
isInterestFloodRequested(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.empty()) {
    return false;
  }
  try {
    appParams.parse();
    return appParams.has(tlv::optoflood::InterestFloodRequest);
  }
  catch (const tlv::Error&) {
    return false;
  }
}

} // namespace optoflood
} // namespace ndn
