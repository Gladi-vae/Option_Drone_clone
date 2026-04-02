#include "hcsr04.h"

static void delay_us(HCSR04_HandleTypeDef *sensor, uint32_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(sensor->htim_us);
    while ((__HAL_TIM_GET_COUNTER(sensor->htim_us) - start) < us)
    {
    }
}

void HCSR04_Init(HCSR04_HandleTypeDef *sensor,
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin,
                 TIM_HandleTypeDef *htim_us)
{
    sensor->trig_port = trig_port;
    sensor->trig_pin = trig_pin;
    sensor->echo_port = echo_port;
    sensor->echo_pin = echo_pin;
    sensor->htim_us = htim_us;

    sensor->waiting_falling = 0;
    sensor->t_rising = 0;
    sensor->t_falling = 0;
    sensor->pulse_us = 0;
    sensor->distance_cm = 0;
    sensor->measurement_ready = 0;
    sensor->measurement_in_progress = 0;
    sensor->timeout = 0;

    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef HCSR04_Trigger(HCSR04_HandleTypeDef *sensor)
{
    sensor->measurement_ready = 0;
    sensor->measurement_in_progress = 1;
    sensor->waiting_falling = 0;
    sensor->timeout = 0;

    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_RESET);
    delay_us(sensor, 2);

    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_SET);
    delay_us(sensor, 10);
    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_RESET);

    return HAL_OK;
}

void HCSR04_EchoCallback(HCSR04_HandleTypeDef *sensor)
{
    uint32_t now = __HAL_TIM_GET_COUNTER(sensor->htim_us);

    if (HAL_GPIO_ReadPin(sensor->echo_port, sensor->echo_pin) == GPIO_PIN_SET)
    {
        sensor->t_rising = now;
        sensor->waiting_falling = 1;
    }
    else
    {
        if (sensor->waiting_falling)
        {
            sensor->t_falling = now;
            sensor->pulse_us = sensor->t_falling - sensor->t_rising;
            sensor->distance_cm = sensor->pulse_us / 58U;
            sensor->measurement_ready = 1;
            sensor->measurement_in_progress = 0;
            sensor->waiting_falling = 0;
        }
    }
}

uint8_t HCSR04_ReadDistanceCm(HCSR04_HandleTypeDef *sensor, uint32_t *distance_cm)
{
    if (sensor->measurement_ready)
    {
        *distance_cm = sensor->distance_cm;
        sensor->measurement_ready = 0;
        return 1;
    }

    return 0;
}
