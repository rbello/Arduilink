//============================================================================
// Name        : NanoWattWatcher.ino
// Author      : R. BELLO <https://github.com/rbello>
// Version     : 1.0
// Copyright   : Creative Commons (by)
// Description : A small device that captures the power consumption and climatic conditions and send them by 433 Mhz radio.
//============================================================================

#include <Arduilink.h>
#include <dht.h>
#include <RCSwitch.h>

// Sensor DHT
#define PIN_DHT 9
#define DHT_UPDATE_DELAY 302000 // 5 min + 2 sec 

// Power consumption sensor
#define PIN_PHOTO_CELL A0
#define FLASH_DELTA_SENSIBILITY 200
#define FLASH_DELTA_DELAY 90
#define POWER_UPDATE_DELAY 60000 // W.min

// Sensor RF 433
#define PIN_433_EMITTER 11 // Emitter pin

// ## -- End of configuration -- ##

// Sensor RF 433Mhz
RCSwitch rf433write = RCSwitch();

// Arduilink
Arduilink lnk = Arduilink(2);

// DHT Sensor
dht DHT;

// Send to RF 433Mhz emitter
void sendRf433(unsigned long data) {
  rf433write.send(data, 32);
}

void setup()
{
  // Add sensors
  lnk.addSensor(1, S_INFO | S_HIT, "Compteur EDF", "Wh/min")->verbose = true;
  lnk.addSensor(2, S_INFO, "Temperature", "C")->verbose = true;
  lnk.addSensor(3, S_INFO, "Humidite", "%")->verbose = true;
  
  // Enable RF 433
  rf433write.enableTransmit(PIN_433_EMITTER);
  rf433write.setPulseLength(320);
  rf433write.setProtocol(2);
  rf433write.setRepeatTransmit(5);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  lnk.init();
}

void loop()
{

  // Time stamp for power counter sampling
  unsigned long lastPowerHit = 0;
  unsigned long wattPerMinute = 0;
  unsigned long lastPowerConsumptionUpdate = 0;
  
  // Light measure
  int lightValue = 0;
  
  // Time stamp for DHT sensor update
  unsigned long lastDHTupdate = 0;
  
  // Current values for DHT sensor
  double currentTemperature = 0;
  double currentHumidity = 0;
 
  while (1)
  {
    
    // Ligh sensor
    if (millis() - lastPowerHit > FLASH_DELTA_DELAY) {
      long delta = lightValue - analogRead(PIN_PHOTO_CELL);
      if (delta > FLASH_DELTA_SENSIBILITY) {
        wattPerMinute++;
      }
      lastPowerHit = millis();
      lightValue = analogRead(PIN_PHOTO_CELL);
    }

    // Power consumption sensor update
    if (millis() - lastPowerConsumptionUpdate > POWER_UPDATE_DELAY) {
      lastPowerConsumptionUpdate = millis();
      lnk.setValue(1, wattPerMinute);
      //Serial.print("W/min = "); Serial.println(lnk.getEncoded32(1));
      sendRf433(lnk.getEncoded32(1));
      wattPerMinute = 0;
    }

    // DHT update
    if (millis() - lastDHTupdate > DHT_UPDATE_DELAY) {
      lastDHTupdate = millis();
      int chk = DHT.read11(PIN_DHT);
      if (chk != DHTLIB_OK) {
        continue; // Error
      }
      lnk.setValue(2, currentTemperature = DHT.temperature);
      lnk.setValue(3, currentHumidity = DHT.humidity);
      //Serial.print("T = "); Serial.println(lnk.getEncoded32(2));
      //Serial.print("H = "); Serial.println(lnk.getEncoded32(3));
      sendRf433(lnk.getEncoded32(2));
      sendRf433(lnk.getEncoded32(3));
    }

    // Handle inputs from Serial
    if (Serial.available() > 0)
    {
      unsigned long data = String(Serial.readString()).toInt();
      if (data > 0)
      {
        sendRf433(data);
      }
    }
    
  }
  
}
