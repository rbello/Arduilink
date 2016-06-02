#include <Arduilink.h>
#include <stdio.h>
#include <string.h>

Arduilink::Arduilink(unsigned int _id) {
	nodeId = _id;
	head = queue = 0;
	sensorsCount = 0;
}

SensorItem* Arduilink::addSensor(unsigned int _id, SensorType _type, const char* _name) {
	return addSensor(_id, _type, _name, 0);
}

SensorItem* Arduilink::addSensor(unsigned int _id, SensorType _type, const char* _name, void(*_writter)(const char *msg)) {
	// Create sensor
	SensorItem *sensor;
	sensor = new SensorItem;
	// Set sensor's attributes
	sensor->id = _id;
	sensor->name = _name;
	sensor->type = _type;
	sensor->writter = _writter;
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
	sensor->value = String(_value);
	// Ouput TODO Add a test
	//char buff[256];
	//sprintf(buff, "D;%d;%d;%s\n", nodeId, _id, _value);
	//Serial.print(buff);
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
		sprintf(buff, "S;%d;%d;%s;%d\n", nodeId, sensor->id, sensor->name, sensor->type);
		Serial.print(buff);
		sensor = sensor->next;
	}
}

void Arduilink::send(unsigned int _id, const char* _msg) {
	SensorItem* sensor = getSensor(_id);
	if (sensor == NULL) return; // TODO return error
	if (sensor->writter != NULL)
		sensor->writter(_msg);
}

void Arduilink::send(unsigned int sensorId, String &msg) {
	char buf[msg.length() + 1];
	msg.toCharArray(buf, msg.length() + 1);
	send(sensorId, buf);
}

void Arduilink::setFailure(unsigned int sensorId, const char* msg) {

}

int Arduilink::handleInput() {
	if (Serial.available() > 0) {
		String str = Serial.readString();

		char buf[str.length() + 1];
		str.toCharArray(buf, str.length() + 1);

		char* opcode = NULL;
		int node = -1;
		int sensorId = -1;

		char* pch;
		pch = strtok(buf, ";");
		while (pch != NULL)
		{
			if (opcode == NULL) {
				if (strcmp(pch, "G") != 0) {
					Serial.print("Warning: invalid opcode ");
					Serial.println(pch);
					return 2;
				}
				opcode = pch;
			}
			else if (node == -1) {
				node = atoi(pch);
			}
			else if (sensorId == -1) {
				sensorId = atoi(pch);
			}
			pch = strtok(NULL, ";");
		}

		/*Serial.print("Opcode: ");
		Serial.print(opcode);
		Serial.print(" NodeId: ");
		Serial.print(node);
		Serial.print(" SensorId: ");
		Serial.println(sensorId);*/

		if (nodeId != node) {
			Serial.print("Warning: no route found for node ");
			Serial.println(node);
			return 3;
		}

		SensorItem* sensor = getSensor(sensorId);
		if (sensor == NULL) {
			Serial.print("Warning: sensor not found ");
			Serial.println(sensorId);
			return 4;
		}

		char buff[256];
		sprintf(buff, "V;%d;%d;%d;%s;", node, sensor->id, sensor->type, sensor->name);
		Serial.print(buff);
		Serial.println(sensor->value);

		return 0;
	}
	return 1;
}