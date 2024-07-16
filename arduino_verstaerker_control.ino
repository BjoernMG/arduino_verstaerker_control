/***************************************************************************/
// Mega 2560 Röhrenverstärker Steuerung
// Input Steuerung
//
// Volumen / Klangregler Steuerung
// 
/***************************************************************************/

#include <Encoder.h>    // Verwendung der  Bibliothek 
#include "arduino_verstaerker_control.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "stdint.h"


uint32_t totalMin;
uint32_t startMillis;
uint32_t endMillis;

uint32_t toneControlMillis;
int toneControlEnd = 5000;


byte inputs[5] = {OUTPUT_RELAY_INPUT_1, OUTPUT_RELAY_INPUT_2, OUTPUT_RELAY_INPUT_3, OUTPUT_RELAY_INPUT_4, OUTPUT_RELAY_INPUT_5};
int test = 8;
byte activeInput = 0;
int inputLastEncoderPos = 0;
const int INP_CLK = 61;      // Definition der Pins. CLK an D6, DT an D5. 
const int INP_DT = 60;

/***************************************/
// VOLUME
/***************************************/
//byte volumeRelay[NUM_VOL_RELAYS] = {OUTPUT_RELAY_VOLUME_1, OUTPUT_RELAY_VOLUME_2, OUTPUT_RELAY_VOLUME_3, OUTPUT_RELAY_VOLUME_4, OUTPUT_RELAY_VOLUME_5, OUTPUT_RELAY_VOLUME_6};
byte volumeRelay[NUM_VOL_RELAYS] = {OUTPUT_RELAY_VOLUME_1, OUTPUT_RELAY_VOLUME_2, OUTPUT_RELAY_VOLUME_6, OUTPUT_RELAY_VOLUME_5, OUTPUT_RELAY_VOLUME_4, OUTPUT_RELAY_VOLUME_3};
//byte volumeRelay[NUM_VOL_RELAYS] = {OUTPUT_RELAY_VOLUME_3, OUTPUT_RELAY_VOLUME_4, OUTPUT_RELAY_VOLUME_5, OUTPUT_RELAY_VOLUME_6, OUTPUT_RELAY_VOLUME_2, OUTPUT_RELAY_VOLUME_1};

const int VOL_CLK = 55;      // Definition der Pins. CLK an D6, DT an D5. 
const int VOL_DT = 54;
const int SW = 2;       // Der Switch wird mit Pin D2 Verbunden. ACHTUNG : Verwenden Sie einen interrupt-Pin!
byte volPosition = 0;  // Definition der "alten" Position (Diese fiktive alte Position wird benötigt, damit die aktuelle Position später im seriellen Monitor nur dann angezeigt wird, wenn wir den Rotary Head bewegen)
int volLastEncoderPos = 0;
const int volMin = 0;
const int volMax = 63;
/***************************************/
// BASS
/***************************************/
byte bassRelay[NUM_BASS_RELAYS] = {OUTPUT_RELAY_BASS_1, OUTPUT_RELAY_BASS_2, OUTPUT_RELAY_BASS_3, OUTPUT_RELAY_BASS_4, OUTPUT_RELAY_BASS_5};
const int BA_CLK = 57;      // Definition der Pins. CLK an D6, DT an D5. 
const int BA_DT = 56;
const int bassMin = 1;
const int bassMax = 31;
int bassLastEncoderPos = 0;
byte bassPosition = 0;  // Definition der "alten" Position (Diese fiktive alte Position wird benötigt, damit die aktuelle Position später im seriellen Monitor nur dann angezeigt wird, wenn wir den Rotary Head bewegen)

/***************************************/
// TREBLE
/***************************************/
byte trebleRelay[NUM_TREBLE_RELAYS] = {OUTPUT_RELAY_TREBLE_1, OUTPUT_RELAY_TREBLE_2, OUTPUT_RELAY_TREBLE_3, OUTPUT_RELAY_TREBLE_4, OUTPUT_RELAY_TREBLE_5};
const int TR_CLK = 59;      // Definition der Pins. CLK an D6, DT an D5. 
const int TR_DT = 58;
const int trebleMin = 1;
const int trebleMax = 31;
int trebleLastEncoderPos = 0;
byte treblePosition = 0;  // Definition der "alten" Position (Diese fiktive alte Position wird benötigt, damit die aktuelle Position später im seriellen Monitor nur dann angezeigt wird, wenn wir den Rotary Head bewegen)

