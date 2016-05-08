#include <Arduilink.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

Arduilink::Arduilink(unsigned int _id, void(*_serial)(const char *)) {
	id = _id;
	serial = _serial;
	head = queue = 0;
	sensorsCount = 0;
}

/*void Arduilink::setValue(unsigned int _id, long _value) {
	char buff[256];
	sprintf(buff, "D:%d;%d;%d\n", id, _id, _value);
	serial(buff);
}*/
void Arduilink::setValue(unsigned int _id, char* _value) {
	Sensor* sensor = head;
	while (sensor != NULL) {
		if (sensor->id == _id) break;
		sensor = sensor->next;
	}
	if (sensor == NULL)
		return; // TODO Debug
	char buff[256];
	sprintf(buff, "D:%d;%d;%s;%s\n", id, _id, _value, sensor->name);
	serial(buff);
}

int Arduilink::addSensor(unsigned int _id, char* _name) {
	Sensor *sensor;
	sensor = new Sensor;

	sensor->id = _id;
	sensor->name = _name;

	sensor->next = head;
	head = sensor;

	if (sensorsCount == 0)
		queue = sensor;

	++sensorsCount;

	return 1;
}

void Arduilink::printSensors() {
	Sensor* sensor = head;
	while (sensor != NULL) {
		char buff[256];
		sprintf(buff, "S:%d;%d;%s;tick\n", id, sensor->id, sensor->name);
		serial(buff);
		sensor = sensor->next;
	}
}