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

#ifndef NDN_CXX_UTIL_STRING_HELPER_HPP
#define NDN_CXX_UTIL_STRING_HELPER_HPP

#include "ndn-cxx/encoding/buffer.hpp"
#include "ndn-cxx/util/span.hpp"

#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ndn {

class StringHelperError : public std::invalid_argument
{
public:
  using std::invalid_argument::invalid_argument;
};

/**
 * @brief Output the hex representation of @p num to the output stream @p os.
 *
 * @param os Output stream
 * @param num Number to print in hexadecimal format
 * @param wantUpperCase if true, print uppercase hex chars; the default is to use lowercase
 *
 * The output string is a continuous sequence of hex characters without any whitespace separators.
 */
void
printHex(std::ostream& os, uint64_t num, bool wantUpperCase = false);

/**
 * @brief Output the hex representation of the bytes in @p buffer to the output stream @p os.
 *
 * @param os Output stream
 * @param buffer Range of bytes to print in hexadecimal format
 * @param wantUpperCase if true (the default) print uppercase hex chars
 *
 * Each octet of input is always converted to two hex characters (e.g., "00" for octet==0).
 * The output string is a continuous sequence of hex characters without any whitespace separators.
 */
void
printHex(std::ostream& os, span<const uint8_t> buffer, bool wantUpperCase = true);

/**
 * @brief Helper class to convert a number to hexadecimal format, for use with
 *        stream insertion operators.
 *
 * Example usage:
 *
 * @code
 * std::cout << AsHex{42};                   // outputs "0x2a"
 * std::cout << std::uppercase << AsHex{42}; // outputs "0x2A"
 * @endcode
 */
class AsHex
{
public:
  constexpr explicit
  AsHex(uint64_t val) noexcept
    : m_value(val)
  {
  }

private:
  friend std::ostream&
  operator<<(std::ostream& os, const AsHex& hex)
  {
    printHex(os, hex.m_value, os.flags() & std::ostream::uppercase);
    return os;
  }

private:
  uint64_t m_value;
};

/**
 * @brief Return a string containing the hex representation of the bytes in @p buffer.
 *
 * @param buffer Range of bytes to convert to hexadecimal format
 * @param wantUpperCase if true (the default) use uppercase hex chars
 *
 * Each octet of input is always converted to two hex characters (e.g., "00" for octet==0).
 * The output string is a continuous sequence of hex characters without any whitespace separators.
 */
[[nodiscard]] std::string
toHex(span<const uint8_t> buffer, bool wantUpperCase = true);

/**
 * @brief Convert a hex string to a raw byte buffer.
 * @param hexString Sequence of pairs of hex numbers (lower and upper case can be mixed)
 *        without any whitespace separators (e.g., "48656C6C6F2C20776F726C6421")
 * @throw StringHelperError Input string is invalid
 */
std::shared_ptr<Buffer>
fromHex(std::string_view hexString);

/**
 * @brief Convert (the least significant nibble of) @p n to the corresponding hex character.
 */
[[nodiscard]] constexpr char
toHexChar(unsigned int n, bool wantUpperCase = true) noexcept
{
  return wantUpperCase ?
         "0123456789ABCDEF"[n & 0xf] :
         "0123456789abcdef"[n & 0xf];
}

/**
 * @brief Convert the hex character @p c to an integer in [0, 15], or -1 if it's not a hex character.
 */
[[nodiscard]] constexpr int
fromHexChar(char c) noexcept
{
  return (c >= '0' && c <= '9') ? int(c - '0') :
         (c >= 'A' && c <= 'F') ? int(c - 'A' + 10) :
         (c >= 'a' && c <= 'f') ? int(c - 'a' + 10) :
         -1;
}

/**
 * @brief Percent-encode a string.
 *
 * This function will encode all characters that are not one of the following:
 * ALPHA ("a" to "z" and "A" to "Z") / DIGIT (0 to 9) / "-" / "." / "_" / "~"
 *
 * The hex encoding uses the numbers `0-9` and the uppercase letters `A-F`.
 *
 * Examples:
 *
 * @code
 * escape("hello world") == "hello%20world"
 * escape("100%") == "100%25"
 * @endcode
 *
 * @see RFC 3986 section 2
 */
[[nodiscard]] std::string
escape(std::string_view str);

void
escape(std::ostream& os, std::string_view str);

/**
 * @brief Decode a percent-encoded string.
 *
 * When `%` is not followed by two hex characters, the output is not transformed.
 *
 * Examples:
 *
 * @code
 * unescape("hello%20world") == "hello world"
 * unescape("hello%20world%FooBar") == "hello world%FooBar"
 * @endcode
 *
 * @see RFC 3986 section 2
 */
[[nodiscard]] std::string
unescape(std::string_view str);

void
unescape(std::ostream& os, std::string_view str);

} // namespace ndn

#endif // NDN_CXX_UTIL_STRING_HELPER_HPP
