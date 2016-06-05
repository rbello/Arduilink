#include <dht.h>
#include <Arduilink.h>

#define DELAY_SLEEP 250 // En millisecondes
#define PIN_DHT 7 // Pin pour la sonde DHT

dht DHT;
Arduilink lnk = Arduilink(0);

void setup() {
  lnk.addSensor(1, S_TEMP, "Temperature sensor (DHT)");
  lnk.addSensor(2, S_HUM, "Humidity sensor (DHT)");
  Serial.begin(9600);
  lnk.init();
}

void loop() {

  // Mise à jour du capteur DHT
  int chk = DHT.read11(PIN_DHT);
  if (chk != DHTLIB_OK) {
    lnk.setFailure(1, "Error");
    lnk.setFailure(2, "Error");
  }
  else {
    // Update sensors
    lnk.setValue(1, DHT.temperature);
    lnk.setValue(2, DHT.humidity);
  }

  // On met le programme en pause
  delay(DELAY_SLEEP);
  
}

void serialEvent() {
  lnk.handleInput();
}
