/* servo motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 700 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2100 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate



static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 27);    //Set GPIO 27 as PWM0A, to which servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 12);    //Set GPIO 12 as PWM0B, to ESC is connected
}

void calibrateESC() {
    printf("Crawler on in 3seconds\n");       
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
    printf("Crawler high\n");
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 2100); // HIGH signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Crawler low\n");   
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 700);  // LOW signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Crawler neutral\n"); 
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1400); // NEUTRAL signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Crawler neutral now, calibration done\n");
}
/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

/**
 * @brief Configure MCPWM module
 */
void master_init()
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}

void driving_servo(void *arg)
{
    int count_speed;
    printf("\n STOP TO FORWORD \n");
    for (count_speed = 1400; count_speed < 1700; count_speed +=50)
    {
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    printf("\n FORWARD TO STOP \n");
    for (count_speed = 1700; count_speed > 1300; count_speed -=50)
    {
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    printf("\n STOP TO BACKWARD \n");
    for (count_speed = 1300; count_speed > 1000; count_speed -=50)
    {
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    printf("\n BACKWARD TO STOP \n");
    for (count_speed = 1000; count_speed < 1400; count_speed +=50)
    {
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1400);
    vTaskDelete(NULL);
}

void steering_servo(void *arg)
{
   int count, angle;
    while (1) {
        for (count = 0; count < SERVO_MAX_DEGREE; count++) {
            angle = servo_per_degree_init(count);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
            vTaskDelay(100 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
        for (count = SERVO_MAX_DEGREE; count > 0; count--) {
            angle = servo_per_degree_init(count);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
            vTaskDelay(100 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}

void app_main(void)
{
    master_init();
    calibrateESC();

    printf("Testing RC car.......\n");
    xTaskCreate(steering_servo, "steering_servo", 4096, NULL, 5, NULL);
    xTaskCreate(driving_servo,"driving_servo", 4096, NULL, 5, NULL);
}
