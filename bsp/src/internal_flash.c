#include "board.h"
#include "internal_flash.h"
#include "config_store.h"
#include <string.h>
/* Genuine STM32F103C8: 64 KiB, reserve final 2 x 1 KiB in linker. */
#define CONFIG_FLASH_BASE 0x0800F800u
static bool valid(uint8_t bank,size_t off,size_t len){return bank<2u&&off<=CONFIG_BANK_STRIDE&&len<=CONFIG_BANK_STRIDE-off;}
static uint32_t address(uint8_t bank,size_t off){return CONFIG_FLASH_BASE+(uint32_t)bank*CONFIG_BANK_STRIDE+(uint32_t)off;}
app_result_t flash_bank_read(uint8_t bank,size_t off,uint8_t *data,size_t len){
    if(!data||!len||!valid(bank,off,len))return APP_INVALID_INPUT;
    memcpy(data,(const void *)(uintptr_t)address(bank,off),len);return APP_OK;
}
app_result_t flash_bank_erase(uint8_t bank){
    if(bank>=2u)return APP_INVALID_INPUT;
    if(HAL_FLASH_Unlock()!=HAL_OK)return APP_IO_ERROR;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
    FLASH_EraseInitTypeDef erase={0};erase.TypeErase=FLASH_TYPEERASE_PAGES;erase.PageAddress=address(bank,0);erase.NbPages=1;
    uint32_t page_error=0;HAL_StatusTypeDef st=HAL_FLASHEx_Erase(&erase,&page_error);
    HAL_StatusTypeDef lk=HAL_FLASH_Lock();return st==HAL_OK&&lk==HAL_OK?APP_OK:APP_IO_ERROR;
}
app_result_t flash_bank_program(uint8_t bank,size_t off,const uint8_t *data,size_t len){
    if(!data||!len||((off|len)&1u)||!valid(bank,off,len))return APP_INVALID_INPUT;
    if(HAL_FLASH_Unlock()!=HAL_OK)return APP_IO_ERROR;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
    app_result_t r=APP_OK;
    for(size_t i=0;i<len;i+=2){uint32_t at=address(bank,off+i);uint16_t value=(uint16_t)(data[i]|(uint16_t)data[i+1]<<8);
        if(value==0xffffu){if(*(volatile const uint16_t *)(uintptr_t)at!=0xffffu){r=APP_IO_ERROR;break;}continue;}
        if(*(volatile const uint16_t *)(uintptr_t)at!=0xffffu||HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,at,value)!=HAL_OK||*(volatile const uint16_t *)(uintptr_t)at!=value){r=APP_IO_ERROR;break;}}
    if(HAL_FLASH_Lock()!=HAL_OK)r=APP_IO_ERROR;
    return r;
}
