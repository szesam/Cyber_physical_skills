
#include <stdio.h>
#include "driver/i2c.h"

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

// LIDAR addresses
#define SLAVE_ADDR                         0x62 //7-bit slave address with default value
#define REGISTER_READ                      0X00 // register to write to initiate ranging
#define MEASURE_VALUE                      0x04 // Value to initiate ranging
#define HIGH_LOW                           0x8f //for multi-byte read


// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init(){
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
  if (err == ESP_OK) {printf("- initialized: yes\n");}

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return err;
}

// Utility function to scan for i2c device
static void i2c_scanner() {
  int32_t scanTimeout = 1000;
  printf("\n>> I2C scanning ..."  "\n");
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++) {
    // printf("0x%X%s",i,"\n");
    if (testConnection(i, scanTimeout) == ESP_OK) {
      printf( "- Device found at address: 0x%X%s", i, "\n");
      count++;
    }
  }
  if (count == 0) {printf("- No I2C devices found!" "\n");}
}

////////////////////////////////////////////////////////////////////////////////

// LIDAR V3 Functions ///////////////////////////////////////////////////////////

// Write one byte to register
void writeRegister(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //slave address followed by write bit
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    //register pointer sent
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //data sent
    i2c_master_write_byte(cmd, data, ACK_CHECK_DIS);
    //stop command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

// Read register
uint16_t readRegister(uint8_t reg) {
    int ret;
    uint8_t value;
    uint8_t value2;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //sensor address, write, ack
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    //register address ack
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //stop
    i2c_master_stop(cmd);
    //stop condition 
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);

    //start command for cmd2
    i2c_master_start(cmd2);
    //sensor address, read, ack
    i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
    //data out high byte #1
    i2c_master_read_byte(cmd2, &value, ACK_CHECK_DIS);
    //data out low byte #2
    i2c_master_read_byte(cmd2, &value2, ACK_CHECK_DIS);
    //stop
    i2c_master_stop(cmd2);
    //ret is device ID
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
    //delete two i2c comm packets
    i2c_cmd_link_delete(cmd2);
    i2c_cmd_link_delete(cmd);
    printf("value: %d, value2: %d \n", value, value2);
    return (uint16_t)(value<<8|value2);
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
    uint8_t val1;
    uint8_t val2;
    val1 = readRegister(reg);
    if (reg == 41) {
        val2 = 0;
    } else {
        val2 = readRegister(reg+1);
    }
    return (((int16_t)val2 << 8) | val1);
}


////////////////////////////////////////////////////////////////////////////////

// Task to continuously poll acceleration and calculate roll and pitch
static void test_lidar() {
  printf("\n>> Polling Lidar\n");
  while (1) {
    uint8_t reg = REGISTER_READ;
    uint8_t data = MEASURE_VALUE;
    writeRegister(reg,data);
    // continously read register 0x01 until first bit (LSB) goes 0
    int compare = 1;
    while(compare)
    {
      uint8_t reading = readRegister(0x01);
      compare = reading&(1<<7);
      // printf("Reading: %d\n", reading);
      vTaskDelay(5);
    }
    uint16_t distance = readRegister(HIGH_LOW);
    printf("Distance: %d\n", distance);
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

void app_main() {

  // Routine
  i2c_master_init();
  i2c_scanner();

  // Create task to poll ADXL343
  xTaskCreate(test_lidar,"test_lidar", 4096, NULL, 5, NULL);
}