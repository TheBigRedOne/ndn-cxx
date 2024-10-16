/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2024 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef NDN_CXX_DATA_HPP
#define NDN_CXX_DATA_HPP

#include "ndn-cxx/detail/packet-base.hpp"
#include "ndn-cxx/encoding/block.hpp"
#include "ndn-cxx/meta-info.hpp"
#include "ndn-cxx/name.hpp"
#include "ndn-cxx/security/security-common.hpp"
#include "ndn-cxx/signature-info.hpp"

namespace ndn {

/**
 * @brief Represents a %Data packet.
 * @sa https://docs.named-data.net/NDN-packet-spec/0.3/data.html
 */
class Data : public PacketBase, public std::enable_shared_from_this<Data>
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  /**
   * @brief Construct an unsigned Data packet with given @p name and empty Content.
   *
   * @warning In certain contexts that use `Data::shared_from_this()`, the Data must be created
   *          using `std::make_shared`. Otherwise, `shared_from_this()` will throw `std::bad_weak_ptr`.
   *          One example where this is necessary is storing Data into a subclass of InMemoryStorage.
   */
  explicit
  Data(const Name& name = Name());

  /**
   * @brief Construct a Data packet by decoding from @p wire.
   * @param wire TLV element of type tlv::Data; may be signed or unsigned.
   *
   * @warning In certain contexts that use `Data::shared_from_this()`, the Data must be created
   *          using `std::make_shared`. Otherwise, `shared_from_this()` will throw `std::bad_weak_ptr`.
   *          One example where this is necessary is storing Data into a subclass of InMemoryStorage.
   */
  explicit
  Data(const Block& wire);

  /**
   * @brief Prepend wire encoding to @p encoder.
   * @param encoder EncodingEstimator or EncodingBuffer instance.
   * @param wantUnsignedPortionOnly If true, prepend only Name, MetaInfo, Content, and
   *        SignatureInfo to @p encoder, but omit SignatureValue and the outermost TLV
   *        Type and Length of the %Data element. This is intended to be used with
   *        wireEncode(EncodingBuffer&, span<const uint8_t>) const.
   * @throw Error Signature is not present and @p wantUnsignedPortionOnly is false.
   */
  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder, bool wantUnsignedPortionOnly = false) const;

  /**
   * @brief Finalize Data packet encoding with the specified signature.
   * @param encoder EncodingBuffer containing Name, MetaInfo, Content, and SignatureInfo, but
   *                without SignatureValue and the outermost Type-Length of the %Data element.
   * @param signature Raw signature bytes, without TLV Type and Length; this will become the
   *                  TLV-VALUE of the SignatureValue element added to the packet.
   *
   * This method is intended to be used in concert with `wireEncode(encoder, true)`, e.g.:
   * @code
   * Data data;
   * ...
   * EncodingBuffer encoder;
   * data.wireEncode(encoder, true);
   * ...
   * auto signature = create_signature_over_signed_portion(encoder.data(), encoder.size());
   * data.wireEncode(encoder, signature);
   * @endcode
   */
  const Block&
  wireEncode(EncodingBuffer& encoder, span<const uint8_t> signature) const;

  /**
   * @brief Encode into a Block.
   * @pre Data must be signed.
   */
  const Block&
  wireEncode() const;

  /**
   * @brief Decode from @p wire.
   */
  void
  wireDecode(const Block& wire);

  /**
   * @brief Check if this instance has cached wire encoding.
   */
  bool
  hasWire() const noexcept
  {
    return m_wire.hasWire();
  }

  /**
   * @brief Get the full name (including implicit digest).
   * @pre hasWire() == true, i.e., wireEncode() must have been called.
   * @throw Error Data has no wire encoding
   */
  const Name&
  getFullName() const;

