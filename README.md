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
		sleep(5000);
	}
}
```

***

### Serial Protocol

The serial protocol used between the Gateway and the Controller is a simple semicolon separated list of values.

There are two types of frames which can be exchanged :
- a frame describing available sensors,
- a frame containing the payload (he message coming in from sensors or instruction going out)

The last part of this "command" is the payload. All commands ends with a newline.

##### Sensor presentation
The serial commands has the following format:
#####`S`:`node-id`;`child-sensor-id`;`sensor-type`;`sensor-name`\n

Message Part | Comment
--- | ---
*node-id* | The unique id of the node that sends the message (address)
*child-sensor-id* | Each node can have several sensors attached. This is the child-sensor-id that uniquely identifies one attached sensor
*sensor-type* | Type of message sent - See table below
*sensor-name* | Name of the sensor

##### Data/instruction transfert
The serial commands has the following format:
#####`D`:`node-id`;`child-sensor-id`;`message-type`;`ack`;`payload`\n

Message Part | Comment
--- | ---
*node-id* | The unique id of the node that sends the message (address)
*child-sensor-id* | Each node can have several sensors attached. This is the child-sensor-id that uniquely identifies one attached sensor
*message-type* | Type of message sent - See table below
*payload* | The payload holds the message coming in from sensors or instruction going out to actuators.

Type | Value | Comment | Unit
--- | --- | --- | ----
S_DOOR | 0 | Door and window sensors | V_TRIPPED, V_ARMED
S_MOTION | 1 | Motion sensors | V_TRIPPED, V_ARMED
S_SMOKE | 2 | Smoke sensor | V_TRIPPED, V_ARMED
S_LIGHT | 3 | Light Actuator (on/off) | V_STATUS (or V_LIGHT), V_WATT
S_BINARY | 3 | Binary device (on/off), Alias for S_LIGHT  | V_STATUS (or V_LIGHT), V_WATT
S_DIMMER | 4 | Dimmable device of some kind | V_STATUS (on/off), V_DIMMER (dimmer level 0-100), V_WATT
S_COVER | 5 | Window covers or shades | V_UP, V_DOWN, V_STOP, V_PERCENTAGE
S_TEMP | 6 | Temperature sensor | Temperature (degrees)
S_HUM | 7 | Humidity sensor | Humidity percentage
S_BARO | 8 | Barometer sensor (Pressure) | V_PRESSURE, V_FORECAST
S_WIND | 9 | Wind sensor | V_WIND, V_GUST
S_RAIN | 10 | Rain sensor | V_RAIN, V_RAINRATE
S_UV | 11 | UV sensor | V_UV
S_WEIGHT | 12 | Weight sensor for scales etc. | V_WEIGHT, V_IMPEDANCE
S_POWER | 13 | Power measuring device, like power meters | V_WATT, V_KWH
S_HEATER | 14 | Heater device | V_HVAC_SETPOINT_HEAT, V_HVAC_FLOW_STATE, V_TEMP
S_DISTANCE | 15 | Distance sensor | V_DISTANCE, V_UNIT_PREFIX
S_LIGHT_LEVEL | 16 | Light sensor | Lux
S_ARDUINO_NODE | 17 | Arduino node device | 
S_ARDUINO_REPEATER_NODE | 18 | Arduino repeating node device | 
S_LOCK | 19 | Lock device | Lock status
S_IR | 20 | Ir sender/receiver device | V_IR_SEND, V_IR_RECEIVE
S_WATER | 21 | Water meter | V_FLOW, V_VOLUME
S_AIR_QUALITY | 22 | Air quality sensor e.g. MQ-2 | V_LEVEL, V_UNIT_PREFIX
S_CUSTOM | 23 | Use this for custom sensors where no other fits. | 
S_DUST | 24 | Dust level sensor | V_LEVEL, V_UNIT_PREFIX
S_SCENE_CONTROLLER | 25 | Scene controller device | V_SCENE_ON, V_SCENE_OFF
S_RGB_LIGHT | 26 | RGB light | V_RGB, V_WATT
S_RGBW_LIGHT | 27 | RGBW light (with separate white component) | V_RGBW, V_WATT
S_COLOR_SENSOR | 28 | Color sensor | V_RGB
S_HVAC | 29 | Thermostat/HVAC device | V_HVAC_SETPOINT_HEAT, V_HVAC_SETPOINT_COLD, V_HVAC_FLOW_STATE, V_HVAC_FLOW_MODE, V_HVAC_SPEED
S_MULTIMETER | 30 | Multimeter device | V_VOLTAGE, V_CURRENT, V_IMPEDANCE
S_SPRINKLER | 31 | Sprinkler device | V_STATUS (turn on/off), V_TRIPPED (if fire detecting device)
S_WATER_LEAK | 32 | Water leak sensor | V_TRIPPED, V_ARMED
S_SOUND | 33 | Sound sensor | V_LEVEL (in dB), V_TRIPPED, V_ARMED
S_VIBRATION | 34 | Vibration sensor | V_LEVEL (vibration in Hz), V_TRIPPED, V_ARMED
S_MOISTURE | 35 | Moisture sensor | V_LEVEL (water content or moisture in percentage?), V_TRIPPED, V_ARMED