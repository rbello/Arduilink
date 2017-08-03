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

SensorItem* Arduilink::setValue(unsigned int _id, const char* _value) {
	SensorItem* sensor = getSensor(_id);
	if (sensor == NULL) return NULL; // TODO Debug
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
	return sensor;
}

SensorItem* Arduilink::setValue(unsigned int _id, double _value) {
	String val = String(_value);
	char buf[val.length() + 1];
	val.toCharArray(buf, val.length() + 1);
	return setValue(_id, buf);
}

SensorItem* Arduilink::setValue(unsigned int _id, unsigned long _value) {
	String val = String(_value);
	char buf[val.length() + 1];
	val.toCharArray(buf, val.length() + 1);
	return setValue(_id, buf);
}

void Arduilink::printSensor(SensorItem* sensor, unsigned int _nodeId) {
	char buff[256];
	sprintf(buff, "300;%d;%d;%d;%s;%d;%s", _nodeId, sensor->id, sensor->flags, sensor->unit, sensor->verbose, sensor->name);
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

void Arduilink::send(unsigned int _id, String &_msg) {
	char buf[_msg.length() + 1];
	_msg.toCharArray(buf, _msg.length() + 1);
	send(_id, buf);
}

void Arduilink::setFailure(unsigned int _id, const char* _msg) {

}

unsigned long Arduilink::getEncoded32(unsigned int _id) {
	SensorItem* sensor = getSensor(_id);
	if (sensor == NULL) return 0;
	return (nodeId * 10000000) + (sensor->id * 100000) + min(String(sensor->value).toInt(), 99999);
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

		// GET - Récupérer la valeur d'un capteur
		if (strcmp(opcode, "GET") == 0) {
			char buff[256];
			sprintf(buff, "200;%d;%d;", node, sensor->id);
			Serial.print(buff);
			Serial.println(sensor->value);
		}

		// SET - Modifier un attribut d'un capteur
		else if (strcmp(opcode, "SET") == 0) {
			
			// Choose attribute mode
			pch = strtok(NULL, ";");
			int mode = 0;
			if (strcmp(pch, "VERBOSE") == 0) mode = 1;
			else if (strcmp(pch, "VAL") == 0) mode = 2;
			else {
				Serial.print("400;ATTR;");
				Serial.println(pch);
				write = false;
				return 5;
			}

			// Ack
			pch = strtok(NULL, ";");
			bool ack = strcmp(pch, "1") == 0;
			
			// Value
			pch = strtok(NULL, ";");

			// Handle Set Verbose
			if (mode == 1) {
				if (strcmp(pch, "1") == 0) {
					if (ack) {
						Serial.println("201;VERBOSE;1");
						Serial.flush();
					}
					sensor->verbose = true;
				}
				else if (strcmp(pch, "0") == 0) {
					sensor->verbose = false;
					if (ack) {
						Serial.println("201;VERBOSE;0"); // TODO Ameliorer avec ids
						Serial.flush();
					}
				}
				else {
					Serial.print("400;OPT;VERBOSE;"); // TODO Ameliorer avec ids
					Serial.println(pch);
					write = false;
					return 6;
				}
			}

			// Handle Set Value
			else if (mode == 2) {
				sensor->value = pch;
				if (sensor->writter != NULL)
					sensor->writter(pch);
				if (ack) {
					Serial.print("201;SET;");
					Serial.print(node);
					Serial.print(";");
					Serial.print(sensor->id);
					Serial.print(";VAL;");
					Serial.println(pch);
				}
			}
			
		}

		Serial.flush();
		write = false;
		return 0;
	}
	return 1;
}
