/**
 * Code for reading data from a HX711 load cell amplifier and sending it to
 * Helium(Chirpstack) using LoRaWAN. author: @isnisn (an224qi@student.lnu.se)
 */
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hx711.h>

#include "ttn.h"

// NOTE:
// The LoRaWAN frequency and the radio chip must be configured by running
// 'idf.py menuconfig'. Go to Components / The Things Network, select the
// appropriate values and save.

// Copy the below hex strings from the TTN console (Applications > Your
// application > End devices > Your device > Activation information)

// AppEUI (sometimes called JoinEUI)
const char *appEui = "05c2660247e3113f";
// DevEUI
const char *devEui = "c496422c5c2a7f78";
// AppKey
const char *appKey = "27e52edcd7015d465b955173ac8eb150";

// Pins and other resources
#define TTN_SPI_HOST SPI2_HOST
#define TTN_SPI_DMA_CHAN SPI_DMA_DISABLED
#define TTN_PIN_SPI_SCLK 5
#define TTN_PIN_SPI_MOSI 27
#define TTN_PIN_SPI_MISO 19
#define TTN_PIN_NSS 18
#define TTN_PIN_RXTX TTN_NOT_CONNECTED
#define TTN_PIN_RST 14
#define TTN_PIN_DIO0 26
#define TTN_PIN_DIO1 35
#define TX_INTERVAL 30
#define LED_PIN 25

// Data to be sent
static uint8_t msgData[4];

// Tags for logging
static const char *TAG_LORA = "LoRaWAN:";
static const char *TAG_HX711 = "HX711:";
static const char *TAG_MAIN = "Main:";

// HX711 device
hx711_t dev = {.dout = 12, .pd_sck = 13, .gain = HX711_GAIN_A_64};

// Function prototypes
static void shift_data(const int32_t *data);

void sendMessages(void *pvParameter) {
  printf("Sending message...\n");
  ttn_response_code_t res =
      ttn_transmit_message(msgData, sizeof(msgData), 1, false);
  if (res == TTN_SUCCESSFUL_TRANSMISSION) {
    ESP_LOGI(TAG_LORA, "Message sent.");
  } else {
    ESP_LOGE(TAG_LORA, "Transmission failed.\n");
  }

  return;
}

void read_from_hx711(void *pvParameters) {
  int32_t data;
  size_t times = 10;

  // read from device
  esp_err_t r = hx711_wait(&dev, 500);
  if (r != ESP_OK) {
    ESP_LOGE(TAG_HX711, "Device not found: %d (%s)\n", r, esp_err_to_name(r));
    return;
  }

  r = hx711_read_average(&dev, times, &data);
  if (r != ESP_OK) {
    ESP_LOGE(TAG_HX711, "Could not read data: %d (%s)\n", r,
             esp_err_to_name(r));
    return;
  }

  ESP_LOGI(TAG_HX711, "Raw data: %" PRIi32, data);

  // Sleep for 10 seconds
  vTaskDelay(pdMS_TO_TICKS(1000));

  shift_data(&data);
  sendMessages(NULL);
}

// Shift the data to the array
static void shift_data(const int32_t *data) {
  for (int i = 0; i < sizeof(msgData); ++i) {
    msgData[i] = ((int32_t)*data >> (i * 8)) & 0xFF;
  }
}

void setup() {

  /***** GPIO *****/

  // Configure LED_PIN as an output
  esp_rom_gpio_pad_select_gpio(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(26, GPIO_MODE_OUTPUT);

  // Set the LED to low level (turn it off if it's active low)
  gpio_set_level(LED_PIN, 0);
  gpio_set_level(26, 0);

  /**** TTN/Helium *****/

  esp_err_t err;

  // Initialize the GPIO ISR handler service
  err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  ESP_ERROR_CHECK(err);

  // Initialize the NVS (non-volatile storage) for saving and restoring the keys
  err = nvs_flash_init();
  ESP_ERROR_CHECK(err);

  // Initialize SPI bus
  spi_bus_config_t spi_bus_config = {.miso_io_num = TTN_PIN_SPI_MISO,
                                     .mosi_io_num = TTN_PIN_SPI_MOSI,
                                     .sclk_io_num = TTN_PIN_SPI_SCLK,
                                     .quadwp_io_num = -1,
                                     .quadhd_io_num = -1};
  err = spi_bus_initialize(TTN_SPI_HOST, &spi_bus_config, TTN_SPI_DMA_CHAN);
  ESP_ERROR_CHECK(err);

  // Initialize TTN (Or Helium)
  ttn_init();

  // Configure the SX127x pins
  ttn_configure_pins(TTN_SPI_HOST, TTN_PIN_NSS, TTN_PIN_RXTX, TTN_PIN_RST,
                     TTN_PIN_DIO0, TTN_PIN_DIO1);

  // The below line can be commented after the first run as the data is saved in
  // NVS
  ttn_provision(devEui, appEui, appKey);

  ttn_set_adr_enabled(true);
  ttn_set_data_rate(TTN_DR_EU868_SF9);
  ttn_set_max_tx_pow(14);

  /***** HX711 *****/

  // initialize device
  ESP_ERROR_CHECK(hx711_init(&dev));
}

void app_main(void) {

  setup();

  if (ttn_resume_after_deep_sleep()) {
    ESP_LOGI(TAG_MAIN, "Resumed from deep sleep.");
  } else {

    ESP_LOGI(TAG_LORA, "Joining...");
    if (ttn_join()) {
      ESP_LOGI(TAG_LORA, "Joined.");
    } else {
      ESP_LOGE(TAG_LORA, "Join failed. Goodbye.");
      return;
    }
  }

  ESP_LOGI(TAG_HX711, "Starting read from HX711...");

  // xTaskCreate(read_from_hx711, "test", configMINIMAL_STACK_SIZE * 5, NULL, 5,
  // NULL);

  read_from_hx711(NULL);

  ESP_LOGI(TAG_MAIN, "Preparing for deepsleep...");

  // Wait until TTN communication is idle and save state
  ttn_wait_for_idle();
  ttn_prepare_for_deep_sleep();

  // Schedule wake up
  esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000LL);

  // Power down the HX711
  esp_err_t pdhx = hx711_power_down(&dev, true);
  if (pdhx == ESP_OK) {
    ESP_LOGI(TAG_HX711, "Powered down HX711\n");
  } else {
    ESP_LOGE(TAG_HX711, "Could not power down HX711: %d (%s)\n", pdhx,
             esp_err_to_name(pdhx));
  }

  // Delar for a second
  vTaskDelay(pdMS_TO_TICKS(1000));

  ESP_LOGI(TAG_MAIN, "Entering deep sleep for %d seconds", TX_INTERVAL);

  esp_deep_sleep_start();
}
