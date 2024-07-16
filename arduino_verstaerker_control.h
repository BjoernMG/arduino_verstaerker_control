
/***************************************/
// Input select:
/***************************************/
#define OUTPUT_RELAY_INPUT_1 23
#define OUTPUT_RELAY_INPUT_2 25
#define OUTPUT_RELAY_INPUT_3 27
#define OUTPUT_RELAY_INPUT_4 29
#define OUTPUT_RELAY_INPUT_5 31
#define INPUT_RELAY_OFF  LOW
#define INPUT_RELAY_ON  HIGH
#define NUM_INPUTS 5


/***************************************/
// VOL
/***************************************/
/*#define OUTPUT_RELAY_VOLUME_1 43
#define OUTPUT_RELAY_VOLUME_2 45
#define OUTPUT_RELAY_VOLUME_3 47
#define OUTPUT_RELAY_VOLUME_4 49
#define OUTPUT_RELAY_VOLUME_5 51
#define OUTPUT_RELAY_VOLUME_6 53*/

#define OUTPUT_RELAY_VOLUME_1 53
#define OUTPUT_RELAY_VOLUME_2 51
#define OUTPUT_RELAY_VOLUME_3 49
#define OUTPUT_RELAY_VOLUME_4 47
#define OUTPUT_RELAY_VOLUME_5 45
#define OUTPUT_RELAY_VOLUME_6 43
#define NUM_VOL_RELAYS  6

/***************************************/
// BASS
/***************************************/
#define OUTPUT_RELAY_BASS_1 22
#define OUTPUT_RELAY_BASS_2 24
#define OUTPUT_RELAY_BASS_3 26
#define OUTPUT_RELAY_BASS_4 28
#define OUTPUT_RELAY_BASS_5 30
#define NUM_BASS_RELAYS  5

/***************************************/
// TREBLE
/***************************************/
#define OUTPUT_RELAY_TREBLE_1 44
#define OUTPUT_RELAY_TREBLE_2 46
#define OUTPUT_RELAY_TREBLE_3 48
#define OUTPUT_RELAY_TREBLE_4 50
#define OUTPUT_RELAY_TREBLE_5 52
#define NUM_TREBLE_RELAYS  5

/***************************************/
// TONE CONTROL
/***************************************/
#define OUTPUT_RELAY_TONE_CONTROL A15


// EEPROM Adresses:
#define EEPROM_INPUT  24
#define EEPROM_VOLUME  4
#define EEPROM_BASS  8
#define EEPROM_TREBLE  12
#define EEPROM_TONE_CONTROL_ON_OFF  16
#define EEPROM_MUTE_ON_OFF  20
#define EEPROM_HOURS_COUNT  32