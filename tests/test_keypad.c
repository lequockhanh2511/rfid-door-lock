#include <assert.h>
#include <stdio.h>
#include "keypad.h"
#include "device_api.h"
#include "driver_port.h"
static bool active[4];static uint16_t raw;
void port_keypad_row(uint8_t r,bool a){assert(r<4);active[r]=a;unsigned n=0;for(int i=0;i<4;i++)n+=active[i];assert(n<=1);}
void port_delay_us(uint32_t us){assert(us==5);}
uint8_t port_keypad_columns(void){for(int i=0;i<4;i++)if(active[i])return (uint8_t)(raw>>(4*i));return 0;}
int main(void){
 keypad_state_t s;char key;uint16_t m;const char expected[]="123A456B789C*0#D";
 for(unsigned i=0;i<16;i++){
  raw=(uint16_t)(1u<<i);assert(keypad_read_matrix(&m)==APP_OK&&m==raw);
  keypad_state_init(&s,0);assert(keypad_update(&s,m,0,&key)==APP_NO_EVENT);
  assert(keypad_update(&s,m,29,&key)==APP_NO_EVENT);
  assert(keypad_update(&s,m,30,&key)==APP_OK&&key==expected[i]);
  assert(keypad_update(&s,m,1000,&key)==APP_NO_EVENT);
  for(int j=0;j<4;j++)assert(!active[j]);
 }
 puts("PASS keypad scan all 16 positions, settle, debounce and hold suppression");
 keypad_state_init(&s,0);keypad_update(&s,1,0,&key);keypad_update(&s,0,5,&key);
 keypad_update(&s,1,10,&key);assert(keypad_update(&s,1,39,&key)==APP_NO_EVENT);
 assert(keypad_update(&s,1,40,&key)==APP_OK);
 keypad_update(&s,3,41,&key);keypad_update(&s,2,50,&key);
 assert(keypad_update(&s,2,100,&key)==APP_NO_EVENT);
 keypad_update(&s,0,101,&key);keypad_update(&s,0,131,&key);
 keypad_update(&s,2,132,&key);assert(keypad_update(&s,2,162,&key)==APP_OK&&key=='2');
 puts("PASS keypad bouncing, chord quarantine until full stable release");
 keypad_state_init(&s,UINT32_MAX-10);keypad_update(&s,1,UINT32_MAX-10,&key);
 assert(keypad_update(&s,1,19,&key)==APP_OK&&key=='1');
 assert(keypad_update(NULL,1,0,&key)==APP_INVALID_INPUT);
 puts("PASS keypad wraparound and null argument");
}