Encoder volEncoder(VOL_DT,VOL_CLK);  // An dieser Stelle wird ein neues Encoder Projekt erstellt. Dabei wird die Verbindung über die zuvor definierten Varibalen (DT und CLK) hergestellt.
Encoder bassEncoder(BA_DT,BA_CLK);  // An dieser Stelle wird ein neues Encoder Projekt erstellt. Dabei wird die Verbindung über die zuvor definierten Varibalen (DT und CLK) hergestellt.
Encoder trebleEncoder(TR_DT,TR_CLK);  // An dieser Stelle wird ein neues Encoder Projekt erstellt. Dabei wird die Verbindung über die zuvor definierten Varibalen (DT und CLK) hergestellt.
Encoder inputEncoder(INP_DT,INP_CLK);  // An dieser Stelle wird ein neues Encoder Projekt erstellt. Dabei wird die Verbindung über die zuvor definierten Varibalen (DT und CLK) hergestellt.

#define OUTPUT_RELAY_POWER 16
#define OUTPUT_TONE_CONTROL 14
#define OUTPUT_VOL_UP 12
#define OUTPUT_VOL_DOWN 13

boolean power = true;
boolean mute = false;
boolean toneControl = false;
boolean onBass = false;
boolean onTreble = false;




void setup()   // Beginn des Setups
{
  Serial.begin(115200); 
  while(!Serial) continue;

  Serial1.begin(9600); 
  while(!Serial1) continue;
    
  // init 
  //resetHobsMinutes();
  // Initialize inputs:
  pinMode(OUTPUT_RELAY_INPUT_1, OUTPUT);
  pinMode(OUTPUT_RELAY_INPUT_2, OUTPUT);
  pinMode(OUTPUT_RELAY_INPUT_3, OUTPUT);
  pinMode(OUTPUT_RELAY_INPUT_4, OUTPUT);
  pinMode(OUTPUT_RELAY_INPUT_5, OUTPUT);
  
  pinMode(OUTPUT_RELAY_VOLUME_1, OUTPUT);
  pinMode(OUTPUT_RELAY_VOLUME_2, OUTPUT);
  pinMode(OUTPUT_RELAY_VOLUME_3, OUTPUT);
  pinMode(OUTPUT_RELAY_VOLUME_4, OUTPUT);
  pinMode(OUTPUT_RELAY_VOLUME_5, OUTPUT);
  pinMode(OUTPUT_RELAY_VOLUME_6, OUTPUT);

  pinMode(OUTPUT_RELAY_BASS_1, OUTPUT);
  pinMode(OUTPUT_RELAY_BASS_2, OUTPUT);
  pinMode(OUTPUT_RELAY_BASS_3, OUTPUT);
  pinMode(OUTPUT_RELAY_BASS_4, OUTPUT);
  pinMode(OUTPUT_RELAY_BASS_5, OUTPUT);

  pinMode(OUTPUT_RELAY_TREBLE_1, OUTPUT);
  pinMode(OUTPUT_RELAY_TREBLE_2, OUTPUT);
  pinMode(OUTPUT_RELAY_TREBLE_3, OUTPUT);
  pinMode(OUTPUT_RELAY_TREBLE_4, OUTPUT);
  pinMode(OUTPUT_RELAY_TREBLE_5, OUTPUT);

  getStoredSettings();
  setVolume(volPosition);
  setEncoderValue(&volEncoder, volPosition);

  setBass(bassPosition);
  setEncoderValue(&bassEncoder, bassPosition);
  setTreble(treblePosition);
  setEncoderValue(&trebleEncoder, treblePosition);

  if(mute){
    muteInput();  
    sendSerial("MUTE", "1");
  }
  Serial.print("stored input-> "); 
  Serial.println(activeInput);
  switchInput();
  sendSerial("INPUT", String(activeInput));

  toneControlMillis = 0;
  
  //digitalWrite(OUTPUT_RELAY_INPUT_1, INPUT_RELAY_ON);
  //activeInput = OUTPUT_RELAY_INPUT_1;
  //for(int i = 0; i < 5; i++){
  //  Serial.print("input: ");     
  //  Serial.print(i); 
  //  Serial.print("-> "); 
  //  Serial.println(inputs[i]);
 // }

/*  // bevor volume position gespeichert wird
  // workaround: Volume = 0 bei neustart
  digitalWrite(volumeRelay[0], 1);  //invert state-> volume relay: 0 = on 1 = off
  digitalWrite(volumeRelay[1], 1);
  digitalWrite(volumeRelay[2], 1);
  digitalWrite(volumeRelay[3], 1);
  digitalWrite(volumeRelay[4], 1);
  digitalWrite(volumeRelay[5], 1);
*/

}


