#pragma once

#include <cstddef>
#include <cstdint>

#include "zero_reference_capture.hpp"

namespace gauge_core {

/// Fixed serialized size of the versioned zero record stored in NVS.
constexpr std::size_t kZeroReferenceRecordBytes = 16;

/// Encode a record with an explicit byte order and checksum, independent of
/// compiler structure padding or the host/ESP32 architecture.
bool encodeZeroReferenceRecord(const ZeroReferenceRecord& record,
                               std::uint8_t* output,
                               std::size_t outputSize);

/// Decode and verify magic, version, reserved bytes, and checksum.
bool decodeZeroReferenceRecord(const std::uint8_t* input,
                               std::size_t inputSize,
                               ZeroReferenceRecord& record);

}  // namespace gauge_core