public: // Data fields
  /**
   * @brief Get the %Data name.
   */
  const Name&
  getName() const noexcept
  {
    return m_name;
  }

  /**
   * @brief Set the %Data name.
   * @return A reference to this Data, to allow chaining.
   */
  Data&
  setName(const Name& name);

  /**
   * @brief Get the `MetaInfo` element.
   */
  const MetaInfo&
  getMetaInfo() const noexcept
  {
    return m_metaInfo;
  }

  /**
   * @brief Set the `MetaInfo` element.
   * @return A reference to this Data, to allow chaining.
   */
  Data&
  setMetaInfo(const MetaInfo& metaInfo);

  /**
   * @brief Return whether this Data has a `Content` element.
   */
  bool
  hasContent() const noexcept
  {
    return m_content.isValid();
  }

  /**
   * @brief Get the `Content` element.
   *
   * If the element is not present (hasContent() == false), an invalid Block will be returned.
   *
   * The value of the returned Content Block (if valid) can be accessed through
   *   - Block::value_bytes(), or
   *   - Block::value() and Block::value_size(), or
   *   - Block::value_begin() and Block::value_end().
   *
   * @sa hasContent()
   * @sa Block::value_bytes(), Block::blockFromValue(), Block::parse()
   */
  const Block&
  getContent() const noexcept
  {
    return m_content;
  }

  /**
   * @brief Set `Content` from a Block.
   * @param block TLV element to be used as Content; must be valid
   * @return A reference to this Data, to allow chaining.
   *
   * If the block's TLV-TYPE is tlv::Content, it will be used directly as this Data's
   * Content element. Otherwise, the block will be nested into a Content element.
   */
  Data&
  setContent(const Block& block);

  /**
   * @brief Set `Content` by copying from a contiguous sequence of bytes.
   * @param value buffer with the TLV-VALUE of the content
   * @return A reference to this Data, to allow chaining.
   */
  Data&
  setContent(span<const uint8_t> value);

  /**
   * @brief Set `Content` by copying from a string.
   * @param value string with the TLV-VALUE of the content
   * @return A reference to this Data, to allow chaining.
   */
  Data&
  setContent(std::string_view value);

  /**
   * @brief Set `Content` from a shared buffer.
   * @param value buffer with the TLV-VALUE of the content; must not be null
   * @return A reference to this Data, to allow chaining.
   */
  Data&
  setContent(ConstBufferPtr value);

  Data&
  setContent(std::nullptr_t) = delete;

  /**
   * @brief Remove the `Content` element.
   * @return A reference to this Data, to allow chaining.
   * @post hasContent() == false
   */
  Data&
  unsetContent();

  /**
   * @brief Get the `SignatureInfo` element.
   */
  const SignatureInfo&
  getSignatureInfo() const noexcept
  {
    return m_signatureInfo;
  }

  /**
   * @brief Set the `SignatureInfo` element.
   *
   * This is a low-level function that should not normally be called directly by applications.
   * Instead, provide a SignatureInfo to the SigningInfo object passed to KeyChain::sign().
   *
   * @return A reference to this Data, to allow chaining.
   * @warning SignatureInfo is overwritten when the packet is signed via KeyChain::sign().
   * @sa SigningInfo
   */
  Data&
  setSignatureInfo(const SignatureInfo& info);

  /**
   * @brief Get the `SignatureValue` element.
   */
  const Block&
  getSignatureValue() const noexcept
  {
    return m_signatureValue;
  }

  /**
   * @brief Set `SignatureValue` by copying from a contiguous sequence of bytes.
   * @param value buffer from which the TLV-VALUE of the SignatureValue will be copied
   * @return A reference to this Data, to allow chaining.
   *
   * This is a low-level function that should not normally be called directly by applications.
   * Instead, use KeyChain::sign() to sign the packet.
   *
   * @warning SignatureValue is overwritten when the packet is signed via KeyChain::sign().
   */
  Data&
  setSignatureValue(span<const uint8_t> value);

  /**
   * @brief Set `SignatureValue` from a shared buffer.
   * @param value buffer containing the TLV-VALUE of the SignatureValue; must not be null
   * @return A reference to this Data, to allow chaining.
   *
   * This is a low-level function that should not normally be called directly by applications.
   * Instead, use KeyChain::sign() to sign the packet.
   *
   * @warning SignatureValue is overwritten when the packet is signed via KeyChain::sign().
   */
  Data&
  setSignatureValue(ConstBufferPtr value);

  Data&
  setSignatureValue(std::nullptr_t) = delete;

  /**
   * @brief Extract ranges of Data covered by the signature.
   * @throw Error Data cannot be encoded or is missing ranges necessary for signing
   * @warning The returned pointers will be invalidated if wireDecode() or wireEncode() are called.
   */
  [[nodiscard]] InputBuffers
  extractSignedRanges() const;

