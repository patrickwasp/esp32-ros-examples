#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <inttypes.h>

constexpr int8_t kDomainId = 8;
constexpr char kNodeName[] = "example_micro_ros_node";
constexpr char kNamespace[] = "example_namespace";

constexpr uint32_t kSerialBaudRate = 115200;

constexpr uint16_t kPublisherPeriodMs = 1000;  // 1Hz
constexpr uint16_t kUpdatePeriodMs = 500;      // 2Hz
constexpr uint8_t kNumberOfHandles = 2;        // # subscriptions + # timers

constexpr uint8_t kStringLength = 140;

#endif /* CONFIG_H_ */
