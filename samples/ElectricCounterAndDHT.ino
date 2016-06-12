#include <Arduilink.h>
#include <dht.h>

#define PIN_DHT 7 // Pin pour la sonde DHT
#define PIN_PHOTO_CELL A0 // Pin du capteur de luminosité
#define FLASH_DELTA_SENSIBILITY 20
#define FLASH_DELTA_DELAY 150
#define DHT_UPDATE_DELAY 10000

Arduilink lnk = Arduilink(0);
dht DHT;

void setup()
{
  // Add sensors
  lnk.addSensor(1, S_INFO | S_HIT, "Compteur EDF", "Wh")->verbose = true;
  lnk.addSensor(2, S_INFO, "Temperature", "C");
  lnk.addSensor(3, S_INFO, "Humidite", "%");
  
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
  int lighValue = 0;
  // Sum of recorded values
  unsigned long sum = 0;
  // Time stamp for DHT update
  unsigned long lastDHTupdate = 0;
 
  while (1)
  {
    // Ligh sensor
    long delta = lighValue - analogRead(PIN_PHOTO_CELL);
    if (abs(delta) > FLASH_DELTA_SENSIBILITY) {
      // TODO Gerer la remise à zero de millis() tous les 50 jours
      if (millis() - lastPowerHit > FLASH_DELTA_DELAY) {
        lastPowerHit = millis();
        lnk.setValue(1, ++sum);
      }
    }
    lighValue = analogRead(PIN_PHOTO_CELL);

    // DHT
    if (millis() - lastDHTupdate > DHT_UPDATE_DELAY) {
      lastDHTupdate = millis();
      int chk = DHT.read11(PIN_DHT);
      if (chk != DHTLIB_OK) {
        //lnk.setError();
        continue;
      }
      // Update sensors
      lnk.setValue(2, DHT.temperature);
      lnk.setValue(3, DHT.humidity);
    }

    // Read inputs
    lnk.handleInput();
    
  }
  
}
