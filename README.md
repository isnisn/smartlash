# Building a Smart Lashing Detection System

**Name:** Andreas Nilsson  
**Student Credentials:** an224qi
**Time to setup:** ~1 hour

## Short Project Overview
This project involves assembling and configuring a system that uses load cells to detect whether lashing is executed correctly and remains secure throughout a shipping journey.

## Objective
* Ensuring the proper lashing of containers onboard ships is crucial for safety and preventing cargo damage.
* This project aims to alert the crew if the lashing slackens or fails, ensuring timely corrective measures.
* By monitoring the tension on container lashings continuously, we can reduce the risk of accidents and improve shipping safety protocols.

## Material

### List of Material
Here are the key components required to build this device:

1. **RS PRO Load Cell, 300kg Range, Compression, Tension Measure**
    - Quantity: 1
    - Price: ~€160
    - Shop: [RS-Online](https://se.rs-online.com/web/p/strain-gauges/2042768)
    - [Datasheet](https://docs.rs-online.com/b2dd/A700000007176393.pdf)

2. **Load Cell 300 kg for Tension and Compression, IP68, Stainless**
    - Quantity: 1
    - Price: ~€298
    - Shop: [Vetek](https://www.vetek.com/en/article/load-cell-300-kg-for-tension-and-compression-ip68-stainless)
    - [Datasheet](https://www.vetek.com/en/dynamics/WebFiles/document/71853b56-87c4-411f-a51d-bdafb98a6e72/Datasheet_101BS_V2.pdf)

3. **Articulated Rod-End Ball Joint for S-Load Cell M12x1.75**
    - Quantity: 2
    - Price: ~€28
    - Shop: [Vetek](https://www.vetek.com/en/article/articulated-rod-end-ball-joint-for-s-load-cell-m12x1-75)
    - [Datasheet](https://www.vetek.com/en/dynamics/WebFiles/document/27fe49d1-d544-4471-baf0-c7c44f18191e/Datasheet_RBJ_V1.pdf)

4. **HX711 Load Cell Amplifier**
    - Quantity: 1
    - Price: ~€12
    - Shop: [Electrokit](https://www.electrokit.com/forstarkare-for-lastceller-hx711)
    - [Datasheet](https://www.electrokit.com/upload/product/41016/41016232/hx711_english.pdf)

**Total Cost:** ~€526

### What the Different Components Do
- **Load Cell:** Measures tension or compression force.
- **HX711 Load Cell Amplifier:** Amplifies the small analog signal from the load cell and converts it to a digital signal.
- **Articulated Rod-End Ball Joint:** Provides flexible connection points for the load cell to the lashing system.


### Putting Everything Together

### Circuit Diagram and Wiring
![diagram](doc/2.png)

Connect everything on a breadboard.

1. **Connect the Load Cell to the HX711 Amplifier**:
    - Red (E+): Connect to E+ on HX711.
    - Black (E-): Connect to E- on HX711.
    - White (A-): Connect to A- on HX711.
    - Green (A+): Connect to A+ on HX711.

2. **Connect the HX711 to the Heltec ESP32 V2**:
    - VCC: Connect to the 3.3V pin on the ESP32.
    - GND: Connect to a GND pin on the ESP32.
    - DT (Data): Connect to GPIO 12 on the ESP32.
    - SCK (Clock): Connect to GPIO 13 on the ESP32.

---
## Steps to Configure HX711 and LoRaWAN

**First choose correct pins for SPI, byteorder and TX interval(in seconds):**
```c
#define TTN_PIN_SPI_SCLK 5
#define TTN_PIN_SPI_MOSI 27
#define TTN_PIN_SPI_MISO 19
#define TTN_PIN_NSS 18
#define TTN_PIN_RXTX TTN_NOT_CONNECTED
#define TTN_PIN_RST 14
#define TTN_PIN_DIO0 26
#define TTN_PIN_DIO1 35

#define TX_INTERVAL 30

#define DOUT_PIN 12
#define PD_SCK_PIN 13
#define LED_PIN 25
#define __BYTE_ORDER LITTLE_ENDIAN

```
---

### Chosen IDE and Steps

**IDE Used:** Free of choice, I used Vim
**Code Uploading:** Using `idf.py`
**Steps:**
1.
    - Clone the ESP-IDF repository:
        ```bash
        git clone --recursive https://github.com/espressif/esp-idf.git
        cd esp-idf
        ```
    - Install the ESP-IDF tools:
        ```bash
        ./install.sh
        ```
    - Set up the environment variables:
        ```bash
        . ./export.sh
        ```
    - Connect the Heltec ESP32 to your computer via USB
    - cd to this projects root-folder
    - Run `idf.py set-target esp32` to set the target to ESP32 (if not already set).
    - Run `idf.py build` to compile the code.
    - Run `idf.py flash` to upload the code to the ESP32.
    - Run `idf.py monitor` to view the output from the ESP32 serial monitor.

---


### Platform Setup
I am using Chirpstack and a VPS hosted on Linode running Ubuntu.

- [Chirpstack](https://console.helium-iot.xyz/)
- [Linode](https://cloud.linode.com/)

I chose these platforms because I already had them, and they could be scaled significantly.

---

## The Code
### Setup and Helium. 
In the app_main function, we first call the setup function to initialize various components such as GPIO pins, SPI bus, TTN/Helium settings, and the HX711 device. This initialization process sets up the necessary configurations and prepares the device for operation. 

#### Data Shifting Function Selection

The setup function begins by selecting the appropriate shift_data function based on the system's byte order (endian format). This is done using __BYTE_ORDER, which checks if the system is Big Endian or Little Endian and sets shift_data_func.

#### GPIO Configuration

The function configures the LED_PIN and another pin (26) as outputs. These pins are then set to a low level (turned off if active low).

#### TTN/Helium Initialization

- The GPIO ISR handler service is initialized, which provides routines to manage GPIO interrupt service routines.
- The NVS (Non-Volatile Storage) flash is initialized. This allows for saving and restoring keys.
- The SPI bus is initialized with the given pin configuration for MISO, MOSI, and SCLK.
- TTN (Using Helium, but lib is called TTN) is initialized and configured with the necessary pins for the SX127x LoRa module.
- The device is provisioned with the DevEUI, AppEUI, and AppKey. The line that provisions the device can be commented out after the first successful run because the data is saved in NVS.
- The function enables Adaptive Data Rate (ADR), sets the data rate, and sets the maximum transmit power.

#### HX711 Initialization
The HX711 load cell amplifier is initialized for reading data from the load cell.

#### Deep Sleep Resume Check
The code checks if the device has resumed from deep sleep using ttn_resume_after_deep_sleep(). If true, it logs that resumption has occurred.

#### TTN Joining
If the device has not resumed from deep sleep, it attempts to join the TTN network. If the join is successful, it logs this; otherwise, it logs an error and exits the function.

#### Starting HX711 Read
It logs that reading from the HX711 sensor is starting. The read_from_hx711 function is called with the appropriate data shifting function. This reads the data from the HX711 sensor and applies the endian-specific byte shifting.

#### Sending Messages
The sendMessages function is called to send the read data over the network.

#### Preparing for Deep Sleep
The function logs that the device is preparing for deep sleep.
It waits for TTN communication to become idle and prepares the TTN component for deep sleep.
The timer wake-up interval is set to TX_INTERVAL seconds.

#### Powering Down HX711
The HX711 is powered down, and the function logs whether the operation was successful or not.

#### Delay and Enter Deep Sleep
The code delays for one second before logging that it is entering deep sleep for the specified interval.
The device then enters deep sleep mode using esp_deep_sleep_start(), conserving power until the next scheduled wake-up.

### Sending the payload
```c
/** 
 * This function shifts the bits of a 32-bit signed integer and stores each byte into an array,
 * prepping it for being sent over LoRaWAN, depending on your choice of the __BYTE_ORDER macro.
 *
 *      - During each iteration:
 *          - The dereferenced value of `data` is bitwise right-shifted by `(i * 8)` bits. 
 *            This operation effectively moves the byte for the current loop iteration to the rightmost position.
 *
 *          - The result is then bitwise ANDed with `0xFF` to isolate the least significant byte.
 *
 *          - The isolated byte is then stored in the `msgData` array at index `i`.
 *          This results in filling the `msgData` array with individual bytes from the input integer.
 */
static void shift_data_le(const int32_t *data) {
  for (int i = 0; i < sizeof(msgData); ++i) {
    msgData[i] = ((int32_t)*data >> (i * 8)) & 0xFF;
  }
}

static void shift_data_be(const int32_t *data) {
  for (int i = 0; i < sizeof(msgData); ++i) {
    msgData[sizeof(msgData) - 1 - i] = ((int32_t)*data >> (i * 8)) & 0xFF;
}

/**
* The function pointer points to either of the functions depending on chosen byte order.
*/
typedef void (*shift_data_func_t)(const int32_t *);
...

shift_data_func_t shift_data_func;

switch (__BYTE_ORDER) {
case BIG_ENDIAN:
  shift_data_func = shift_data_be;
  break;
case LITTLE_ENDIAN:
  shift_data_func = shift_data_le;
}
...
read_from_hx711(shift_data_func);
...
sendMessages();
```

---

### Transmitting the Data / Connectivity
### Data Transmission

The data is sent every 15 minutes (can be changed in code) over LoRaWAN using the Helium network and Chirpstack. LoRaWAN is best suited here because of the low amount of data being sent and the improved battery life due to not using Wi-Fi. The data is transmitted over LoRaWAN (routed in the Helium network) to Chirpstack.

#### Simple Socket with HTTP
Using a simple Python script that listens for data sent from Chirpstack (via HTTP) by posting to the socket on my VPS. The server then uses a library called InfluxDBClient to insert a Point object in the Influx database.

#### Battery Life and Range
The battery life is significantly better with LoRaWAN than traditional Wi-Fi. The range is also better considering that it might be installed on a boat/ship which could be very long, and the signal is being transmitted through thick walls. Wi-Fi might end up short here; a simple Dragino or RAK gateway installed on the boat/ship would do the job. LoRaWAN radio signals also utilize sub-GHz bandwidth, allowing better penetration through different materials.

---

## Design Choices
**LoRaWAN:** Chosen for its long range and low power consumption, suitable for maritime environments.
**MQTT:** Efficient for low-bandwidth communications.

---

## Presenting the Data
### Dashboard
#### Dashboard built using InfluxDB's own dashboard tool
![dashboard](doc/3.png)

### Data Retention
The data is put into buckets with a retention period of 30 days.
I chose InfluxDB because it's a time-series database. I could have chosen MongoDB or other time-series databases, but I find InfluxDB the easiest to work with.


With this guide, you should be able to build and monitor a smart lashing system for onboard ship containers, ensuring cargo safety during transit.
```
