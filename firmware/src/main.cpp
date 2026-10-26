#include <Arduino.h>

namespace {

constexpr unsigned long kHeartbeatIntervalMs = 1000;
unsigned long lastHeartbeatMs = 0;

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("LS PCV gauge firmware started");
}

void loop() {
  const unsigned long now = millis();
  if (now - lastHeartbeatMs >= kHeartbeatIntervalMs) {
    lastHeartbeatMs = now;
    Serial.println("heartbeat");
  }
}
