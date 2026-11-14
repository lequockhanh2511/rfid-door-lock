#include "app_ipc.h"
#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct TestQueue {unsigned length,size,count,head;unsigned char *data;};
static int creations,fail_at,alive;static TickType_t last_ticks;
QueueHandle_t xQueueCreate(unsigned len,unsigned size){
    if(++creations==fail_at)return NULL;
    QueueHandle_t q=calloc(1,sizeof *q);assert(q);q->length=len;q->size=size;q->data=calloc(len,size);assert(q->data);alive++;return q;
}
void vQueueDelete(QueueHandle_t q){free(q->data);free(q);alive--;}
int xQueueSend(QueueHandle_t q,const void *p,TickType_t t){
    last_ticks=t;if(q->count==q->length)return 0;
    memcpy(q->data+((q->head+q->count)%q->length)*q->size,p,q->size);q->count++;return pdPASS;
}
int xQueueReceive(QueueHandle_t q,void *p,TickType_t t){
    last_ticks=t;if(!q->count)return 0;
    memcpy(p,q->data+q->head*q->size,q->size);q->head=(q->head+1)%q->length;q->count--;return pdPASS;
}
int xQueueOverwrite(QueueHandle_t q,const void *p){assert(q->length==1);memcpy(q->data,p,q->size);q->count=1;return pdPASS;}
int main(void){
    app_command_t cmd={0},got={0};assert(app_ipc_send_command(&cmd,0)==APP_NOT_READY);
    for(int fail=1;fail<=3;fail++){creations=0;fail_at=fail;assert(app_ipc_init()==APP_NOT_READY&&alive==0);}
    fail_at=0;assert(app_ipc_init()==APP_OK&&alive==3);assert(app_ipc_init()==APP_BUSY&&alive==3);
    assert(app_ipc_receive_command(&got,0)==APP_NO_EVENT);
    assert(app_ipc_receive_command(&got,INT32_MAX)==APP_TIMEOUT&&last_ticks==INT32_MAX);
    assert(app_ipc_send_command(NULL,0)==APP_INVALID_INPUT);
    assert(app_ipc_send_command(&cmd,UINT32_MAX)==APP_INVALID_INPUT);
    for(unsigned i=0;i<APP_COMMAND_QUEUE_DEPTH;i++){cmd.request_id=i+1;assert(app_ipc_send_command(&cmd,0)==APP_OK);}
    cmd.request_id=99;assert(app_ipc_send_command(&cmd,0)==APP_BUSY);
    for(unsigned i=0;i<APP_COMMAND_QUEUE_DEPTH;i++){assert(app_ipc_receive_command(&got,0)==APP_OK&&got.request_id==i+1);}
    app_event_t e={0},out;
    for(unsigned i=0;i<APP_EVENT_QUEUE_DEPTH;i++){e.request_id=i+1;assert(app_ipc_send_event(&e,0)==APP_OK);}
    assert(app_ipc_send_event(&e,0)==APP_BUSY);
    for(unsigned i=0;i<APP_EVENT_QUEUE_DEPTH;i++)assert(app_ipc_receive_event(&out,0)==APP_OK&&out.request_id==i+1);
    app_ui_snapshot_t v={0},ui={0};v.card_id=1;assert(app_ipc_publish_ui(&v)==APP_OK);v.card_id=2;assert(app_ipc_publish_ui(&v)==APP_OK);
    assert(app_ipc_receive_ui(&ui,0)==APP_OK&&ui.card_id==2);assert(app_ipc_receive_ui(&ui,0)==APP_NO_EVENT&&ui.card_id==2);
    puts("PASS IPC wrappers with queue model: allocation cleanup, copy/FIFO, BUSY, finite timeouts, UI latest-value overwrite");
}
