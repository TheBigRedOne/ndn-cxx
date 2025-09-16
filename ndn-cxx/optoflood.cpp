#include "optoflood.hpp"
#include "encoding/tlv-optoflood.hpp"
#include "encoding/encoder.hpp"
#include "encoding/block-helpers.hpp"
#include "encoding/tlv.hpp"

#include <vector>

namespace ndn {
namespace optoflood {

Block
makeMobilityFlagBlock()
{
  return makeEmptyBlock(tlv::optoflood::MobilityFlag);
}

Block
makeFloodIdBlock(uint64_t floodId)
{
  return makeNonNegativeIntegerBlock(tlv::optoflood::FloodId, floodId);
}

Block
makeNewFaceSeqBlock(uint32_t seq)
{
  return makeNonNegativeIntegerBlock(tlv::optoflood::NewFaceSeq, seq);
}

Block
makeTraceHintBlock(const std::vector<uint8_t>& hint)
{
  return makeBinaryBlock(tlv::optoflood::TraceHint, hint.begin(), hint.end());
}

bool
hasMobilityFlag(const MetaInfo& metaInfo)
{
  return metaInfo.findAppMetaInfo(tlv::optoflood::MobilityFlag) != nullptr;
}

std::optional<uint64_t>
getFloodId(const MetaInfo& metaInfo)
{
  if (const auto* block = metaInfo.findAppMetaInfo(tlv::optoflood::FloodId)) {
    return readNonNegativeInteger(*block);
  }
  return std::nullopt;
}

std::optional<uint32_t>
getNewFaceSeq(const MetaInfo& metaInfo)
{
  if (const auto* block = metaInfo.findAppMetaInfo(tlv::optoflood::NewFaceSeq)) {
    return readNonNegativeIntegerAs<uint32_t>(*block);
  }
  return std::nullopt;
}

std::optional<std::vector<uint8_t>>
getTraceHint(const MetaInfo& metaInfo)
{
  if (const auto* block = metaInfo.findAppMetaInfo(tlv::optoflood::TraceHint)) {
    return std::vector<uint8_t>(block->value_begin(), block->value_end());
  }
  return std::nullopt;
}

Block
makeInterestFloodingParameters(const std::optional<std::vector<uint8_t>>& traceHint,
                               const std::optional<uint8_t>& hopLimit)
{
    encoding::Encoder floodRequestEncoder;

    if (hopLimit) {
        uint8_t val = *hopLimit;
        encoding::prependBlock(floodRequestEncoder, makeBinaryBlock(tlv::HopLimit, &val, &val + 1));
    }

    if (traceHint) {
        encoding::prependBlock(floodRequestEncoder, makeTraceHintBlock(*traceHint));
    }
    
    Block floodRequestBlock = floodRequestEncoder.block();
    floodRequestBlock.encode(); // Ensure wire format is generated
    
    encoding::Encoder appParamsEncoder;
    prependBlock(appParamsEncoder, makeNestedBlock(tlv::optoflood::InterestFloodRequest, floodRequestBlock));
    
    return makeNestedBlock(tlv::ApplicationParameters, appParamsEncoder.block());
}


bool
isInterestFloodRequested(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.elements_size() == 0) {
    return false;
  }
  
  return appParams.find(tlv::optoflood::InterestFloodRequest) != appParams.elements_end();
}

} // namespace optoflood
} // namespace ndn
