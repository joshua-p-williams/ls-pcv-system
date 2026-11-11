#include "gauge_core/zero_reference_record.hpp"

namespace gauge_core {
namespace {

constexpr std::uint32_t kRecordMagic = 0x5A50534Cu;  // "LSPZ" in little-endian bytes.

/// Encode an unsigned integer explicitly so persisted records have stable bytes.
void writeU32(std::uint8_t* output, std::uint32_t value) {
  output[0] = static_cast<std::uint8_t>(value);
  output[1] = static_cast<std::uint8_t>(value >> 8u);
  output[2] = static_cast<std::uint8_t>(value >> 16u);
  output[3] = static_cast<std::uint8_t>(value >> 24u);
}

/// Decode a little-endian word without relying on alignment or host endianness.
std::uint32_t readU32(const std::uint8_t* input) {
  return static_cast<std::uint32_t>(input[0]) |
      (static_cast<std::uint32_t>(input[1]) << 8u) |
      (static_cast<std::uint32_t>(input[2]) << 16u) |
      (static_cast<std::uint32_t>(input[3]) << 24u);
}

/// CRC-32 detects incomplete/corrupted NVS bytes before interpreting a zero.
std::uint32_t checksum(const std::uint8_t* bytes, std::size_t length) {
  std::uint32_t crc = 0xFFFFFFFFu;
  for (std::size_t index = 0; index < length; ++index) {
    crc ^= bytes[index];
    for (std::uint8_t bit = 0; bit < 8; ++bit) {
      const std::uint32_t mask = 0u - (crc & 1u);
      crc = (crc >> 1u) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

}  // namespace

/// Serialize only the current schema into a deterministic 16-byte record.
bool encodeZeroReferenceRecord(const ZeroReferenceRecord& record,
                               std::uint8_t* output,
                               std::size_t outputSize) {
  if (output == nullptr || outputSize < kZeroReferenceRecordBytes ||
      record.version != ZeroReferenceRecord::kCurrentVersion) {
    return false;
  }
  writeU32(output, kRecordMagic);
  output[4] = static_cast<std::uint8_t>(record.version);
  output[5] = static_cast<std::uint8_t>(record.version >> 8u);
  output[6] = 0;
  output[7] = 0;
  writeU32(output + 8, record.zeroVoltageMicrovolts);
  writeU32(output + 12, checksum(output, 12));
  return true;
}

/// Reject wrong-size, unknown-version, modified, or malformed serialized data.
bool decodeZeroReferenceRecord(const std::uint8_t* input,
                               std::size_t inputSize,
                               ZeroReferenceRecord& record) {
  if (input == nullptr || inputSize != kZeroReferenceRecordBytes ||
      readU32(input) != kRecordMagic || input[6] != 0 || input[7] != 0 ||
      readU32(input + 12) != checksum(input, 12)) {
    return false;
  }
  const std::uint16_t version = static_cast<std::uint16_t>(input[4]) |
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(input[5]) << 8u);
  if (version != ZeroReferenceRecord::kCurrentVersion) return false;
  record.version = version;
  record.zeroVoltageMicrovolts = readU32(input + 8);
  return true;
}

}  // namespace gauge_core