void loop()
{
    checkEncoders();
    checkSerialCmd();

    if(toneControlMillis){
      //Serial.print("Timer:");
      // Serial.print(toneControlMillis);
      //Serial.print("   millis:");
      // Serial.println(millis());

      if((toneControlMillis + toneControlEnd)  < millis() ){
        Serial.print("Tone Control timer elapsed");
        onBass = false;
        onTreble = false;
        toneControlMillis = 0;
      }

    }
    
}

void checkSerialCmd(){
  // Check if the other Arduino is transmitting
  if (Serial1.available()) 
  {

    // Allocate the JSON document
    // This one must be bigger than the sender's because it must store the strings
    StaticJsonDocument<300> doc;

    // Read the JSON document from the "link" serial port
    DeserializationError err = deserializeJson(doc, Serial1);

    if (err == DeserializationError::Ok) 
    {
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      //String tmp = JSON.stringify(doc);

      String remoteCmd = doc["REMOTE_CMD"].as<String>();
      Serial.print("RemoteCmd: ");
      Serial.println(remoteCmd);
      if(remoteCmd.equals("POWER_ON")){
        power=true;
        startHobbsMeter();
        digitalWrite(OUTPUT_RELAY_POWER, INPUT_RELAY_ON);
        if(!mute){
          switchInput();
        }
      } else if(remoteCmd.equals("POWER_OFF")){
        power=false;
        muteInput();
        stopHobbsMeter();
        storeSettings();
        digitalWrite(OUTPUT_RELAY_POWER, INPUT_RELAY_OFF);
      }else if((remoteCmd.equals("INPUT_1"))&&(power)){
        if(activeInput != 1){
          activeInput = 1;
          if(!mute){
            switchInput();
            sendSerial("INPUT", "1");
          }
        }
      }else if((remoteCmd.equals("INPUT_2"))&&(power)){
        if(activeInput != 2){
          activeInput = 2;
          if(!mute){
            switchInput();
            sendSerial("INPUT", "2");
          }
        }

      }else if((remoteCmd.equals("INPUT_3"))&&(power)){
        if(activeInput != 3){
          activeInput = 3;
          if(!mute){
            switchInput();
            sendSerial("INPUT", "3");
          }
        }

      }else if((remoteCmd.equals("INPUT_4"))&&(power)){
        if(activeInput != 4){
            activeInput = 4;
            if(!mute){
              switchInput();
              sendSerial("INPUT", "4");
            }
        }
   
      }else if((remoteCmd.equals("INPUT_5"))&&(power)){
        if(activeInput != 5){
            activeInput = 5;
            if(!mute){
              switchInput();
              sendSerial("INPUT", "5");
            }
        }
   
      }else if((remoteCmd.equals("MUTE_ON"))&&(power)){
        mute=true;
        muteInput();
        sendSerial("MUTE", "1");
        
      }else if((remoteCmd.equals("MUTE_OFF"))&&(power)){
        mute=false;
        switchInput();
        sendSerial("MUTE", "0");

      }else if((remoteCmd.equals("TONE_CONTROL_ON"))&&(power)){
        muteInput();
        digitalWrite(OUTPUT_TONE_CONTROL, INPUT_RELAY_ON);
        delay(100);
        switchInput();
        
      }else if((remoteCmd.equals("TONE_CONTROL_OFF"))&&(power)){
        muteInput();
        digitalWrite(OUTPUT_TONE_CONTROL, INPUT_RELAY_OFF);
        delay(100);
        switchInput();
      }else if((remoteCmd.equals("VOLUME_UP"))&&(power)){
        if(onBass){
          if(bassPosition < bassMax){
            bassPosition++;
            setBass(bassPosition);
            setEncoderValue(&bassEncoder, bassPosition);
            toneControlMillis = millis();
          } 
        } else if(onTreble){
          if(treblePosition < trebleMax){
            treblePosition++;
            setTreble(treblePosition);
            setEncoderValue(&trebleEncoder, treblePosition);
            toneControlMillis = millis();
          } 
        } else {
          if(volPosition < volMax){
            volPosition++;
            setVolume(volPosition);
            setEncoderValue(&volEncoder, volPosition);
            
          }
        }

      
      }else if((remoteCmd.equals("VOLUME_DOWN"))&&(power)){
        if(onBass){
          if(bassPosition > bassMin){
            bassPosition--;
            Serial.print("bassPosition:");
            Serial.println(bassPosition);
            setBass(bassPosition);
            setEncoderValue(&bassEncoder, bassPosition);
            toneControlMillis = millis();
          } 
        } else if(onTreble){
          if(treblePosition > trebleMin){
            treblePosition--;
            setTreble(treblePosition);
            setEncoderValue(&trebleEncoder, treblePosition);
            toneControlMillis = millis();
          } 
        } else {
          if(volPosition > volMin){
            volPosition--;
            setVolume(volPosition);
            setEncoderValue(&volEncoder, volPosition);
          }    
        }



  
      }else if((remoteCmd.equals("HOME"))&&(power)){
        Serial.print("BASS enabled");
        toneControlMillis = millis();
        onBass=true;
        onTreble=false;
      }else if((remoteCmd.equals("MENU"))&&(power)){
        Serial.print("TREBLE enabled");
        toneControlMillis = millis();
        onTreble=true;
        onBass=false;
      }
    } 
    else 
    {
      // Print error to the "debug" serial port
      //Serial.print("deserializeJson() returned ");
      //Serial.println(err.c_str());
  
      // Flush all bytes in the "link" serial port buffer
      while (Serial1.available() > 0)
        Serial1.read();
    }
  }
}


