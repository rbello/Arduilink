#include <Arduilink.h>
#include <stdio.h>
#include <string.h>
#include <Arduino.h>

Arduilink::Arduilink(unsigned int _id) {
	nodeId = _id;
	head = queue = 0;
	sensorsCount = 0;
}

SensorItem* Arduilink::addSensor(unsigned int _id, SensorType type, const char* _name) {
	// Create sensor
	SensorItem *sensor;
	sensor = new SensorItem;
	// Set sensor's attributes
	sensor->id = _id;
	sensor->name = _name;
	sensor->type = type;
	// Chained list
	sensor->next = head;
	head = sensor;
	if (sensorsCount == 0)
		queue = sensor;
	// Counter
	++sensorsCount;
	// Return created sensor
	return sensor;
}

SensorItem* Arduilink::getSensor(unsigned int _id) {
	SensorItem* sensor = head;
	while (sensor != NULL) {
		if (sensor->id == _id) break;
		sensor = sensor->next;
	}
	return sensor;
}

void Arduilink::setValue(unsigned int _id, const char* _value) {
	SensorItem* sensor = getSensor(_id);
	if (sensor == NULL) return; // TODO Debug
	//if (strncmp(_value, sensor->value, strlen(_value)) == 0) return;
	//if (strcmp(copy, sensor->value) == 0) return;
	sensor->value = _value;
	char buff[256];
	sprintf(buff, "D:%d;%d;%s;%s\n", nodeId, _id, _value, sensor->name);
	Serial.print(buff);
}

void Arduilink::setValue(unsigned int _id, double _value) {
	String val = String(_value);
	char buf[val.length() + 1];
	val.toCharArray(buf, val.length() + 1);
	setValue(_id, buf);
}

void Arduilink::printSensors() {
	SensorItem* sensor = head;
	while (sensor != NULL) {
		char buff[256];
		sprintf(buff, "S:%d;%d;%s;%d\n", nodeId, sensor->id, sensor->name, sensor->type);
		Serial.print(buff);
		sensor = sensor->next;
	}
}