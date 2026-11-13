#include "board.h"
#include "driver_port.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
static SemaphoreHandle_t i2c_mutex;
static app_result_t result(HAL_StatusTypeDef s) {
    if(s==HAL_OK) return APP_OK;
    if(s==HAL_TIMEOUT) return APP_TIMEOUT;
    if(s==HAL_BUSY) return APP_BUSY;
    return APP_IO_ERROR;
}
app_result_t board_bus_mutex_init(void) {
    if(i2c_mutex) return APP_BUSY;
    i2c_mutex=xSemaphoreCreateMutex(); return i2c_mutex ? APP_OK : APP_NOT_READY;
}
static bool valid(uint8_t a,size_t n,uint32_t t) {
    return a<=0x7f && n>0 && n<=UINT16_MAX && driver_timeout_valid(t);
}
static app_result_t take(uint32_t t) {
    if(!i2c_mutex) return APP_NOT_READY;
    return xSemaphoreTake(i2c_mutex,pdMS_TO_TICKS(t))==pdTRUE ? APP_OK : APP_TIMEOUT;
}
uint32_t port_time_ms(void) { return HAL_GetTick(); }
void port_delay_ms(uint32_t ms) {
    if(xTaskGetSchedulerState()==taskSCHEDULER_RUNNING) vTaskDelay(pdMS_TO_TICKS(ms)+1u);
    else { uint32_t t=HAL_GetTick(); while(HAL_GetTick()-t<ms) {} }
}
void port_delay_us(uint32_t us) {
    /* DWT cycle counter: bounded settling delays only (keypad 5 us). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint32_t start=DWT->CYCCNT, cycles=(SystemCoreClock/1000000u)*us;
    while((uint32_t)(DWT->CYCCNT-start)<cycles) {}
}
void port_rc522_reset(bool high) { HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,high?GPIO_PIN_SET:GPIO_PIN_RESET); }
void port_keypad_row(uint8_t row,bool active) {
    if(row<4) HAL_GPIO_WritePin(GPIOC,(uint16_t)(1u<<(row+4u)),active?GPIO_PIN_RESET:GPIO_PIN_SET);
}
uint8_t port_keypad_columns(void) { return (uint8_t)((~GPIOC->IDR>>8)&0x0fu); }
void port_set_lock_leds(bool unlocked) {
    /* Atomic on GPIOB: only one of the two LEDs is on. */
    GPIOB->BSRR=unlocked ? (GPIO_PIN_10 | ((uint32_t)GPIO_PIN_1<<16))
                        : (GPIO_PIN_1 | ((uint32_t)GPIO_PIN_10<<16));
}
app_result_t port_spi_transfer(const uint8_t *tx,uint8_t *rx,size_t n,uint32_t t) {
    if(!tx||!rx||!n||n>UINT16_MAX||!driver_timeout_valid(t)) return APP_INVALID_INPUT;
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
    HAL_StatusTypeDef s=HAL_SPI_TransmitReceive(&hspi1,(uint8_t *)tx,rx,(uint16_t)n,t);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
    return result(s);
}
app_result_t port_i2c_write(uint8_t a,const uint8_t *d,size_t n,uint32_t t) {
    if(!d||!valid(a,n,t)) return APP_INVALID_INPUT;
    uint32_t start=port_time_ms(); app_result_t s=take(t); if(s!=APP_OK) return s;
    uint32_t left=driver_remaining(start,t);
    s=left ? result(HAL_I2C_Master_Transmit(&hi2c1,(uint16_t)(a<<1),(uint8_t *)d,(uint16_t)n,left)) : APP_TIMEOUT;
    xSemaphoreGive(i2c_mutex); return s;
}
app_result_t port_i2c_write_read(uint8_t a,const uint8_t *p,size_t pn,uint8_t *d,size_t n,uint32_t t) {
    /* STM32 memory-read produces repeated START; supported prefixes 1 or 2. */
    if(!p||!d||!valid(a,n,t)||(pn!=1&&pn!=2)) return APP_INVALID_INPUT;
    uint16_t addr=pn==2 ? (uint16_t)((p[0]<<8)|p[1]) : p[0];
    uint32_t start=port_time_ms(); app_result_t s=take(t); if(s!=APP_OK) return s;
    uint32_t left=driver_remaining(start,t);
    s=left ? result(HAL_I2C_Mem_Read(&hi2c1,(uint16_t)(a<<1),addr,
       pn==2?I2C_MEMADD_SIZE_16BIT:I2C_MEMADD_SIZE_8BIT,d,(uint16_t)n,left)) : APP_TIMEOUT;
    xSemaphoreGive(i2c_mutex); return s;
}
app_result_t port_i2c_probe(uint8_t a,uint32_t t) {
    if(!valid(a,1,t)) return APP_INVALID_INPUT;
    uint32_t start=port_time_ms(); app_result_t s=take(t); if(s!=APP_OK) return s;
    uint32_t left=driver_remaining(start,t);
    s=left ? result(HAL_I2C_IsDeviceReady(&hi2c1,(uint16_t)(a<<1),1,left)) : APP_TIMEOUT;
    xSemaphoreGive(i2c_mutex); return s;
}
