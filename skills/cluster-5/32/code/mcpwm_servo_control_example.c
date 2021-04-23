// standard library
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

//mcpwm library
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

//adc library for encoder
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

//esp timer
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 700 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2100 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate

//ADC definitions
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   50          //Multisampling
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_5;     //GPIO33 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_10; //10bit width for ez conversion. 
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

//global defintions
int count = 0; //used for counting the number of black-white transitions in encoder
bool one_pulse = true; //used for determining whether or not to increment count in adc_reading

//ADC init 
static void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 27);    //Set GPIO 27 as PWM0A, to which servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 12);    //Set GPIO 12 as PWM0B, to ESC is connected
}

void calibrateESC() {
    printf("Crawler on in 3seconds\n");       
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
    printf("Crawler neutral\n"); 
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1200); // NEUTRAL signal in microseconds
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    printf("Crawler neutral now, calibration done\n");
}

static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

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
    while(1)
    {
        printf("\n STOP TO FORWORD \n");
        for (count_speed = 1400; count_speed < 1700; count_speed +=50)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
        }
        printf("\n FORWARD TO STOP \n");
        for (count_speed = 1700; count_speed > 1200; count_speed -=50)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
    // // reverse twice to trigger reverse motion in buggy
    // printf("First reverse (Depress joystick)\n");
    // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 800);
    // printf("Back to neutral \n");
    // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1200);
    // printf("Second reverse \n");
    // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 800);
    // printf("Back to neutral \n");
    // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1200);

    // //now do reverse spinning
    // printf("\n STOP TO BACKWARD \n");
    // for (count_speed = 1200; count_speed > 800; count_speed -=50)
    // {
    //     mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); 
    // }
    // printf("\n BACKWARD TO STOP \n");
    // for (count_speed = 800; count_speed < 1200; count_speed +=50)
    // {
    //     mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, count_speed);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); 
    // }
    // vTaskDelay(100 / portTICK_PERIOD_MS); 
    // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1400);
    // vTaskDelete(NULL);
}

// void steering_servo(void *arg)
// {
//    int count, angle;
//     while (1) {
//         for (count = 0; count < SERVO_MAX_DEGREE; count++) {
//             angle = servo_per_degree_init(count);
//             mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
//             vTaskDelay(100 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
//         }
//         for (count = SERVO_MAX_DEGREE; count > 0; count--) {
//             angle = servo_per_degree_init(count);
//             mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
//             vTaskDelay(100 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS); 
//     }
// }

void encoder_adc(void* arg)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1
    while (1) 
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        // printf("adc_reading: %d\n", adc_reading); //adc_reading is 1023 when black, below that when white.
        if (adc_reading < 1000 && one_pulse == true)
        {
            //only read one count each transition from black to white. (read at falling edge)
            count++;
            one_pulse = false;
        }
        if (adc_reading >= 1000) one_pulse = true;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void getting_wheel_speed(void *arg)
{
    int time_one_rev, timenow = 0, time_taken;
    float speed;
    // timenow = esp_timer_get_time();//get time in microseconds since boot
    while(1)
    {
        if (count == 7)
        {
            // one rev
            time_one_rev = esp_timer_get_time();
            // time taken to go one rev
            time_taken = time_one_rev - timenow;
            // find speed here:
            speed = (1.0/(time_taken/1000000.0))*(0.2136);
            printf("speed of wheel: %.2f\n",speed);
            //set old time to timenow
            timenow = time_one_rev;
            //reset count
            count = 0;
        }
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void app_main(void)
{
    master_init();
    calibrateESC();

    printf("Testing RC car.......\n");
    // xTaskCreate(steering_servo, "steering_servo", 4096, NULL, 5, NULL);
    xTaskCreate(driving_servo,"driving_servo", 4096, NULL, 5, NULL);
    xTaskCreate(encoder_adc,"encoder_adc", 4096, NULL, 5, NULL);
    xTaskCreate(getting_wheel_speed,"getting_wheel_speed", 4096, NULL, 5, NULL);
}
