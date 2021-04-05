/* Infrared IR/UART beacons for crawler capture the flag!
   November 2019 -- Emily Lam

   RMT Pulse          -- pin 26 -- A0
   UART Transmitter   -- pin 25 -- A1
   UART Receiver      -- pin 34 -- A2

   Hardware interrupt -- pin 4 -- A5
   ID Indicator       -- pin 13 -- Onboard LED

   Red LED            -- pin 15
   Green LED          -- pin 32
   Blue LED           -- Pin 14

   Features:
   - Sends UART payload -- | START | myColor | myID | Checksum? |
   - Outputs 38kHz using RMT for IR transmission
   - Onboard LED blinks device ID (myID)
   - Button press to change device ID
   - RGB LED shows traffic light state (red, green, yellow)
   - Timer controls traffic light state (r - 10s, g - 10s, y - 2s)
*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// RMT definitions
#define RMT_TX_CHANNEL    1     // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25    // GPIO number for transmitter signal -- A1

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)

//Button defintions
#define GPIO_BUTTON_1      4  //GPIO A5 button 1
#define GPIO_BUTTON_2      36 //GPIO A4

// LED Output pins definitions
#define BLUEPIN   14
#define GREENPIN  32
#define REDPIN    15
#define ONBOARD   13

// Default ID/color
int ID = 0;
// ID = 0 -> no light
// ID = 1 -> red LED
// ID = 2 -> green LED
// ID = 3 -> blue LED
int send_flag = 0;

// Utilities ///////////////////////////////////////////////////////////////////

// Checksum
char genCheckSum(char *p, int len) {
  char temp = 0;
  for (int i = 0; i < len; i++){
    temp = temp^p[i];
  }
  // printf("%X\n",temp);

  return temp;
}
bool checkCheckSum(uint8_t *p, int len) {
  char temp = (char) 0;
  bool isValid;
  for (int i = 0; i < len-1; i++){
    temp = temp^p[i];
  }
  // printf("Check: %02X ", temp);
  if (temp == p[len-1]) {
    isValid = true; }
  else {
    isValid = false; }
  return isValid;
}

// Init Functions //////////////////////////////////////////////////////////////
// RMT tx init
void rmt_tx_init() {
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    // rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    // Carrier Frequency of the IR receiver
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    // Never idle -> aka ontinuous TX of 38kHz pulses
    rmt_tx.tx_config.idle_level = 1;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
    // rmt_config_t config;
    // config.rmt_mode = 0;
    // config.channel = RMT_TX_CHANNEL;
    // config.gpio_num = RMT_TX_GPIO_NUM;
    // config.mem_block_num = 1;
    // config.tx_config.loop_en = 0;
    // config.tx_config.carrier_en = 1;
    // config.tx_config.idle_output_en = true;
    // config.tx_config.idle_level = 1;
    // config.tx_config.carrier_duty_percent = 50;
    // config.tx_config.carrier_freq_hz = 38000;
    // config.tx_config.carrier_level = 1;
    // config.clk_div = 100;
}

// Configure UART
static void uart_init() {
  // Basic configs
  uart_config_t uart_config = {
      .baud_rate = 1200, // Slow BAUD rate
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_RXD_INV);

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs
static void led_init() {
    gpio_pad_select_gpio(BLUEPIN);
    gpio_pad_select_gpio(GREENPIN);
    gpio_pad_select_gpio(REDPIN);
    gpio_pad_select_gpio(ONBOARD);
    gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ONBOARD, GPIO_MODE_OUTPUT);
}


// Button init
static void button_init() {
    gpio_pad_select_gpio(GPIO_BUTTON_1);
    gpio_set_direction(GPIO_BUTTON_1, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(GPIO_BUTTON_2);
    gpio_set_direction(GPIO_BUTTON_2, GPIO_MODE_INPUT);
}

////////////////////////////////////////////////////////////////////////////////

// Tasks ///////////////////////////////////////////////////////////////////////
// Button task -- rotate through myIDs
// void lightled()
// {
//   if (ID == 1) gpio_set_level(REDPIN,1)
// }

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

void button_task_led(){
  while(1) {
    int button_state = gpio_get_level(GPIO_BUTTON_1);
    if (button_state == 0)
    {
      if (ID == 3) ID = 0;
      else ID++;
      printf("went thru button led task\n");
      // lightled();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Send task -- sends LED information payload
void send_task()
{
  static const char *TX_TASK_TAG = "TX_TASK";
  esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
  switch(ID)
  {
    case 0: //no led
      sendData(TX_TASK_TAG, "OFF");
      break;
    case 1: //red led
      sendData(TX_TASK_TAG, "RED");
      break;
    case 2: //green led
      sendData(TX_TASK_TAG, "GREEN");
      break;
    case 3: //blue led
      sendData(TX_TASK_TAG, "BLUE");
      break;
  }
}


// Button 2 task -- send a message through UART every button press
void button_task_IR() {
  while(1)
  {
    int button_state = gpio_get_level(GPIO_BUTTON_2);
    if (button_state == 1)
    {
      send_task();
      printf("went through IR button task\n");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Receives task
void recv_task(){
  // Buffer for input data
  static const char *RX_TASK_TAG = "RX_TASK";
  esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);
  while (1) {
    printf("i am here\n");
    int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 1000 / portTICK_RATE_MS);
    printf("i am here %d\n", len_in);
    if (len_in >0) {
      data_in[len_in] = 0;
      ESP_LOGI(RX_TASK_TAG, "read %d bytes: '%s'", len_in, data_in);
      if (strcmp((const char*) data_in, "RED") == 0) 
      {
        gpio_set_level(REDPIN, 1);
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(BLUEPIN, 0);
      } 
      else if (strcmp((const char*) data_in, "GREEN") == 0) 
      {
        gpio_set_level(REDPIN, 0);
        gpio_set_level(GREENPIN, 1);
        gpio_set_level(BLUEPIN, 0);
      } 
      else if (strcmp((const char*) data_in, "BLUE") == 0) 
      {
        gpio_set_level(REDPIN, 0);
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(BLUEPIN, 1);
      } 
      else if (strcmp((const char*) data_in, "OFF") == 0) 
      {
        gpio_set_level(REDPIN, 0);
        gpio_set_level(GREENPIN, 0);
        gpio_set_level(BLUEPIN, 0);
      }
    }
    else{
      //printf("Nothing received.\n");
    }
  }
  free(data_in);
}


void app_main() {
    // Initialize all the things
    rmt_tx_init();
    uart_init();
    led_init();
    button_init();

    // Create tasks for receive, send, set gpio, and button
    xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, 8, NULL);
    xTaskCreate(button_task_led, "button_task_led", 1024*2, NULL, 6, NULL);
    xTaskCreate(button_task_IR, "button_task_IR", 1024*2, NULL, 7, NULL);
}
