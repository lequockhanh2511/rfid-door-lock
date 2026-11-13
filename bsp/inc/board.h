#ifndef BOARD_H
#define BOARD_H
#include "stm32f4xx_hal.h"
#include "app_types.h"
app_result_t board_init(void);
app_result_t board_bus_mutex_init(void);
extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c1;
#endif
