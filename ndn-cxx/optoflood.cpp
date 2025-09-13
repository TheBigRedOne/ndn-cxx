#include "optoflood.hpp"
#include "encoding/tlv-optoflood.hpp"
#include "encoding/encoder.hpp"
#include "encoding/buffer-stream.hpp"
#include "util/non-negative-integer.hpp"

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
  returndetail::makeNonNegativeIntegerBlock(tlv::optoflood::FloodId, floodId);
}

Block
makeNewFaceSeqBlock(uint32_t seq)
{
  return detail::makeNonNegativeIntegerBlock(tlv::optoflood::NewFaceSeq, seq);
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

} // namespace optoflood
} // namespace ndn
