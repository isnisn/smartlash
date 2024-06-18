# Building a Smart Lashing Detection System

**Name:** Andreas Nilsson  
**Student Credentials:** an224qi

## Short Project Overview
This project involves assembling and configuring a system that uses load cells to detect whether lashing is executed correctly and remains secure throughout a shipping journey. The project will take approximately 8-10 hours to complete.

## Objective
* **Why you chose this project:** Ensuring the proper lashing of containers onboard ships is crucial for safety and preventing cargo damage.
* **What purpose does it serve:** This project aims to alert the crew if the lashing slackens or fails, ensuring timely corrective measures.
* **What insights you think it will give:** By monitoring the tension on container lashings continuously, we can reduce the risk of accidents and improve shipping safety protocols.

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

## Computer Setup

### Chosen IDE and Steps
1. **IDE Used:** Vim
2. **Code Uploading:** Via idf.py
3. **Steps:**
    - Install Arduino IDE.
    - Install the HX711 library: Go to **Sketch** > **Include Library** > **Manage Libraries**, and search for HX711, then install.
    - Connect the ESP32 to your computer via USB.
    - Write and upload the code to the ESP32.

## Putting Everything Together

### Circuit Diagram and Wiring
1. **Connect the Load Cell to the HX711 Amplifier**:
    - Red (E+): Connect to E+ on HX711.
    - Black (E-): Connect to E- on HX711.
    - White (A-): Connect to A- on HX711.
    - Green (A+): Connect to A+ on HX711.

2. **Connect the HX711 to the Heltec ESP32 V2**:
    - VCC: Connect to the 3.3V pin on the ESP32.
    - GND: Connect to a GND pin on the ESP32.
    - DT (Data): Connect to GPIO 16 on the ESP32.
    - SCK (Clock): Connect to GPIO 4 on the ESP32.

## Platform Setup
Im using Chirpstack and a VPS hosted on Linode running Ubuntu.
https://console.helium-iot.xyz/
https://cloud.linode.com/

I choose this platforms because I already had them. It could be scaled quite big.

## The Code

```cpp
/** 
 * This function shifts the bits of an integer and stores each byte into an array. Prepping it for being sent over LoRaWAN.
 *
 * The function `shift_data` takes a pointer to a 32-bit signed integer (`int32_t`) and shifts
 * the bits to separate each byte. It stores each resulting byte into the `msgData` array.

 *      - During each iteration:
 *          - The dereferenced value of `data` is bitwise right-shifted by `(i * 8)` bits. 
 *            This operation effectively moves the byte for the current loop iteration to the rightmost position.
 *
 *          - The result is then bitwise ANDed with `0xFF` to isolate the least significant byte.
 *
 *          - The isolated byte is then stored in the `msgData` array at index `i`.
 *          This results in filling the `msgData` array with individual bytes from the input integer.
 */
static void shift_data(const int32_t *data)
{
    for (int i = 0; i < sizeof(msgData); ++i)
    {
        msgData[i] = ((int32_t)*data >> (i * 8)) & 0xFF;
    }
}

```

## Explanation
The setup initializes the serial communication and HX711.
The loop reads and prints the weight data from the load cell every second.

## Transmitting the Data / Connectivity
### Data Transmission
How often is the data sent?

Which wireless protocols did you use (WiFi, LoRa, etc …)?

Which transport protocols were used (MQTT, webhook, etc …)

*Elaborate on the design choices regarding data transmission and wireless protocols. That is how your choices affect the device range and battery consumption.

The data is sent every 15 minutes over LoRaWAN using Helium network and Chirpstack. LoRaWAN is best suited here imo because of the low amount of data being sent, also the battery life is increasing due to not using wifi. The data is transmitted over LoRaWAN(routed in the Helium network) to Chirpstack. 

#### Simple socket with HTTP
Im using a simple python script that listens on data being sent from a Chirpstack(by HTTP) by posting to the socket on my VPS. The server then uses a library for Python called InfluxDBClient, that insert a Point-object in the influx database.

#### Battery life and range
Well, the battery life is way better with LoRaWAN than traditional wifi. The range is also better considering the might be installed on a boat/ship that could be very long, and the signal is being transmitted through thick walls. Wifi might end up short here, a simple Dragino or RAK gateway installed on the boat/ship would to the job. LoRaWAN radio signals also utilizes sub-ghz bandwidth allowing better penetration through different materials.

## Design Choices
LoRaWAN: Chosen for its long range and low power consumption, suitable for maritime environments.
MQTT: Efficient for low-bandwidth communications.

## Presenting the Data
### Dashboard
#### Dashboard built using InfluxDB own dashboard tool
![dashboard](https://hackmd.io/_uploads/B1Nt5ykHC.png)

### How often is data saved in the database.
The data is put into buckets with a retention period of 30 days.
I choose influxdb because its a time-series database. I could have choosen mongodb or other time-series but I find influxdb easiest to work with.

## Dashboard and Database
Dashboard: Built using InfluxDB V2.
Data Preservation: Data stored in a InfluxDB instance to be preserved over long durations.


Sample Dashboard
ALT
Image failed to load

Note: Replace with an actual image

Data Automation
Set triggers for alerts when the lashing force drops below a threshold.
Finalizing the Design
Results and Reflections
The final project achieved its goal of detecting lashing tension. Future improvements could include better shielding for noise reduction and enhanced calibration routines.

ALT
Image failed to load

Note: Replace with an actual image

Video Presentation
A video presentation link here.

With this guide, you should be able to build and monitor a smart lashing system for onboard ship containers, ensuring cargo safety during transit.

