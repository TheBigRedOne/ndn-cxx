#include "optoflood.hpp"
#include "encoding/tlv-optoflood.hpp"
#include "encoding/encoder.hpp"
#include "encoding/block-helpers.hpp"
#include "encoding/tlv.hpp"
#include "util/logger.hpp"

#include <vector>

namespace ndn {
namespace optoflood {

NDN_LOG_INIT(ndn.optoflood);

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
  // Return inner InterestFloodRequest TLV; caller stores it into ApplicationParameters value
  Block floodReq(tlv::optoflood::InterestFloodRequest);
  if (hopLimit) {
    Block hop = makeNonNegativeIntegerBlock(tlv::HopLimit, static_cast<uint64_t>(*hopLimit));
    floodReq.push_back(hop);
  }
  floodReq.encode();
  return floodReq;
}


bool
isInterestFloodRequested(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.value_size() == 0) {
    return false;
  }
  Block v(appParams.value_bytes());
  try {
    v.parse();
  }
  catch (const tlv::Error& e) {
    NDN_LOG_WARN("Interest " << interest.getName()
                  << " carries malformed ApplicationParameters (" << e.what() << ')');
    return false;
  }
  if (v.type() != tlv::optoflood::InterestFloodRequest) {
    NDN_LOG_DEBUG("Interest " << interest.getName()
                  << " ApplicationParameters type=" << v.type()
                  << " (expect tlv::optoflood::InterestFloodRequest)");
    return false;
  }
  return true;
}

std::optional<uint8_t>
getFloodHopLimit(const Interest& interest)
{
  const auto& appParams = interest.getApplicationParameters();
  if (appParams.value_size() == 0) {
    return std::nullopt;
  }
  Block v(appParams.value_bytes());
  try {
    v.parse();
  }
  catch (const tlv::Error& e) {
    NDN_LOG_WARN("Interest " << interest.getName()
                  << " carries malformed InterestFloodRequest (" << e.what() << ')');
    return std::nullopt;
  }
  if (v.type() != tlv::optoflood::InterestFloodRequest) {
    NDN_LOG_DEBUG("Interest " << interest.getName()
                  << " ApplicationParameters type=" << v.type()
                  << " (expect tlv::optoflood::InterestFloodRequest)");
    return std::nullopt;
  }
  auto hop = v.find(tlv::HopLimit);
  if (hop != v.elements_end()) {
    return static_cast<uint8_t>(readNonNegativeInteger(*hop));
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
