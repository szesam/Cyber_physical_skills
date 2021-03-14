//Standard C library
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//LEDC library
#include "driver/ledc.h"
#include "esp_err.h"

//Console IO Library
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "sdkconfig.h"

//LEDC definitions only using gpio 26
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (26)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE

#define LEDC_TEST_DUTY         (4000) //50% duty cycle
#define LEDC_TEST_FADE_TIME    (3000)

//declare two structs led_timer_config and led_channel_config
ledc_timer_config_t ledc_timer = {
    // The frequency and the duty resolution are interdependent. 
    // The higher the PWM frequency, the lower duty resolution is available, and vice versa.
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
};
ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
};

void init()
{
    // initialize ledc
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);
    // Initialize fade service.
    ledc_fade_func_install(0);
}
void duty_cycle()
{
    int duty = 0;
    int intensity = 0;
    while (duty < 8001) 
    {
        printf("Current LED intensity: %d\n", intensity);
        ledc_set_duty_and_update(ledc_channel.speed_mode, ledc_channel.channel, duty, 0);
        duty = duty + 1000;
        intensity++;
        vTaskDelay(100/portTICK_PERIOD_MS);
    } 
    duty = 8000;
    intensity = 9;
    while (duty >= 0) 
    {
        printf("Current LED intensity: %d\n", intensity);
        ledc_set_duty_and_update(ledc_channel.speed_mode, ledc_channel.channel, duty, 0);
        duty = duty - 1000;
        intensity--;
        vTaskDelay(100/portTICK_PERIOD_MS);
    } 
}
void ledc()
{
    //just use channel 0 - gpio 26
    int user_intensity;
    int duty; 
     /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );
    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    char buff[10];
    while(1)
    {
        //ask for user input led intensity here
        printf("Enter LED intensity from 0 to 9 (0 is lowest): \n");
        printf("Or cycle through intensity by typing 'cycle' \n");
        gets(buff);
        if(strcmp(buff,"cycle") == 0)
        {
            //do duty cycle increase then decrease
            duty_cycle();
        }
        else 
        {
            //set user input led intensity here (convert user input intensity to duty cycle)
            // user input is from 1 to 9
            // duty cycle is from 0 to 90%
            user_intensity = atoi(buff);
            duty = (LEDC_TEST_DUTY *2) * ((float)user_intensity/10.0);
            ledc_set_duty_and_update(ledc_channel.speed_mode, ledc_channel.channel, duty, 0);
            printf("LED duty set to: %d, intensity level: %d\n", duty, user_intensity);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main()
{
    // initializations
    init();
    // task
    xTaskCreate(ledc, "ledc", 4096, NULL, 5, NULL);
}