# esp32-ros-examples

These are some examples of using micro-ROS on ESP32 boards, specifically the [Olimex ESP32-POE-ISO](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE-ISO/open-source-hardware) board.

### Prerequisites
- VSCode with PlatformIO
- Docker


### Running the Serial ROS agent
Before running the agent and flashing the firmware, check the configuration of the USB device in `docker-compose-serial.yaml` to make sure
it matches your device. The default configuration is `/dev/ttyUSB0`.

Run the following docker command to start the ROS agent in serial mode:

```bash
docker compose --file docker-compose-serial.yaml up
```

### Running the UDP ROS agent

Before running the agent and flashing the firmware, check the configuration of the IPs and network mask in `basic-ethernet-publisher/include/config.h` to make sure it matches your network configuration. 

Run the following docker command to start the ROS agent in UDP mode:

```bash
docker compose --file docker-compose-udp.yaml up
``` 
