#include <Arduilink.h>
#include <stdio.h>
#include <string.h>

Arduilink::Arduilink(unsigned int _id, void(*_serial)(const char *)) {
	id = _id;
	serial = _serial;
	head = queue = 0;
	sensorsCount = 0;
}


SensorItem* Arduilink::addSensor(unsigned int _id, SensorType type, const char* _name) {
	SensorItem *sensor;
	sensor = new SensorItem;

	sensor->id = _id;
	sensor->name = _name;
	sensor->type = type;

	sensor->next = head;
	head = sensor;

	if (sensorsCount == 0)
		queue = sensor;

	++sensorsCount;

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
	sprintf(buff, "D:%d;%d;%s;%s\n", id, _id, _value, sensor->name);
	serial(buff);
}


void Arduilink::printSensors() {
	SensorItem* sensor = head;
	while (sensor != NULL) {
		char buff[256];
		sprintf(buff, "S:%d;%d;%s;%d\n", id, sensor->id, sensor->name, sensor->type);
		serial(buff);
		sensor = sensor->next;
	}
}