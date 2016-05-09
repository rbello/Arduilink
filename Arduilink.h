
// Type of sensor (used when presenting sensors)
typedef enum  {
	S_DOOR, // Door sensor, V_TRIPPED, V_ARMED
	S_MOTION,  // Motion sensor, V_TRIPPED, V_ARMED 
	S_SMOKE,  // Smoke sensor, V_TRIPPED, V_ARMED
	S_LIGHT, // Binary light or relay, V_STATUS (or V_LIGHT), V_WATT
	S_BINARY = 3, // Binary light or relay, V_STATUS (or V_LIGHT), V_WATT (same as S_LIGHT)
	S_DIMMER, // Dimmable light or fan device, V_STATUS (on/off), V_DIMMER (dimmer level 0-100), V_WATT
	S_COVER, // Blinds or window cover, V_UP, V_DOWN, V_STOP, V_DIMMER (open/close to a percentage)
	S_TEMP, // Temperature sensor, V_TEMP
	S_HUM, // Humidity sensor, V_HUM
	S_BARO, // Barometer sensor, V_PRESSURE, V_FORECAST
	S_WIND, // Wind sensor, V_WIND, V_GUST
	S_RAIN, // Rain sensor, V_RAIN, V_RAINRATE
	S_UV, // Uv sensor, V_UV
	S_WEIGHT, // Personal scale sensor, V_WEIGHT, V_IMPEDANCE
	S_POWER, // Power meter, V_WATT, V_KWH
	S_HEATER, // Header device, V_HVAC_SETPOINT_HEAT, V_HVAC_FLOW_STATE, V_TEMP
	S_DISTANCE, // Distance sensor, V_DISTANCE
	S_LIGHT_LEVEL, // Light level sensor, V_LIGHT_LEVEL (uncalibrated in percentage),  V_LEVEL (light level in lux)
	S_ARDUINO_NODE, // Used (internally) for presenting a non-repeating Arduino node
	S_ARDUINO_REPEATER_NODE, // Used (internally) for presenting a repeating Arduino node 
	S_LOCK, // Lock device, V_LOCK_STATUS
	S_IR, // Ir device, V_IR_SEND, V_IR_RECEIVE
	S_WATER, // Water meter, V_FLOW, V_VOLUME
	S_AIR_QUALITY, // Air quality sensor, V_LEVEL
	S_CUSTOM, // Custom sensor 
	S_DUST, // Dust sensor, V_LEVEL
	S_SCENE_CONTROLLER, // Scene controller device, V_SCENE_ON, V_SCENE_OFF. 
	S_RGB_LIGHT, // RGB light. Send color component data using V_RGB. Also supports V_WATT 
	S_RGBW_LIGHT, // RGB light with an additional White component. Send data using V_RGBW. Also supports V_WATT
	S_COLOR_SENSOR,  // Color sensor, send color information using V_RGB
	S_HVAC, // Thermostat/HVAC device. V_HVAC_SETPOINT_HEAT, V_HVAC_SETPOINT_COLD, V_HVAC_FLOW_STATE, V_HVAC_FLOW_MODE, V_TEMP
	S_MULTIMETER, // Multimeter device, V_VOLTAGE, V_CURRENT, V_IMPEDANCE 
	S_SPRINKLER,  // Sprinkler, V_STATUS (turn on/off), V_TRIPPED (if fire detecting device)
	S_WATER_LEAK, // Water leak sensor, V_TRIPPED, V_ARMED
	S_SOUND, // Sound sensor, V_TRIPPED, V_ARMED, V_LEVEL (sound level in dB)
	S_VIBRATION, // Vibration sensor, V_TRIPPED, V_ARMED, V_LEVEL (vibration in Hz)
	S_MOISTURE, // Moisture sensor, V_TRIPPED, V_ARMED, V_LEVEL (water content or moisture in percentage?) 
} SensorType;

typedef struct SensorItem {
	unsigned int id;
	SensorType type;
	const char* name;
	const char* value;
	SensorItem* next;
};

class Arduilink {
public:
	Arduilink(unsigned int nodeId);

	SensorItem* addSensor(unsigned int sensorId, SensorType sensorType, const char* sensorName);
	void setValue(unsigned int sensorId, const char* sensorValue);
	void setValue(unsigned int sensorId, double sensorValue);
	SensorItem* getSensor(unsigned int sensorId);
	void printSensors();

private:
	unsigned int nodeId;
	SensorItem *head;
	SensorItem *queue;
	int sensorsCount;

};
