#include <RCSwitch.h>
#include <Arduilink.h>
#include <dht.h>

#define INT_433_SENSOR 0 // Pour l'interruption sur pin #2
#define PIN_433_EMITTER 10 // Pin de l'emetteur RF 433 Mhz
#define PIN_DHT 7 // Pin pour la sonde DHT
#define DELAY_UPDATE_DHT 10000 // En millisecondes

// Sensor RF 433Mhz
RCSwitch rf433read = RCSwitch();
RCSwitch rf433write = RCSwitch();

// Arduilink
Arduilink lnk = Arduilink(10);

// DHT Sensor
dht DHT;

void sendRf(const char* msg) {
  long data = String(msg).toInt();
  if (data > 0)
  {
    Serial.print("SENT ");
    Serial.println(data);
    // Send to RF 433Mhz emitter
    rf433write.send(data, 32);
    rf433write.send(data, 32);
    rf433write.send(data, 32);
    //rf433write.switchOn("11111", "00010"); // DIP
    //rf433write.send(5393, 24); // Decimal
    //rf433write.send("000000000001010100010001"); // Binary
    //rf433write.sendTriState("00000FFF0FF0"); // Tri-state code
  }
}

/**
 * Setup
 */
void setup()
{
  
  Serial.begin(115200); // bauds
  Serial.setTimeout(10); // milliseconds

  rf433read.enableReceive(INT_433_SENSOR);
  rf433read.setPulseLength(321);

  rf433write.enableTransmit(PIN_433_EMITTER);
  rf433write.setPulseLength(320);
  rf433write.setProtocol(2);

  lnk.addSensor(1, S_POWER, "EDF");
  lnk.addSensor(2, S_CUSTOM, "RF433", sendRf);
  lnk.addSensor(10, S_BINARY, "Prise Otio 1");
  lnk.addSensor(11, S_BINARY, "Prise Otio 2");
  lnk.addSensor(20, S_BINARY, "Interrupteur Optio 1");
  lnk.addSensor(30, S_BINARY, "Detecteur de porte Otio 1");
  lnk.addSensor(40, S_TEMP, "Temperature sonde DHT");
  lnk.addSensor(41, S_HUM, "Humidite sonde DHT");

  Serial.println("A R D U I L I N K");
  Serial.println("~ Available sensors are:");
  lnk.printSensors();
  Serial.println("~ Available commands are: getsensors");
  Serial.println("~ Everything else is parsed like an integer and sent thru RF 433MHz.");
  
}

/**
 * Loop
 */
void loop()
{

  Serial.println("Running...");

  unsigned long lastUpdateTimeDHT;
 
  while (1)
  {
    
    // Read data from RF 433Mhz receiver
    if (rf433read.available())
    {
      unsigned long value = rf433read.getReceivedValue();
      if (value == 0)
      {
        Serial.print("RCVD ERR");
      }
      else
      {
        
        if      (value ==  93074432) lnk.setValue(10, "On");
        else if (value == 227292160) lnk.setValue(10, "Off");
        
        else if (value == 965489664) lnk.setValue(11, "On");
        else if (value == 898380800) lnk.setValue(11, "Off");

        else if (value == 893534720) lnk.setValue(20, "On");
        else if (value == 960643584) lnk.setValue(20, "Off");
        
        else if (value == 553451792) lnk.setValue(30, "On");
        else if (value == 687669520) lnk.setValue(30, "Off");
        
        else lnk.setValue(2, value);

        // rf433read.getReceivedBitlength());
        // rf433read.getReceivedProtocol());

      }
      rf433read.resetAvailable();
    }

    // Read commands from Serial stream
    if (Serial.available() > 0)
    {
      String input = Serial.readString();
      if (input == "getsensors") {
        lnk.printSensors();
        continue;
      }
      // Send to RF sensor
      lnk.send(2, input);
    }

    if (millis() - lastUpdateTimeDHT > DELAY_UPDATE_DHT) {
      lastUpdateTimeDHT = millis();
      int chk = DHT.read11(PIN_DHT);
      if (chk != DHTLIB_OK) {
        //lnk.setError();
        continue;
      }
      // Update sensors
      lnk.setValue(40, DHT.temperature);
      lnk.setValue(41, DHT.humidity);
    }
    
  }
}
