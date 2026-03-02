#include <BleKeyboard.h>
#define FORCE_SENSOR_PIN1 39
#define FORCE_SENSOR_PIN2 34
#define FORCE_SENSOR_PIN3 35
#define FORCE_SENSOR_PIN4 36

uint8_t a = 0x04;
uint8_t b = 0x05;
uint8_t c = 0x06;
uint8_t d = 0x07;

const int MAX_THRESHOLD = 3600;
const int MIN_THRESHOLD = 4000;
bool isPressed[4] = {false, false, false, false};    

//Set the name of the bluetooth keyboard (that shows up in the bluetooth menu of your device)
BleKeyboard bleKeyboard("ESP_KEYBOARD");
   
void setup() {
  //Start the Serial communication (with the computer at 115200 bits per second)
  Serial.begin(115200);
  //Send this message to the computer
  Serial.println("Starting BLE work!");
  //Begin the BLE keyboard/start advertising the keyboard (so phones can find it)
  bleKeyboard.begin();
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
 
}

void loop() {
  int analogReading1 = analogRead(FORCE_SENSOR_PIN1);
  int analogReading2 = analogRead(FORCE_SENSOR_PIN2);
  int analogReading3 = analogRead(FORCE_SENSOR_PIN3);
  int analogReading4 = analogRead(FORCE_SENSOR_PIN4);
  Serial.print("Baseline Reading1: ");
  Serial.println(analogReading1);

  Serial.print("Baseline Reading2: ");
  Serial.println(analogReading2);

  Serial.print("Baseline Reading3: ");
  Serial.println(analogReading3);

  Serial.print("Baseline Reading4: ");
  Serial.println(analogReading4);



  if (bleKeyboard.isConnected()) {        
    //if the keyboard is connected to a device
    if (analogReading1 < MIN_THRESHOLD && isPressed[0] == false) {
      //Press the 'A' key
      isPressed[0] = true;
      delay(0.01);
    }

    if (analogReading2 < MIN_THRESHOLD && isPressed[1] == false) {

      isPressed[1] = true;
      delay(0.01);
    }

    if (analogReading3 < MIN_THRESHOLD && isPressed[2] == false) {


      isPressed[2] = true;
      delay(0.01);

    }

    if (analogReading4 < MIN_THRESHOLD && isPressed[3] == false) {

      isPressed[3] = true;
      delay(0.01);
    }
   
 
    sendValues(isPressed);
    delay(50);
  }
}

void sendValues(bool isPressed[]) {

  for (int i = 0; i < sizeof(isPressed); i++) {
    if (isPressed[i]) {
      switch (i) {
        case 0:
          bleKeyboard.print("w");
          break;
       
        case 1:
          bleKeyboard.print("a");
          break;

        case 2:
          bleKeyboard.print("s");
          break;

        case 3:
          bleKeyboard.print("d");
          break;
      }
    }
  }
  for (int i = 0; i < sizeof(isPressed); i++)
  {
    isPressed[i] = false;
  }
  bleKeyboard.releaseAll();

}