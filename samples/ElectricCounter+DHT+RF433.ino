#include <Arduilink.h>
#include <dht.h>
#include <RCSwitch.h>

// Sonde DHT
#define PIN_DHT 7
#define DHT_UPDATE_DELAY 10000

// Capteur de luminosité (impulsions conso EDF)
#define PIN_PHOTO_CELL A0
#define FLASH_DELTA_SENSIBILITY 250
#define FLASH_DELTA_DELAY 100

// Capteur RF 433
#define INT_433_SENSOR 0 // Pour l'interruption sur pin #2
#define PIN_433_EMITTER 10 // Pin de l'emetteur RF 433 Mhz

// ## -- Fin de la configuration -- ##

// Sensor RF 433Mhz
RCSwitch rf433read = RCSwitch();
RCSwitch rf433write = RCSwitch();
// Arduilink
Arduilink lnk = Arduilink(0);
// DHT Sensor
dht DHT;

// 
void sendRf433(const char* msg) {
  long data = String(msg).toInt();
  if (data > 0)
  {
    // Send to RF 433Mhz emitter
    rf433write.send(data, 32);
    rf433write.send(data, 32);
    //rf433write.switchOn("11111", "00010"); // DIP
    //rf433write.send(5393, 24); // Decimal
    //rf433write.send("000000000001010100010001"); // Binary
    //rf433write.sendTriState("00000FFF0FF0"); // Tri-state code
  }
}

void setup()
{
  // Add sensors
  lnk.addSensor(1, S_INFO | S_HIT, "Compteur EDF", "Wh/min")->verbose = true;
  lnk.addSensor(2, S_INFO, "Temperature", "C");
  lnk.addSensor(3, S_INFO, "Humidite", "%");
  lnk.addSensor(4, S_INFO, "RF433", "opcode", sendRf433)->verbose = true;
  
  // Enable RF 433
  rf433read.enableReceive(INT_433_SENSOR);
  rf433read.setPulseLength(321);
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
        continue; // Error
      }
      // Update sensors, if the delta is not aberant
      if (currentTemperature == 0 || abs(currentTemperature - DHT.temperature) < 5) {
        lnk.setValue(2, currentTemperature = DHT.temperature);
      }
      if (currentHumidity == 0 || abs(currentHumidity - DHT.humidity) < 5) {
        lnk.setValue(3, currentHumidity = DHT.humidity);
      }
    }

    // Read data from RF 433Mhz receiver
    if (rf433read.available())
    {
      unsigned long value = rf433read.getReceivedValue();
      if (value == 0) {
        rf433read.resetAvailable();
        continue; // Error
      }
      lnk.setValue(4, value);
      rf433read.resetAvailable();
    }

    // Read inputs
    lnk.handleInput();
    
  }
  
}