void checkEncoders(){
  int position;
    
  // VOlume;
  position = getEncoderValue(&volEncoder);
  if (position != volLastEncoderPos)  // Sollte die neue Position ungleich der alten (-999) sein (und nur dann!!)...
  {
    volLastEncoderPos = position;
    if( position < volMin){
      setEncoderValue(&volEncoder, volMin);
      //volEncoder.write(volMin);
      position = volMin;
    } else if(position > volMax){
      //volEncoder.write(volMax);
      setEncoderValue(&volEncoder, volMax);
      position = volMax;
    }
    volPosition = position; 
    Serial.print("Volume: ");      
    Serial.println(position);      // ...soll die aktuelle Position im seriellen Monitor ausgegeben werden.
    setVolume(volPosition);
    
  }
  // Bass;
  //position = bassEncoder.read();  // Die "neue" Position des Encoders wird definiert. Dabei wird die aktuelle Position des Encoders über die Variable.Befehl() ausgelesen. 
  position = getEncoderValue(&bassEncoder);
  if (position != bassLastEncoderPos)  // Sollte die neue Position ungleich der alten (-999) sein (und nur dann!!)...
  {
    bassLastEncoderPos = position;
    if( position < bassMin){
      setEncoderValue(&bassEncoder, bassMin);
      position = bassMin;
    } else if(position > bassMax){
      setEncoderValue(&bassEncoder, bassMax);
      position = bassMax;
    }
    bassPosition = position; 
    Serial.print("Bass: ");      
    Serial.println(position);      // ...soll die aktuelle Position im seriellen Monitor ausgegeben werden.
    setBass(bassPosition);
  }
  // Treble;
  //position = trebleEncoder.read();  // Die "neue" Position des Encoders wird definiert. Dabei wird die aktuelle Position des Encoders über die Variable.Befehl() ausgelesen. 
  position = getEncoderValue(&trebleEncoder);
  if (position != trebleLastEncoderPos)  // Sollte die neue Position ungleich der alten (-999) sein (und nur dann!!)...
  {
    trebleLastEncoderPos = position;
    if( position < trebleMin){
      setEncoderValue(&trebleEncoder, trebleMin);
      position = trebleMin;
    } else if(position > trebleMax){
      setEncoderValue(&trebleEncoder, trebleMax);
      position = trebleMax;
    }
    treblePosition = position;  
    Serial.print("Treble: ");     
    Serial.println(position);      // ...soll die aktuelle Position im seriellen Monitor ausgegeben werden.
    setTreble(treblePosition);
  }
  // Input:
  position = getEncoderValue(&inputEncoder);  // Die "neue" Position des Encoders wird definiert. Dabei wird die aktuelle Position des Encoders über die Variable.Befehl() ausgelesen. 
  //position = inputEncoder.read();
  if (position != inputLastEncoderPos)  // Sollte die neue Position ungleich der alten (-999) sein (und nur dann!!)...
  {
    Serial.print("position: ");     
    Serial.print(position);
    inputLastEncoderPos = position;
    if( position < 1){
      activeInput = NUM_INPUTS;
      setEncoderValue(&inputEncoder, NUM_INPUTS -1);
    } else if(position > (NUM_INPUTS)){
      activeInput = 1;
      setEncoderValue(&inputEncoder, 1);
    } else {
      activeInput = position;
    }
    Serial.print("->Input: ");     
    Serial.println(activeInput);      // ...soll die aktuelle Position im seriellen Monitor ausgegeben werden.
    switchInput();
    sendSerial("INPUT", String(position));
    
  }
}

