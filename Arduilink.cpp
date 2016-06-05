#include <Arduilink.h>
#include <stdio.h>
#include <string.h>

Arduilink::Arduilink(unsigned int _id) {
	nodeId = _id;
	head = queue = 0;
	sensorsCount = 0;
}

void Arduilink::init() {
	Serial.print("100 welcome arduilink-v1.0");
	Serial.println(nodeId);
	Serial.flush();
	// Indique au client que l'arduino est prêt à fonctionner
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
	sensor->verbose = false;
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
	sensor->value = String(_value);
	// Ouput
	if (sensor->verbose == true) {
		//if (strncmp(_value, sensor->value, strlen(_value)) == 0) return;
		//if (strcmp(copy, sensor->value) == 0) return;
		char buff[256];
		sprintf(buff, "200 data %d %d %s\n", nodeId, _id, _value);
		Serial.print(buff);
		Serial.flush();
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
	sprintf(buff, "300 desc sensor %d %d %d %d %s", _nodeId, sensor->id, sensor->type, sensor->verbose, sensor->name);
	Serial.println(buff);
}

void Arduilink::printSensors() {
	SensorItem* sensor = head;
	while (sensor != NULL) {
		printSensor(sensor, nodeId);
		sensor = sensor->next;
	}
	// On ajoute une ligne vide pour signifier la fin
	Serial.println();
	Serial.flush();
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
	while (Serial.available() > 0) {
		
		String str = Serial.readString();

		char buf[str.length() + 1];
		str.toCharArray(buf, str.length() + 1);

		char* opcode = NULL;
		int node = -1;
		int sensorId = -1;

		char* pch;
		pch = strtok(buf, " ");
		while (pch != NULL)
		{
			if (opcode == NULL) {
				if (strcmp(pch, "present") == 0) {
					printSensors();
					return 0;
				}
				if (strcmp(pch, "get") != 0 && strcmp(pch, "info") != 0 && strcmp(pch, "set") != 0) {
					Serial.print("400 invalid opcode ");
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
				break;
			}
			pch = strtok(NULL, " ");
		}

		/*Serial.print("Opcode: ");
		Serial.print(opcode);
		Serial.print(" NodeId: ");
		Serial.print(node);
		Serial.print(" SensorId: ");
		Serial.println(sensorId);*/

		if (nodeId != node) {
			Serial.print("404 notfound node ");
			Serial.println(node);
			return 3;
		}

		SensorItem* sensor = getSensor(sensorId);
		if (sensor == NULL) {
			Serial.print("404 notfound sensor ");
			Serial.println(sensorId);
			return 4;
		}

		// INFO - Récupérer la description d'un capteur
		if (strcmp(opcode, "info") == 0) {
			printSensor(sensor, node);
		}

		// SET - Modifier un attribut d'un capteur
		if (strcmp(opcode, "set") == 0) {
			pch = strtok(NULL, " ");
			if (strcmp(pch, "verbose") == 0) {
				pch = strtok(NULL, " ");
				if (strcmp(pch, "on") == 0) {
					Serial.println("201 set on"); // TODO Ameliorable
					Serial.flush();
					sensor->verbose = true;
				}
				else if (strcmp(pch, "off") == 0) {
					sensor->verbose = false;
					Serial.println("201 set off"); // TODO Ameliorable
					Serial.flush();
				}
				else {
					Serial.print("400 invalid value ");
					Serial.println(pch);
					return 6;
				}
			}
			else {
				Serial.print("400 invalid option ");
				Serial.println(pch);
				return 5;
			}
		}

		// GET - Récupérer la valeur d'un capteur
		if (strcmp(opcode, "get") == 0) {
			char buff[256];
			sprintf(buff, "200 data %d %d ", node, sensor->id);
			Serial.print(buff);
			Serial.println(sensor->value);
		}

		Serial.flush();

		return 0;
	}
	return 1;
}