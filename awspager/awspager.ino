/* 
 *  Eseye AWS pager demo for Eseye anynet-secure click boards
 *  
 *  Simple demo to subscribe to topic "msgout" and display
 *  messages received on the OLED display.
 * 
 *  This uses the eseyeaws library to handle AT command generation.
 * 
 *  Connect OLED B click module to left-side of UNO click
 *  shield. Connect 2G, 3G or 2G/4G click to right-side.
 * 
 *  Using the AWS console publish a message to 'msgout/<thingname>'.
 *  This message should appear on the OLED display.
 *  Message can be JSON or plain text and the sketch below will
 *  display the json "message" element OR plaintext received.
 * 
 */

/* Use the Serial interface (hardware UART) */
#define ATSerial Serial

#include "eseyeaws.h"

/* Create instance of eseyeaws lib */
eseyeAWS myAWS(&ATSerial);

/* Callback function for received subscribed message */
void msgcb(uint8_t *data, uint8_t length){
  /* If it's json it'll be in the "message" parameter */
  char *msgptr = strstr((char *)data, "message");
  if(msgptr != NULL){
    msgptr += 11;
    char *msgend = strchr(msgptr, '"');
    if(msgend != NULL)
      *msgend = 0;
  }else{
    /* If not json we just display the whole message */
    msgptr = (char *)data;
  }
  /* Print message to oled display */
  OLED_Puts(0, 3, "              ");
  OLED_Puts(0, 3, msgptr);
}


void setup() {
  ATSerial.begin(9600);

  myAWS.init(NULL);

  /* Fire up the oled display */
  oled_init();

  delay(2000);
  OLED_Clear();
  OLED_SetScale(1, 1);
  OLED_Puts(0, 0, "Click demo");
  OLED_Puts(0,1,"pager v0.9");

  /* Give the click board time to boot */
  delay(5000);
  
  myAWS.sendAT("ATE=0\r\n");
  /* Ensure pubidx 0 is not registered */
  myAWS.pubunreg(0);

}

boolean subscribed = false;
int msgsub;

/* Poll eseyeaws library for received messages */
void loop() {
    
  myAWS.poll();

  if(!subscribed){
      /* Subscribe to "msgout" and pass received messages to msgcb() */
      msgsub = myAWS.subscribe((char *)"msgout", msgcb);
      subscribed = true;
  }
  
}

