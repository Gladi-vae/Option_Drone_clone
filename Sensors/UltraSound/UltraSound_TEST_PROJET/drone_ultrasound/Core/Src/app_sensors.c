#include "app_sensors.h"
#include <stdio.h>
#include <string.h>

extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;

HCSR04_HandleTypeDef hcsr04;

static void uart_print(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void App_Sensors_Init(void)
{
    HCSR04_Init(&hcsr04,
                TRIG_GPIO_Port, TRIG_Pin,
                ECHO_GPIO_Port, ECHO_Pin,
                &htim5);

    uart_print("HCSR04 init OK\r\n");
}

void App_Sensors_Run(void)
{
    char buffer[128];
    uint32_t distance_cm = 0;

    while (1)
    {
        distance_cm = 0;
        HCSR04_Trigger(&hcsr04);

        uint32_t start = HAL_GetTick();
        while (!HCSR04_ReadDistanceCm(&hcsr04, &distance_cm))
        {
            if ((HAL_GetTick() - start) > 100)
            {
                uart_print("Timeout mesure\r\n");
                break;
            }
        }

        if (distance_cm > 0)
        {
            snprintf(buffer, sizeof(buffer), "Distance: %lu cm\r\n", distance_cm);
            uart_print(buffer);
        }

        HAL_Delay(200);
    }
}
