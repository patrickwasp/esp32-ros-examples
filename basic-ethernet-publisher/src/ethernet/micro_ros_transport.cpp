
#include "ethernet/micro_ros_transport.h"

void EthernetEventCallback(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("micro-ros-1");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.print("Ethernet Connected, MAC: ");
      Serial.println(ETH.macAddress());
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet Disconnected");
      break;
    case ARDUINO_EVENT_ETH_STOP:
      break;
    default:
      break;
  }
}

extern "C" {

static WiFiUDP udp_client;

bool platformio_transport_open(struct uxrCustomTransport *transport) {
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;
  return true == udp_client.begin(locator->port);
}

bool platformio_transport_close(struct uxrCustomTransport *transport) {
  udp_client.stop();
  return true;
}

size_t platformio_transport_write(struct uxrCustomTransport *transport,
                                  const uint8_t *buf, size_t len,
                                  uint8_t *errcode) {
  (void)errcode;
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;

  size_t sent = 0;
  if (true == udp_client.beginPacket(locator->address, locator->port)) {
    sent = udp_client.write(buf, len);
    sent = true == udp_client.endPacket() ? sent : 0;
  }

  udp_client.flush();

  return sent;
}

size_t platformio_transport_read(struct uxrCustomTransport *transport,
                                 uint8_t *buf, size_t len, int timeout,
                                 uint8_t *errcode) {
  (void)errcode;

  int64_t start_time = uxr_millis();

  while ((uxr_millis() - start_time) < ((int64_t)timeout) &&
         udp_client.parsePacket() == 0) {
    delay(1);
  }
  size_t available = 0;
  if (udp_client.available()) {
    available = udp_client.read(buf, len);
  }

  return (available > 0) ? available : 0;
}
}
