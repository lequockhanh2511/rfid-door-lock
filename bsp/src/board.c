#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device_api.h"
SPI_HandleTypeDef hspi1;
I2C_HandleTypeDef hi2c1;
static void gpio_init(GPIO_TypeDef *port, uint32_t pins, uint32_t mode,
                      uint32_t pull, uint32_t alternate) {
    GPIO_InitTypeDef g = {0};
    g.Pin=pins; g.Mode=mode; g.Pull=pull; g.Speed=GPIO_SPEED_FREQ_LOW;
    g.Alternate=alternate; HAL_GPIO_Init(port,&g);
}
app_result_t board_init(void) {
    HAL_Init(); /* HSI 16 MHz: no external oscillator dependency. */
    __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE(); __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
    gpio_init(GPIOB,GPIO_PIN_1|GPIO_PIN_10,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,0);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
    gpio_init(GPIOA,GPIO_PIN_4,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,0);
    gpio_init(GPIOB,GPIO_PIN_0,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,0);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_SET);
    gpio_init(GPIOC,GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,0);
    gpio_init(GPIOC,GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11,GPIO_MODE_INPUT,GPIO_PULLUP,0);
    __HAL_RCC_SPI1_CLK_ENABLE();
    gpio_init(GPIOA,GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_AF_PP,GPIO_NOPULL,GPIO_AF5_SPI1);
    hspi1.Instance=SPI1; hspi1.Init.Mode=SPI_MODE_MASTER;
    hspi1.Init.Direction=SPI_DIRECTION_2LINES; hspi1.Init.DataSize=SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;
    hspi1.Init.NSS=SPI_NSS_SOFT; hspi1.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit=SPI_FIRSTBIT_MSB; hspi1.Init.TIMode=SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;
    if(HAL_SPI_Init(&hspi1)!=HAL_OK) return APP_IO_ERROR;
    __HAL_RCC_I2C1_CLK_ENABLE();
    gpio_init(GPIOB,GPIO_PIN_8|GPIO_PIN_9,GPIO_MODE_AF_OD,GPIO_NOPULL,GPIO_AF4_I2C1);
    hi2c1.Instance=I2C1; hi2c1.Init.ClockSpeed=100000;
    hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2; hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE; hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;
    return HAL_I2C_Init(&hi2c1)==HAL_OK ? APP_OK : APP_IO_ERROR;
}
void board_assert_failed(void) {
    /* Remains safe even if scheduler allocation fails. */
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
    __disable_irq(); for(;;) {}
}
void vApplicationMallocFailedHook(void) { board_assert_failed(); }
void vApplicationStackOverflowHook(TaskHandle_t t, char *name) {
    (void)t; (void)name; board_assert_failed();
}
extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
    HAL_IncTick();
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED) xPortSysTickHandler();
}
