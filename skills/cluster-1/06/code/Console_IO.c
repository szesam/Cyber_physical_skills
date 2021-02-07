#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"	// This is associated with VFS -- virtual file system interface and abstraction -- see the docs
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

#define LED1 13
void blink_LED(int LED)
{
    gpio_set_level(LED, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(LED, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
void echo_hex_mode()
{
    int userint = 0;
    char userchar;  
    while(1)
    {
        printf("Enter an integer: \n");
        if(scanf("%d",&userint) == 1) 
            printf("Hex: %x\n",userint);
        else if(scanf("%c",&userchar) == 1 && userchar == 's')
        {
            printf("toggle mode\n");
            printf("Read: ");
            return;
        }
        else 
        {
            while ((getchar()) != '\n'); 
            continue;
        }
    }
    return;
}
void echo_mode()
{
    char buf[100];
    while(1)
    {
        gets(buf);
        if (buf[0] != '\0') 
        {
            printf("echo: %s\n", buf);
        }
        if (buf[0] == 's' && strlen(buf) == 1)
        {
            printf("echo dec to hex mode\n");
            echo_hex_mode();
            return;
        }
    }
    return;
}
void app_main()
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    gpio_pad_select_gpio(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    uint8_t ch;
    printf("toggle mode \n");
    printf("Read: ");
    while(1) 
    {
        
        ch = getchar();
        if(ch!=0xFF)
        {
            switch(ch)
            {
                case 't':
                    putchar(ch);
                    printf("\nRead: ");
                    blink_LED(LED1);
                    break;
                case 's':
                    putchar(ch);
                    printf("\necho mode\n");
                    echo_mode();
                    break;
            }
        }
    }
}