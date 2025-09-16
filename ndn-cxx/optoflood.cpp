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
        // Manually encode HopLimit TLV
        uint8_t val = *hopLimit;
        floodRequestEncoder.prependVarNumber(1); // Length
        floodRequestEncoder.prependRange(&val, &val + 1);
        floodRequestEncoder.prependVarNumber(tlv::HopLimit); // Type
    }

    if (traceHint) {
        // Manually encode TraceHint TLV
        const auto& hintData = *traceHint;
        floodRequestEncoder.prependVarNumber(hintData.size());
        floodRequestEncoder.prependRange(hintData.begin(), hintData.end());
        floodRequestEncoder.prependVarNumber(tlv::optoflood::TraceHint);
    }
    
    // Now wrap the encoded content in an InterestFloodRequest TLV
    encoding::Encoder interestFloodEncoder;
    interestFloodEncoder.prependVarNumber(floodRequestEncoder.size());
    interestFloodEncoder.prependRange(floodRequestEncoder.begin(), floodRequestEncoder.end());
    interestFloodEncoder.prependVarNumber(tlv::optoflood::InterestFloodRequest);
    
    // Finally, wrap everything in ApplicationParameters TLV
    encoding::Encoder appParamsEncoder;
    appParamsEncoder.prependVarNumber(interestFloodEncoder.size());
    appParamsEncoder.prependRange(interestFloodEncoder.begin(), interestFloodEncoder.end());
    appParamsEncoder.prependVarNumber(tlv::ApplicationParameters);

    return appParamsEncoder.block();
}


bool
isInterestFloodRequested(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.elements_size() == 0) {
    return false;
  }
  
  // parse() must be called before find()
  appParams.parse();
  return appParams.find(tlv::optoflood::InterestFloodRequest) != appParams.elements_end();
}

} // namespace optoflood
} // namespace ndn
