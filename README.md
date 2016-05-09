# Arduilink

A very simple system of communication between Arduino and Raspberry using the serial to collect data from several sensors

### Example

```c
// Required library
#include <Arduilink.h>

// Unique identifier of the node
#define NODE_ID 10

// Output function
void output(const char *string) {
   Serial.print(string);
}

// Create the link
Arduilink link = Arduilink(NODE_ID, output);

// Setup function
void setup()
{
	link.addSensor(1, S_TEMP, "Temperature sensor (DHT)");
	link.addSensor(2, S_HUM, "Humidity sensor (DHT)");
}

// Loop function
void loop()
{
	while (1)
	{
		link.setValue(1, mySensor.getTemperature());
		link.setValue(2, mySensor.getHumidity());
	}
}
```

***

### Serial Protocol

The serial protocol used between the Gateway and the Controller is a simple semicolon separated list of values. The last part of each "command" is the payload. All commands ends with a newline. The serial commands has the following format:

node-id;child-sensor-id;message-type;ack;payload\n

Message Part | Comment
--- | ---
*node-id* | The unique id of the node that sends the message (address)
*child-sensor-id* | Each node can have several sensors attached. This is the child-sensor-id that uniquely identifies one attached sensor
*message-type* | Type of message sent - See table below
*payload* | The payload holds the message coming in from sensors or instruction going out to actuators.