#include <Arduino.h>

#define S_INFO     1
#define S_HIT      2
#define S_ACTION   4
#define S_BATTERY  8

typedef struct SensorItem {
	// Main attributes
	unsigned int id;
	unsigned int flags;
	const char* name;
	const char* unit;
	void(*writter)(const char *msg);
	// Operational attributes
	bool verbose;
	String value;
	// Chained list
	SensorItem* next;
};

class Arduilink {
public:

	/**
	 * Constructor.
	 */
	Arduilink(unsigned int nodeId);

	/**
	 * Prepares communication on the serial link by sending a frame indicating the identifier of the node and the protocol version used.
	 */
	void init();

	/**
	 * Adds a sensor with minimal information.
	 */
	SensorItem* addSensor(unsigned int sensorId, unsigned int sensorFlags, const char* sensorName, const char* quantityUnit);

	/**
	 * Adds a sensor with a callback function used to execute operations on it.
	 */
	SensorItem* addSensor(unsigned int sensorId, unsigned int sensorFlags, const char* sensorName, const char* quantityUnit, void(*_writter)(const char *msg));

	/**
	 * Get a sensor by id.
	 */
	SensorItem* getSensor(unsigned int sensorId);

	/**
	 * Update the current value of a sensor.
	 */
	void setValue(unsigned int sensorId, const char* sensorValue);

	/**
	 * Update the current value of a sensor.
	 */
	void setValue(unsigned int sensorId, double sensorValue);
	
	/**
	 * Send an operation to a sensor.
	 */
	void send(unsigned int sensorId, String &msg);

	/**
	 * Send an operation to a sensor.
	 */
	void send(unsigned int sensorId, const char* msg);

	/**
	 * Indicate that sensor returns an error.
	 */
	void setFailure(unsigned int sensorId, const char* msg);

	/**
	 * Describes all sensors and sends the information on the serial link .
	 */
	void printSensors();

	/**
	 * Manages the reception of data over the serial link, to interpret and handle received orders.
	 * Returns:
	 *		0	OK, command handled
	 *		1	OK, nothing to handle
	 *		2	Invalid opcode (400 bad request)
	 *		3	Invalid node-id (404 not found)
	 *		4	Invalid sensor-id 
	 *		5	Invalid attribute for the SET operation (400 bad request)
	 *		6	Invalid value for an attribute, for the SET operation (400 bad request)
	 */
	int handleInput();

private:
	unsigned int nodeId;
	SensorItem *head;
	SensorItem *queue;
	int sensorsCount;
	bool write;

	void printSensor(SensorItem* sensor, unsigned int nodeId);

};
