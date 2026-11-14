#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H
#include "FreeRTOS.h"
typedef struct TestQueue *QueueHandle_t;
QueueHandle_t xQueueCreate(unsigned length,unsigned size);
void vQueueDelete(QueueHandle_t q);
int xQueueSend(QueueHandle_t q,const void *p,TickType_t ticks);
int xQueueReceive(QueueHandle_t q,void *p,TickType_t ticks);
int xQueueOverwrite(QueueHandle_t q,const void *p);
#endif
