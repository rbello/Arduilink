#include <Arduilink.h>
#include <dht.h>
#include <RCSwitch.h>

// Sonde DHT
#define PIN_DHT 9
#define DHT_UPDATE_DELAY 60000

// Capteur de luminosité (impulsions conso EDF)
#define PIN_PHOTO_CELL A0
#define FLASH_DELTA_SENSIBILITY 250
#define FLASH_DELTA_DELAY 100

// Capteur RF 433
#define PIN_433_EMITTER 11 // Pin de l'emetteur RF 433 Mhz

// ## -- Fin de la configuration -- ##

// Sensor RF 433Mhz
RCSwitch rf433write = RCSwitch();

// Arduilink
Arduilink lnk = Arduilink(2);

// DHT Sensor
dht DHT;

// Send to RF 433Mhz emitter
void sendRf433(const char* msg) {
  long data = String(msg).toInt();
  if (data > 0)
  {
    rf433write.send(data, 32);
    rf433write.send(data, 32);
  }
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
        lnk.setValue(1, "1");
        sendRf433("666666");
      }
      lastPowerHit = millis();
      lightValue = analogRead(PIN_PHOTO_CELL);
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
      //char buff[256];
      //sprintf(buff, "300 %3d %d", currentTemperature, currentHumidity);
      //Serial.println(buff);
    }

    // Handle inputs from Serial
    //lnk.handleInput();
    
  }
  
}
