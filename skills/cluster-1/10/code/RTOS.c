
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <math.h>

int flag = 0;
int global_count = 0;

/* Define GPIO pin number for each LED connected */
#define LED1 25
#define LED2 13
#define LED3 12
#define LED4 27
// Define GPIO pin number for push button //
#define BUTTON_GPIO 33

// 14-Segment Display
#define SLAVE_ADDR                         0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value


// Function to initiate i2c -- note the MSB declaration!
static void i2c_example_master_init(){
    // Debug
    printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                              // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
    err = i2c_param_config(i2c_master_port, &conf);           // Configure
    if (err == ESP_OK) {printf("- parameters: ok\n");}

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    // i2c_set_data_mode(i2c_master_port,I2C_DATA_MODE_LSB_FIRST,I2C_DATA_MODE_LSB_FIRST);
    if (err == ESP_OK) {printf("- initialized: yes\n\n");}

    // Dat in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Turn on oscillator for alpha display
int alpha_oscillator() {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

// Set blink rate to off
int no_blink() {
  int ret;
  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
  i2c_master_stop(cmd2);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd2);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

// Set Brightness
int set_brightness_max(uint8_t val) {
  int ret;
  i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
  i2c_master_start(cmd3);
  i2c_master_write_byte(cmd3, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
  i2c_master_stop(cmd3);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd3);
  vTaskDelay(200 / portTICK_RATE_MS);
  return ret;
}

void init(void) {
    gpio_pad_select_gpio(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LED3);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LED4);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);

    //initialize i2c
    i2c_example_master_init();

    //Configure push button
    gpio_pad_select_gpio(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    // gpio_config_t btn_config;
    // btn_config.intr_type = GPIO_INTR_ANYEDGE; 	//Enable interrupt on both rising and falling edges
    // btn_config.mode = GPIO_MODE_INPUT;        	//Set as Input
    // btn_config.pin_bit_mask = (1 << BUTTON_GPIO); //Bitmask
    // btn_config.pull_up_en = GPIO_PULLUP_DISABLE; 	//Disable pullup
    // btn_config.pull_down_en = GPIO_PULLDOWN_ENABLE; //Enable pulldown
    // gpio_config(&btn_config);
   
}

/*reset function takes in all LED GPIO number and turns them off */
void reset(int t1, int t2, int t3, int t4)
{
    gpio_set_level(t1,0);
    gpio_set_level(t2,0);
    gpio_set_level(t3,0);
    gpio_set_level(t4,0);
}


static void interrupt() // interrupt received from button press
{
    flag = flag ^ 1;
}

static void task_1() //count up/down binary LEDs
{
    while(1)
    {
        if (global_count == 16) global_count = 0;
        if (global_count == -1) global_count = 16;
        if (flag == 0)
        {
            // no interrupt pressed, counting up
            gpio_set_level(LED1, (global_count % 2));
            gpio_set_level(LED2, ((global_count / 2) % 2)); 
            gpio_set_level(LED3, ((global_count / 4) % 2)); 
            gpio_set_level(LED4, ((global_count / 8) % 2));     
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            reset(LED1,LED2,LED3,LED4);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            global_count++;
        }
        if (flag == 1)
        {
            //interrupted, counting down
            gpio_set_level(LED1, (global_count % 2));
            gpio_set_level(LED2, ((global_count / 2) % 2)); 
            gpio_set_level(LED3, ((global_count / 4) % 2)); 
            gpio_set_level(LED4, ((global_count / 8) % 2));     
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            reset(LED1,LED2,LED3,LED4);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            global_count--;
        }
    }
}

static void task_2() //turn on alphanumeric display
{
    int ret;
    ret = alpha_oscillator();
    if(ret == ESP_OK) {printf("- oscillator: ok \n");}
    ret = no_blink();
    if(ret == ESP_OK) {printf("- blink: off \n");}
    ret = set_brightness_max(0xF);
    if(ret == ESP_OK) {printf("- brightness: max \n");}

    // Write to characters to buffer
    uint16_t displaybuffer[8];


    // Continually writes the same command
    while (1) {
        if (flag == 0)
        {
            displaybuffer[0] = 0b0000000000000000;  // nth.
            displaybuffer[1] = 0b0000000000111110;  // U.
            displaybuffer[2] = 0b0000000011110011;  // P.
            displaybuffer[3] = 0b0000000000000000;  // nth.
        }
        if (flag == 1)
        {
            displaybuffer[0] = 0b0001001000001111;  // D
            displaybuffer[1] = 0b0000000000111111;  // O
            displaybuffer[2] = 0b0010100000110110;  // W
            displaybuffer[3] = 0b0010000100110110;  // N
        }
        // Send commands characters to display over I2C
        i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
        i2c_master_start(cmd4);
        i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);
        for (uint8_t i=0; i<8; i++) {
            i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
            i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
        }
        i2c_master_stop(cmd4);
        ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd4);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

static void task_3() //switch button interrupt
{
    while(1)
    {
        int button_state = gpio_get_level(BUTTON_GPIO);
        if (button_state == 1)
        {
            interrupt();
        }
        vTaskDelay(100);
    }
}

void app_main() {		// Your main program
    init();
    xTaskCreate(task_1, "task_1", 1024*4, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(task_2, "task_2", 1024*4, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(task_3, "task_3", 1024*4, NULL, configMAX_PRIORITIES-2, NULL);
	}