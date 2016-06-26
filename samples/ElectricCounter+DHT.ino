#include <Arduilink.h>
#include <dht.h>

#define PIN_DHT 7 // Pin pour la sonde DHT
#define PIN_PHOTO_CELL A0 // Pin du capteur de luminosité
#define FLASH_DELTA_SENSIBILITY 250
#define FLASH_DELTA_DELAY 100
#define DHT_UPDATE_DELAY 10000

Arduilink lnk = Arduilink(0);
dht DHT;

void setup()
{
  // Add sensors
  lnk.addSensor(1, S_INFO | S_HIT, "Compteur EDF", "Wh/min")->verbose = true;
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
  int lightValue = 0;
  // Sum of recorded values
  unsigned long sum = 0;
  // Time stamp for DHT sensor update
  unsigned long lastDHTupdate = 0;
  // Time stamp for electric consumption sensor update
  unsigned long lastEDFupdate = 0;
  // Current values for DHT sensor
  double currentTemperature = 0;
  double currentHumidity = 0;
 
  while (1)
  {
    
    // Ligh sensor
    if (millis() - lastPowerHit > FLASH_DELTA_DELAY) {
      long delta = lightValue - analogRead(PIN_PHOTO_CELL);
      if (delta > FLASH_DELTA_SENSIBILITY) {
        // TODO Gerer la remise à zero de millis() tous les 50 jours
        //Serial.print("Hit "); Serial.println(delta);
        sum++;
      }
      lastPowerHit = millis();
      lightValue = analogRead(PIN_PHOTO_CELL);
      //Serial.println(lightValue);
    }

    // Electric sensor update
    if (millis() - lastEDFupdate > 60000) {
      lastEDFupdate = millis();
      lnk.setValue(1, sum);
      sum = 0;
    }

    // DHT update
    if (millis() - lastDHTupdate > DHT_UPDATE_DELAY) {
      lastDHTupdate = millis();
      int chk = DHT.read11(PIN_DHT);
      if (chk != DHTLIB_OK) {
        continue;
      }
      // Update sensors, if the delta is not aberant
      if (currentTemperature == 0 || abs(currentTemperature - DHT.temperature) < 5) {
        lnk.setValue(2, currentTemperature = DHT.temperature);
      }
      if (currentHumidity == 0 || abs(currentHumidity - DHT.humidity) < 5) {
        lnk.setValue(3, currentHumidity = DHT.humidity);
      }
    }

    // Read inputs
    lnk.handleInput();
    
  }
  
}