void setVolume(int iVolume){
  for (byte i=0; i<NUM_VOL_RELAYS; i++) {
    byte state = bitRead(iVolume, i);
    digitalWrite(volumeRelay[i], !state);  //invert state-> volume relay: 0 = on 1 = off
    Serial.print(state);
  }
  String vol = String(volPosition);
  sendSerial("VOLUME", vol);
}

void setBass(byte iBass){
  Serial.print("setBass:");
  Serial.println(iBass);
  //int iValue = 16+iBass;
  for (byte i=0; i<NUM_BASS_RELAYS; i++) {
    byte state = bitRead(iBass, i);
    digitalWrite(bassRelay[i], !state);  //invert state-> bass relay: 0 = on 1 = off
    Serial.print(state);
  }
  Serial.println("");
  String bass = String(bassPosition);
  sendSerial("BASS", bass);


}

void setTreble(byte iTreble){
  Serial.println("setTreble:");
  //int iValue = 16+iTreble;
  for (byte i=0; i<NUM_TREBLE_RELAYS; i++) {
    byte state = bitRead(iTreble, i);
    digitalWrite(trebleRelay[i], !state);  //invert state-> bass relay: 0 = on 1 = off

    Serial.print(state);
  }
  String treble = String(treblePosition);
  sendSerial("TREBLE", treble);

}

void switchInput(){
    muteInput();
    delay(100);
    //digitalWrite(activeInput, INPUT_RELAY_ON);    
    digitalWrite(inputs[activeInput-1], INPUT_RELAY_ON);
    return;
}

void muteInput(){
    digitalWrite(OUTPUT_RELAY_INPUT_1, INPUT_RELAY_OFF);
    digitalWrite(OUTPUT_RELAY_INPUT_2, INPUT_RELAY_OFF);
    digitalWrite(OUTPUT_RELAY_INPUT_3, INPUT_RELAY_OFF);
    digitalWrite(OUTPUT_RELAY_INPUT_4, INPUT_RELAY_OFF);
    digitalWrite(OUTPUT_RELAY_INPUT_5, INPUT_RELAY_OFF);
    //sendSerial("MUTE", "1");
}

long getEncoderValue(Encoder  * encoder){
  long value = encoder->read();
  value = value / 2;
  return (value);

}

void setEncoderValue(Encoder * encoder, int value){
  value *= 2;
  encoder->write(value);
}


void Interrupt() // Beginn des Interrupts. Wenn der Rotary Knopf betätigt wird, springt das Programm automatisch an diese Stelle. Nachdem...
{
  Serial.println("Switch betaetigt"); //... das Signal ausgegeben wurde, wird das Programm fortgeführt.

}

void sendSerial(String key, String value){
  // Create the JSON document
  StaticJsonDocument<32> doc;
  doc[key] = value;

  // Send the JSON document over the "link" serial port
  serializeJson(doc, Serial1);

}

void storeSettings(){

/*#define EEPROM_INPUT  0
  #define EEPROM_VOLUME  3  volPosition
  #define EEPROM_BASS  6
  #define EEPROM_TREBLE  9
  #define EEPROM_TONE_CONTROL_ON_OFF  12
  #define EEPROM_MUTE_ON_OFF  15
*/
  Serial.print("store activeInput: ");
  Serial.println(activeInput);
  EEPROM.update(EEPROM_INPUT, activeInput);
  Serial.print("store volPosition: ");
  Serial.println(volPosition);
  EEPROM.update(EEPROM_VOLUME, volPosition);
  Serial.print("store bassPosition: ");
  Serial.println(bassPosition);
  EEPROM.update(EEPROM_BASS, bassPosition);
  Serial.print("store treblePosition: ");
  Serial.println(treblePosition);
  EEPROM.update(EEPROM_TREBLE, treblePosition);
  Serial.print("store toneControl: ");
  Serial.println(toneControl);
  EEPROM.update(EEPROM_TONE_CONTROL_ON_OFF, toneControl);
  Serial.print("store mute: ");
  Serial.println(mute);
  EEPROM.update(EEPROM_MUTE_ON_OFF, mute);

}

