#pragma once

#include "gauge_core/zero_reference_capture.hpp"

namespace ls_pcv {
namespace measurement {

/// ESP32 NVS adapter for the small, versioned user-zero record.
/// Only the acquisition task should call this adapter; Preferences access is
/// kept out of the UI and the hardware-independent measurement model.
class PreferencesZeroStore final : public gauge_core::ZeroReferenceStore {
 public:
  /// Load one decoded record or explain whether it is absent, invalid, or unreadable.
  gauge_core::ZeroStoreLoadStatus load(
      gauge_core::ZeroReferenceRecord& record) override;
  /// Persist one complete encoded record; false means storage did not confirm it.
  bool save(const gauge_core::ZeroReferenceRecord& record) override;
};

}  // namespace measurement
}  // namespace ls_pcv
