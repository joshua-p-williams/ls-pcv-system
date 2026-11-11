#include "preferences_zero_store.hpp"

#include <Preferences.h>

#include "gauge_core/zero_reference_record.hpp"

namespace ls_pcv {
namespace measurement {
namespace {

constexpr char kPreferencesNamespace[] = "lspcv";
constexpr char kZeroRecordKey[] = "zero";

}  // namespace

/// Read and decode the single versioned blob, distinguishing first boot from damage.
gauge_core::ZeroStoreLoadStatus PreferencesZeroStore::load(
    gauge_core::ZeroReferenceRecord& record) {
  Preferences preferences;
  // A read-only open fails when this namespace has never been created, which
  // incorrectly looks like a storage error on the first boot. Open or create
  // the empty namespace read-write, then inspect the key; no value is written
  // during load, and a missing key is correctly reported as first-run default.
  if (!preferences.begin(kPreferencesNamespace, false)) {
    return gauge_core::ZeroStoreLoadStatus::kError;
  }

  const std::size_t storedSize = preferences.getBytesLength(kZeroRecordKey);
  if (storedSize == 0) {
    preferences.end();
    return gauge_core::ZeroStoreLoadStatus::kNotFound;
  }
  if (storedSize != gauge_core::kZeroReferenceRecordBytes) {
    preferences.end();
    return gauge_core::ZeroStoreLoadStatus::kInvalid;
  }

  std::uint8_t bytes[gauge_core::kZeroReferenceRecordBytes]{};
  const std::size_t bytesRead = preferences.getBytes(
      kZeroRecordKey, bytes, sizeof(bytes));
  preferences.end();
  if (bytesRead != sizeof(bytes) ||
      !gauge_core::decodeZeroReferenceRecord(bytes, sizeof(bytes), record)) {
    return gauge_core::ZeroStoreLoadStatus::kInvalid;
  }
  return gauge_core::ZeroStoreLoadStatus::kLoaded;
}

/// Encode before opening NVS and report success only when every byte was stored.
bool PreferencesZeroStore::save(const gauge_core::ZeroReferenceRecord& record) {
  std::uint8_t bytes[gauge_core::kZeroReferenceRecordBytes]{};
  if (!gauge_core::encodeZeroReferenceRecord(record, bytes, sizeof(bytes))) {
    return false;
  }

  Preferences preferences;
  if (!preferences.begin(kPreferencesNamespace, false)) return false;
  const std::size_t bytesWritten = preferences.putBytes(
      kZeroRecordKey, bytes, sizeof(bytes));
  preferences.end();
  return bytesWritten == sizeof(bytes);
}

}  // namespace measurement
}  // namespace ls_pcv
