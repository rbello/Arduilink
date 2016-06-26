#include <Arduilink.h>
#include <stdio.h>
#include <string.h>

Arduilink::Arduilink(unsigned int _id) {
	nodeId = _id;
	head = queue = 0;
	sensorsCount = 0;
	write = false;
}

void Arduilink::init() {
	Serial.print("100;");
	Serial.print(nodeId);
	Serial.println(";1.0");
	Serial.flush();
}

SensorItem* Arduilink::addSensor(unsigned int _id, unsigned int _flags, const char* _name, const char* _unit) {
	return addSensor(_id, _flags, _name, _unit, 0);
}

SensorItem* Arduilink::addSensor(unsigned int _id, unsigned int _flags, const char* _name, const char* _unit, void(*_writter)(const char *msg)) {
	// Create sensor
	SensorItem *sensor;
	sensor = new SensorItem;
	// Set sensor's attributes
	sensor->id      = _id;
	sensor->flags   = _flags;
	sensor->name    = _name;
	sensor->unit    = _unit;
	sensor->writter = _writter;
	// Set sensor's default state
	sensor->verbose = false;
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
	sensor->value = String(_value);
	// Auto-ouput (verbose mode)
	if (sensor->verbose == true) {
		//if (strncmp(_value, sensor->value, strlen(_value)) == 0) return;
		//if (strcmp(copy, sensor->value) == 0) return;
		char buff[256];
		sprintf(buff, "200;%d;%d;%s", nodeId, _id, _value);
		while (write) {}
		write = true;
		Serial.println(buff);
		Serial.flush();
		write = false;
	}
}

void Arduilink::setValue(unsigned int _id, double _value) {
	String val = String(_value);
	char buf[val.length() + 1];
	val.toCharArray(buf, val.length() + 1);
	setValue(_id, buf);
}

void Arduilink::printSensor(SensorItem* sensor, unsigned int _nodeId) {
	char buff[256];
	sprintf(buff, "300;%d;%d;%d;%d;%d;%s", _nodeId, sensor->id, sensor->flags, sensor->unit, sensor->verbose, sensor->name);
	Serial.println(buff);
}

void Arduilink::printSensors() {
	SensorItem* sensor = head;
	while (sensor != NULL) {
		printSensor(sensor, nodeId);
		sensor = sensor->next;
	}
	// Add an empty line to indicate the end of list
	Serial.println();
	Serial.flush();
}

void Arduilink::send(unsigned int _id, const char* _msg) {
	SensorItem* sensor = getSensor(_id);
	if (sensor == NULL) return; // TODO return error
	if (sensor->writter != NULL)
		sensor->writter(_msg);
	// TODO else return warning
}

void Arduilink::send(unsigned int sensorId, String &msg) {
	char buf[msg.length() + 1];
	msg.toCharArray(buf, msg.length() + 1);
	send(sensorId, buf);
}

void Arduilink::setFailure(unsigned int sensorId, const char* msg) {

}

int Arduilink::handleInput() {
	
	while (Serial.available() > 0) {
		
		while (write) {}
		write = true;

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
				if (strcmp(pch, "PRESENT") == 0) {
					printSensors();
					write = false;
					return 0;
				}
				if (strcmp(pch, "GET") != 0 && strcmp(pch, "INFO") != 0 && strcmp(pch, "SET") != 0) {
					Serial.print("400;OPCODE;");
					Serial.println(pch);
					write = false;
					return 2;
				}
				opcode = pch;
			}
			else if (node == -1) {
				node = atoi(pch);
			}
			else if (sensorId == -1) {
				sensorId = atoi(pch);
				break;
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
			Serial.print("404;NODE;");
			Serial.println(node);
			write = false;
			return 3;
		}

		SensorItem* sensor = getSensor(sensorId);
		if (sensor == NULL) {
			Serial.print("404;SENSOR;");
			Serial.println(sensorId);
			write = false;
			return 4;
		}

		// INFO - Récupérer la description d'un capteur
		if (strcmp(opcode, "INFO") == 0) {
			printSensor(sensor, node);
		}

		// SET - Modifier un attribut d'un capteur
		if (strcmp(opcode, "SET") == 0) {
			pch = strtok(NULL, ";");
			if (strcmp(pch, "VERBOSE") == 0) {
				pch = strtok(NULL, ";");
				if (strcmp(pch, "on") == 0) {
					Serial.println("201;VERBOSE;1"); // TODO Ameliorable
					Serial.flush();
					sensor->verbose = true;
				}
				else if (strcmp(pch, "off") == 0) {
					sensor->verbose = false;
					Serial.println("201;VERBOSE;0"); // TODO Ameliorable
					Serial.flush();
				}
				else {
					Serial.print("400;OPTION;VERBOSE;");
					Serial.println(pch);
					write = false;
					return 6;
				}
			}
			else if (strcmp(pch, "VALUE") == 0) {
				pch = strtok(NULL, ";");
				if (sensor->writter != NULL)
					sensor->writter(pch);
				else
					sensor->value = pch;
				Serial.print("201;ACK;");
				Serial.print(node);
				Serial.print(";");
				Serial.println(sensor->id);
			}
			else {
				Serial.print("400;ATTRIBUTE;");
				Serial.println(pch);
				write = false;
				return 5;
			}
		}

		// GET - Récupérer la valeur d'un capteur
		if (strcmp(opcode, "GET") == 0) {
			char buff[256];
			sprintf(buff, "200;%d;%d;", node, sensor->id);
			Serial.print(buff);
			Serial.println(sensor->value);
		}

		Serial.flush();
		write = false;
		return 0;
	}
	return 1;
}
