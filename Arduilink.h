//#include "utility/Key.h"


//typedef char KeypadEvent;

typedef struct Sensor {
	unsigned int id;
	char* name;
	Sensor* next;
};

#define LIST_MAX 10		// Max number of keys on the active list

class Arduilink {
public:

	Arduilink(unsigned int _id, void(*_serial)(const char *));

	int addSensor(unsigned int _id, char* _name);

	//void setValue(unsigned int _id, long _value);
	void setValue(unsigned int _id, char* _value);

	void printSensors();

private:
	unsigned int id;
	void(*serial)(const char *fmt);
	Sensor *head;
	Sensor *queue;
	int sensorsCount;

};
