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

// TraceHint removed in current implementation

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

// TraceHint removed in current implementation

Block
makeInterestFloodingParameters(const std::optional<uint8_t>& hopLimit)
{
  // Build inner fields using safe Block constructors
  Block floodReq(tlv::optoflood::InterestFloodRequest);

  if (hopLimit) {
    // HopLimit as NonNegativeInteger encodes to 1 byte for small values
    Block hop = makeNonNegativeIntegerBlock(tlv::HopLimit, static_cast<uint64_t>(*hopLimit));
    floodReq.push_back(hop);
  }

  floodReq.encode();

  Block appParams(tlv::ApplicationParameters);
  appParams.push_back(floodReq);
  appParams.encode();
  return appParams;
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

std::optional<uint8_t>
getFloodHopLimit(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.elements_size() == 0) {
    return std::nullopt;
  }
  appParams.parse();
  auto it = appParams.find(tlv::optoflood::InterestFloodRequest);
  if (it == appParams.elements_end()) {
    return std::nullopt;
  }
  // Parse inner TLVs
  it->parse();
  auto hop = it->find(tlv::HopLimit);
  if (hop != it->elements_end()) {
    // HopLimit value is 1 byte
    if (hop->value_size() == 1) {
      return static_cast<uint8_t>(*hop->value_begin());
    }
  }
  return std::nullopt;
}

std::optional<std::vector<uint8_t>>
getFloodTraceHint(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.elements_size() == 0) {
    return std::nullopt;
  }
  appParams.parse();
  auto it = appParams.find(tlv::optoflood::InterestFloodRequest);
  if (it == appParams.elements_end()) {
    return std::nullopt;
  }
  it->parse();
  auto hint = it->find(tlv::optoflood::TraceHint);
  if (hint != it->elements_end()) {
    return std::vector<uint8_t>(hint->value_begin(), hint->value_end());
  }
  return std::nullopt;
}

} // namespace optoflood
} // namespace ndn