public: // MetaInfo fields
  /**
   * @copydoc MetaInfo::getType()
   */
  uint32_t
  getContentType() const noexcept
  {
    return m_metaInfo.getType();
  }

  /**
   * @copydoc MetaInfo::setType()
   */
  Data&
  setContentType(uint32_t type);

  /**
   * @copydoc MetaInfo::getFreshnessPeriod()
   */
  time::milliseconds
  getFreshnessPeriod() const noexcept
  {
    return m_metaInfo.getFreshnessPeriod();
  }

  /**
   * @copydoc MetaInfo::setFreshnessPeriod()
   */
  Data&
  setFreshnessPeriod(time::milliseconds freshnessPeriod);

  /**
   * @copydoc MetaInfo::getFinalBlock()
   */
  const std::optional<name::Component>&
  getFinalBlock() const noexcept
  {
    return m_metaInfo.getFinalBlock();
  }

  /**
   * @copydoc MetaInfo::setFinalBlock()
   */
  Data&
  setFinalBlock(std::optional<name::Component> finalBlockId);

public: // SignatureInfo fields
  /**
   * @copydoc SignatureInfo::getSignatureType()
   */
  int32_t
  getSignatureType() const noexcept
  {
    return m_signatureInfo.getSignatureType();
  }

  /**
   * @brief Get the `KeyLocator` element.
   */
  std::optional<KeyLocator>
  getKeyLocator() const noexcept
  {
    if (m_signatureInfo.hasKeyLocator()) {
      return m_signatureInfo.getKeyLocator();
    }
    return std::nullopt;
  }

protected:
  /**
   * @brief Clear wire encoding and cached FullName.
   * @note This does not clear the SignatureValue.
   */
  void
  resetWire();

private:
  Name m_name;
  MetaInfo m_metaInfo;
  Block m_content;
  SignatureInfo m_signatureInfo;
  Block m_signatureValue;

  mutable Block m_wire;
  mutable Name m_fullName; // cached FullName computed from m_wire
};

#ifndef DOXYGEN
extern template size_t
Data::wireEncode<encoding::EncoderTag>(EncodingBuffer&, bool) const;

extern template size_t
Data::wireEncode<encoding::EstimatorTag>(EncodingEstimator&, bool) const;
#endif

std::ostream&
operator<<(std::ostream& os, const Data& data);

bool
operator==(const Data& lhs, const Data& rhs);

inline bool
operator!=(const Data& lhs, const Data& rhs)
{
  return !(lhs == rhs);
}

class MetaInfo
{
public:
  // MobilityFlag 相关代码
  void setMobilityFlag(bool flag) {
    m_mobilityFlag = flag;
  }

  bool getMobilityFlag() const {
    return m_mobilityFlag;
  }

  // HopLimit 相关代码
  void setHopLimit(uint8_t hopLimit) {
    m_hopLimit = hopLimit;
  }

  uint8_t getHopLimit() const {
    return m_hopLimit;
  }

  bool hasHopLimit() const {
    return m_hopLimit > 0;
  }

  // TimeStamp 相关代码
  void setTimeStamp(time::milliseconds timeStamp) {
    m_timeStamp = timeStamp;
  }

  time::milliseconds getTimeStamp() const {
    return m_timeStamp;
  }

  // Encode mobility flag and HopLimit in the wire format
  template<encoding::Tag TAG>
  size_t wireEncode(EncodingImpl<TAG>& encoder) const
  {
    size_t totalLength = 0;

    // MobilityFlag 编码逻辑
    if (m_mobilityFlag) {
      totalLength += encoder.appendNonNegativeIntegerBlock(tlv::MobilityFlag, 1);
    }

    // HopLimit 编码逻辑（仅当 HopLimit > 0 时进行编码）
    if (hasHopLimit()) {
      totalLength += encoder.appendNonNegativeIntegerBlock(tlv::HopLimit, m_hopLimit);
    }

    // TimeStamp 编码逻辑
    totalLength += encoder.appendNonNegativeIntegerBlock(tlv::TimeStamp, m_timeStamp.count());

    return totalLength;
  }

  // Decode mobility flag and HopLimit from wire format
  void wireDecode(const Block& block)
  {
    // 解码 MobilityFlag
    if (block.type() == tlv::MobilityFlag) {
      m_mobilityFlag = true;
    }

    // 解码 HopLimit
    if (block.type() == tlv::HopLimit) {
      m_hopLimit = readNonNegativeInteger(block);
    }

    // 解码 TimeStamp
    if (block.type() == tlv::TimeStamp) {
      m_timeStamp = time::milliseconds(readNonNegativeInteger(block));
    }
  }

private:
  bool m_mobilityFlag = false;
  uint8_t m_hopLimit = 0;  // 默认值为 0，表示不限制跳数
  time::milliseconds m_timeStamp = time::steady_clock::now();  // 数据包的生成时间
};

} // namespace ndn

#endif // NDN_CXX_DATA_HPP
