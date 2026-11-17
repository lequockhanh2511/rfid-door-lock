#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device_api.h"
SPI_HandleTypeDef hspi1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;
static bool pwm_ready;
static void gpio_init(GPIO_TypeDef *port,uint32_t pins,uint32_t mode,uint32_t pull) {
    GPIO_InitTypeDef g={0};g.Pin=pins;g.Mode=mode;g.Pull=pull;g.Speed=GPIO_SPEED_FREQ_HIGH;HAL_GPIO_Init(port,&g);
}
app_result_t board_init(void) {
    HAL_Init(); /* Reset clock: HSI 8 MHz, APB1/2 div1, no external crystal. */
    RCC_OscInitTypeDef osc={0};osc.OscillatorType=RCC_OSCILLATORTYPE_HSI;osc.HSIState=RCC_HSI_ON;osc.HSICalibrationValue=RCC_HSICALIBRATION_DEFAULT;osc.PLL.PLLState=RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&osc)!=HAL_OK)return APP_IO_ERROR;
    RCC_ClkInitTypeDef clk={0};clk.ClockType=RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;clk.SYSCLKSource=RCC_SYSCLKSOURCE_HSI;clk.AHBCLKDivider=RCC_SYSCLK_DIV1;clk.APB1CLKDivider=RCC_HCLK_DIV1;clk.APB2CLKDivider=RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&clk,FLASH_LATENCY_0)!=HAL_OK)return APP_IO_ERROR;
    __HAL_RCC_AFIO_CLK_ENABLE();__HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOB_CLK_ENABLE();
    /* TIM2 CH1 PA0: 1 MHz counter / 20,000 = 50 Hz. Set closed before enabling. */
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance=TIM2;htim2.Init.Prescaler=7;htim2.Init.CounterMode=TIM_COUNTERMODE_UP;
    htim2.Init.Period=19999;htim2.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;htim2.Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_PWM_Init(&htim2)!=HAL_OK)return APP_IO_ERROR;
    TIM_OC_InitTypeDef oc={0};oc.OCMode=TIM_OCMODE_PWM1;oc.Pulse=APP_SERVO_CLOSED_US;oc.OCPolarity=TIM_OCPOLARITY_HIGH;oc.OCFastMode=TIM_OCFAST_DISABLE;
    if(HAL_TIM_PWM_ConfigChannel(&htim2,&oc,TIM_CHANNEL_1)!=HAL_OK)return APP_IO_ERROR;
    gpio_init(GPIOA,GPIO_PIN_0,GPIO_MODE_AF_PP,GPIO_NOPULL);
    if(HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1)!=HAL_OK)return APP_IO_ERROR;
    pwm_ready=true;
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
    gpio_init(GPIOA,GPIO_PIN_4,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL);gpio_init(GPIOB,GPIO_PIN_0,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_SET);
    gpio_init(GPIOB,GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL);
    gpio_init(GPIOB,GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10,GPIO_MODE_INPUT,GPIO_PULLUP);
    __HAL_RCC_SPI1_CLK_ENABLE();
    gpio_init(GPIOA,GPIO_PIN_5|GPIO_PIN_7,GPIO_MODE_AF_PP,GPIO_NOPULL);
    gpio_init(GPIOA,GPIO_PIN_6,GPIO_MODE_INPUT,GPIO_NOPULL);
    hspi1.Instance=SPI1; hspi1.Init.Mode=SPI_MODE_MASTER;
    hspi1.Init.Direction=SPI_DIRECTION_2LINES; hspi1.Init.DataSize=SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;
    hspi1.Init.NSS=SPI_NSS_SOFT; hspi1.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit=SPI_FIRSTBIT_MSB; hspi1.Init.TIMode=SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;hspi1.Init.CRCPolynomial=7;
    if(HAL_SPI_Init(&hspi1)!=HAL_OK) return APP_IO_ERROR;
    __HAL_RCC_I2C1_CLK_ENABLE();
    gpio_init(GPIOB,GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_AF_OD,GPIO_NOPULL);
    hi2c1.Instance=I2C1; hi2c1.Init.ClockSpeed=100000;
    hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2; hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE; hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;
    return HAL_I2C_Init(&hi2c1)==HAL_OK ? APP_OK : APP_IO_ERROR;
}
void board_assert_failed(void) {
    /* Command closed if PWM is already running. No mechanical feedback. */
    if(pwm_ready){__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,APP_SERVO_CLOSED_US);}
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
