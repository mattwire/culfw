#include "board.h"

#ifdef HAS_ESP8266

#include "esp8266.h"
#include "display.h"
#include "ttydata.h"
#include "ringbuffer.h"
#include "pigator.h"
#include "clock.h"
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <avr/pgmspace.h>

rb_t ESP_Tx_Buffer;
rb_t ESP_Rx_Buffer;

static uint8_t esp_state;
static uint8_t esp_print;
static uint8_t esp_cmd;

void esp8266_init(void) {
  esp_state = 0;
  esp_print = 0;
  esp_cmd   = 1;
}

void toPIMBuffer_P(const char *s) {
  uint8_t c;
  while((c = __LPM(s))) {
    toPIMBuffer(c);
    s++;
  }
}


void esp8266_task(void) {

  // forward NET Buffer to WiFi
  //
  //  if (ESP_Tx_Buffer.nbytes && (esp_ticks < ticks)) {
  if(ESP_Tx_Buffer.nbytes && esp_cmd) {

    toPIMBuffer_P( PSTR( "CM(\"") );
    
    while (ESP_Tx_Buffer.nbytes) {
      uint8_t byte = rb_get(&ESP_Tx_Buffer);
      switch (byte) {
      case 32 ... 126:
	toPIMBuffer( byte );
	break;
      case 10:
	toPIMBuffer_P( PSTR("\\n") );
	break;
      case 13:
	toPIMBuffer_P( PSTR("\\r") );
	break;
      default:
	toPIMBuffer( ' ' );
      }
    }

    toPIMBuffer_P( PSTR( "\")\r\n") );
    esp_cmd = 0;
  }

  // handle command from Wifi start "==>" until "\r\n"
  while (ESP_Rx_Buffer.nbytes) {

    uint8_t byte = rb_get(&ESP_Rx_Buffer);

    // hook to echo ESP answers (just one line) directly
    if (esp_print) {
      if (byte == 10 || byte == 13) {
	DNL();
	esp_print--;
      } else
	DC(byte);
    }

    if (byte == '>') 
      if (!++esp_cmd)
	esp_cmd = 1;

    switch (esp_state) {
    case 0:
      if (byte == '_')
	esp_state++;
      break;
    case 1:
      if (byte == '_') {
	esp_state++;
      } else
	esp_state = 0;
      break;
    case 2:
      if (byte == '|') {
	esp_state++;
      } else
	esp_state = 0;
      break;
    case 3:
      if (byte == 'q') {
	toPIMBuffer_P( PSTR("if con_std then con_std:close() end\r\n"));
	esp_state = 0;
	return;
      }
    default:
      esp_state++;
      rb_put(&TTY_Rx_Buffer, byte);
      if (byte == 10 || byte == 13) {
	input_handle_func(DISPLAY_TCP);
	esp_state = 0;
	return;
      }
    }
  }

}

void esp8266_func(char *in) {
  if (in[1] == 'n') {
    toPIMBuffer_P( PSTR( "=wifi.sta.getip()\r\n" ) );
    esp_print = 1;
  }
  if (in[1] == 'm') {
    toPIMBuffer_P( PSTR( "=wifi.sta.getmac()\r\n" ) );
    esp_print = 1;
  }
  if (in[1] == 'x') {
    toPIMBuffer_P( PSTR( "file.remove(\"config.lc\")\r\n") );
  }
}

#endif

#ifdef _OFF_
https://raw.githubusercontent.com/dannyvai/esp2866_tools/master/nodemcu/scripts/server_hotspot_untill_connect_lua
https://github.com/kubi57/ESP8266
#endif