void getStoredSettings(){

/*#define EEPROM_INPUT  0
  #define EEPROM_VOLUME  3  volPosition
  #define EEPROM_BASS  6
  #define EEPROM_TREBLE  9
  #define EEPROM_TONE_CONTROL_ON_OFF  12
  #define EEPROM_MUTE_ON_OFF  15
*/

  activeInput = EEPROM.read(EEPROM_INPUT);
  Serial.print("read stored activeInput: ");
  Serial.println(activeInput);
  volPosition = EEPROM.read(EEPROM_VOLUME);
  Serial.print("read stored volPosition: ");
  Serial.println(volPosition);
  bassPosition = EEPROM.read(EEPROM_BASS);
  Serial.print("read stored bassPosition: ");
  Serial.println(bassPosition);
  treblePosition = EEPROM.read(EEPROM_TREBLE);
  Serial.print("read stored treblePosition: ");
  Serial.println(treblePosition);
  toneControl = EEPROM.read(EEPROM_TONE_CONTROL_ON_OFF);
  Serial.print("read stored toneControl: ");
  Serial.println(toneControl);
  mute = EEPROM.read(EEPROM_MUTE_ON_OFF);
  Serial.print("read stored mute: ");
  Serial.println(mute);

}

void startHobbsMeter() {                                        // Aufrufen mit Pin Status
  Serial.println("Start hobs meter");
  startMillis = millis();
  Serial.print("startMillis: ");
  Serial.println(startMillis);
}

void stopHobbsMeter() {                                        // Aufrufen mit Pin Status
  uint32_t endMillis = millis();
  Serial.print("endMillis: ");
  Serial.println(endMillis);
  totalMin = (endMillis - startMillis) / 60000;
  Serial.print("Betriebszeit: ");
  Serial.print(totalMin);
  Serial.println(" Minuten");

  uint32_t totalHobsMinutes = readHobsMinutes(EEPROM_HOURS_COUNT);
  Serial.print("totalHobsMinutes (EEPR): ");
  Serial.print(totalHobsMinutes);
  Serial.println(" Minuten");
  totalHobsMinutes += totalMin;
  storeHobsMinutes(totalHobsMinutes, EEPROM_HOURS_COUNT);
  Serial.print("new totalHobsMinutes (EEPR): ");
  Serial.print(totalHobsMinutes);
  Serial.println(" Minuten gespeichert");

}

void storeHobsMinutes(uint32_t lo, int adr) {
  byte by;

  for(int i=0;i< 4;i++) {
    by = (lo >> ((3-i)*8)) & 0x000000ff; 
    EEPROM.write(adr+i, by);
  }
} 

uint32_t readHobsMinutes(int adr) {
  uint32_t lo=0;

  for(int i=0;i< 3;i++){
    lo += EEPROM.read(adr+i);
    lo = lo << 8;
  }
  lo += EEPROM.read(adr+3);
  return lo;
} 

void resetHobsMinutes() {
  //storeHobsMinutes(0, EEPROM_HOURS_COUNT);
  byte by = 0;
  EEPROM.write(EEPROM_HOURS_COUNT, by);
  byte val = EEPROM.read(EEPROM_HOURS_COUNT);
  Serial.print("val: ");
  Serial.println(val);

  EEPROM.write(EEPROM_HOURS_COUNT+1, by);
  val = EEPROM.read(EEPROM_HOURS_COUNT+1);
  Serial.print("val: ");
  Serial.println(val);

  EEPROM.write(EEPROM_HOURS_COUNT+2, by);
  val = EEPROM.read(EEPROM_HOURS_COUNT+2);
  Serial.print("val: ");
  Serial.println(val);

  EEPROM.write(EEPROM_HOURS_COUNT+3, by);
  val = EEPROM.read(EEPROM_HOURS_COUNT+3);
  Serial.print("val: ");
  Serial.println(val);
} 
