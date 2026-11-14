#include "app_ipc.h"
#include "FreeRTOS.h"
#include "queue.h"
static QueueHandle_t commands,events,ui;
static TickType_t ticks(uint32_t ms){
    uint64_t n=((uint64_t)ms*configTICK_RATE_HZ+999u)/1000u;
    return (TickType_t)(n>=portMAX_DELAY?portMAX_DELAY-1u:n);
}
app_result_t app_ipc_init(void){
    if(commands||events||ui)return APP_BUSY;
    commands=xQueueCreate(APP_COMMAND_QUEUE_DEPTH,sizeof(app_command_t));
    events=xQueueCreate(APP_EVENT_QUEUE_DEPTH,sizeof(app_event_t));
    ui=xQueueCreate(APP_UI_QUEUE_DEPTH,sizeof(app_ui_snapshot_t));
    if(commands&&events&&ui)return APP_OK;
    if(commands)vQueueDelete(commands);
    if(events)vQueueDelete(events);
    if(ui)vQueueDelete(ui);
    commands=NULL;events=NULL;ui=NULL;return APP_NOT_READY;
}
static app_result_t send(QueueHandle_t q,const void *p,uint32_t ms){
    if(!p||ms>INT32_MAX)return APP_INVALID_INPUT;
    if(!q)return APP_NOT_READY;
    return xQueueSend(q,p,ticks(ms))==pdPASS?APP_OK:APP_BUSY;
}
static app_result_t receive(QueueHandle_t q,void *p,uint32_t ms){
    if(!p||ms>INT32_MAX)return APP_INVALID_INPUT;
    if(!q)return APP_NOT_READY;
    return xQueueReceive(q,p,ticks(ms))==pdPASS?APP_OK:(ms?APP_TIMEOUT:APP_NO_EVENT);
}
app_result_t app_ipc_send_command(const app_command_t *m,uint32_t t){return send(commands,m,t);}
app_result_t app_ipc_receive_command(app_command_t *m,uint32_t t){return receive(commands,m,t);}
app_result_t app_ipc_send_event(const app_event_t *m,uint32_t t){return send(events,m,t);}
app_result_t app_ipc_receive_event(app_event_t *m,uint32_t t){return receive(events,m,t);}
app_result_t app_ipc_publish_ui(const app_ui_snapshot_t *v){
    if(!v)return APP_INVALID_INPUT;
    if(!ui)return APP_NOT_READY;
    return xQueueOverwrite(ui,v)==pdPASS?APP_OK:APP_BUSY;
}
app_result_t app_ipc_receive_ui(app_ui_snapshot_t *v,uint32_t t){return receive(ui,v,t);}
