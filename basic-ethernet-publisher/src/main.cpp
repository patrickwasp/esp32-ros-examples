#include <Arduino.h>

#include "micro_ros.h"

void ExampleUpdateCallback(rcl_timer_t *timer, int64_t last_call_time_ns) {
  (void)last_call_time_ns;
  if (timer != NULL) {
    UpdateExampleMessage(millis());
    Serial.println(millis());
  }
}

void setup() {
  Serial.begin(115200);
  InitializeMicroRosEthernetTransport();
  delay(2000);
  Serial.println("Setup complete");
}

void loop() { ManageAgentLifecycle(); }
