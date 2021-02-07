/* Description

Created by Samuel Sze
With reference to blink ESP-IDF example project
*/

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Define GPIO pin number for each LED connected */
#define LED1 25
#define LED2 13
#define LED3 12
#define LED4 27

/*blink_LED function turns on the LED */
void blink_LED(int LED)
{
    gpio_set_level(LED, 1);
}

/* dectobin function takes in an integer decimal number and returns an array of binary numbers in reversed order 
ie. 1 --> 1000 */
int * dectobin(int n)
{
    static int array[32];
    int i = 0;
    while (n>0)
    {
        array[i] = n%2;
        n = n/2;
        i++;
    }
    return array;
}
/*reset function takes in all LED GPIO number and turns them off */
void reset(int t1, int t2, int t3, int t4)
{
    gpio_set_level(t1,0);
    gpio_set_level(t2,0);
    gpio_set_level(t3,0);
    gpio_set_level(t4,0);
}
void app_main(void)
{   
    int * binary_array;
    /* Setting up all the LED pins */
    gpio_pad_select_gpio(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(LED3);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(LED4);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);
    /* infinite loop*/
    while(1)
    {
        for (int i = 0; i <= pow(2,4); i++) /* there are four pins, each pin has 0, 1 state. */
        {
            binary_array = dectobin(i);
            /* 1st LED */
            if (*(binary_array + 0) == 1) blink_LED(LED1);
            /*2nd LED*/
            if (*(binary_array + 1) == 1) blink_LED(LED2);
            /*3rd LED*/
            if (*(binary_array + 2) == 1) blink_LED(LED3);
            /*4th LED*/
            if (*(binary_array + 3) == 1) blink_LED(LED4); 
            /* add in some delay */       
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            reset(LED1,LED2,LED3,LED4);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
