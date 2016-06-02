<?php

namespace Arduilink;

//chmod("/dev/ttyUSB0", 777);

class Watcher {

	protected $file;
	protected $sensors;
	protected $dataListeners;
	
	public function __construct($file = "/dev/ttyUSB0") {
		$this->file = $file;
		$this->sensors = array();
		$this->dataListeners = array();
	}
	
	public function addDataListener(\Closure $callback) {
		$this->dataListeners[] = $callback;
	}
	
	public function addSensorListener(\Closure $callback) {
		
	}
	
	public function notifyDataListeners(Sensor $sensor, $value) {
		foreach ($this->dataListeners as &$callback)
			$callback($sensor, $value);
	}
	
	public function run() {
		
		$handle = fopen("/dev/ttyUSB0", "r");
		
		if (!$handle) return false;
		
		while (($line = fgets($handle)) !== false) {
			
			$opcode = substr($line, 0, 2);
			$tokens = explode(";", rtrim(substr($line, 2)));
			list($nodeId, $sensorId,) = $tokens;
			
			// Sensors presentation
			if ($opcode == "S;") {
				if (array_key_exists($sensorId, $this->sensors)) {
					echo "Error: sensor allready exists: " . $this->sensors[$sensorId] . "\n";
					continue;
				}
				$sensor = new Sensor($nodeId, $sensorId, $tokens[2], $tokens[3]);
				$this->sensors[$sensorId] = $sensor;
				echo "New sensor detected: $sensor\n";
				$sensor = null;
			}
			
			// Data record
			else if ($opcode == "D;") {
				if (!array_key_exists($sensorId, $this->sensors)) {
					echo "Warning: sensor $nodeId:$sensorId is not declared\n";
					continue;
				}
				$sensor = $this->sensors[$sensorId];
				$value = $sensor->setValue($tokens[2]);
				$this->notifyDataListeners($sensor, $value);
				//echo "Value $sensor = $value\n";
				$sensor = null;
			}
			
			// Data record response
			else if ($opcode == "V;") {
				if (!array_key_exists($sensorId, $this->sensors)) {
					echo "Warning: sensor $nodeId:$sensorId is not declared\n";
					continue;
				}
				$sensor = $this->sensors[$sensorId];
				$sensorType = $tokens[2];
				$sensorName = $tokens[3];
				$sensorValue = $sensor->setValue($tokens[4]);
				
				$this->notifyDataListeners($sensor, $sensorValue);
				echo "Value get $sensor = $sensorValue\n";
				$sensor = null;
			}
			
		}
		
		fclose($handle);
		return true;

	}

}

class Arduilink {
	
	public static function getSensorInfo($sensorId) {
		$handle = fopen("/dev/ttyUSB0", "rw");
		fwrite($handle, "G;0;1\n");
		flush($handle);
		while (($line = fgets($handle)) !== false) {
			echo $line;
		}
		fclose($handle);
	}
	
}

class Sensor {
	
	protected $nodeId;
	protected $sensorId;
	protected $sensorName;
	protected $sensorType;
	protected $value = NULL;
	protected $last = 0;
	protected $state;
	
	const STATE_ON = 0;
	const STATE_OFF = 1;
	const STATE_FAILURE = 2;
	
	public function __construct($nodeId, $sensorId, $sensorName, $sensorType) {
		$this->nodeId = intval($nodeId);
		$this->sensorId = intval($sensorId);
		$this->sensorName = trim($sensorName);
		$this->sensorType = intval($sensorType);
		$this->state = self::STATE_ON;
	}
	
	public function setValue($str) {
		if ($this->state != self::STATE_ON) {
			// TODO Event
			$this->state = self::STATE_ON;
		}
		$this->value = $str;
		$this->last = time();
		return $this->value;
	}
	
	//public function stateName() {
	//	if ($this->state )
	//}
	
	public function __toString() {
		return sprintf("Sensor %d %s (%d:%d)", $this->sensorType, $this->sensorName, $this->nodeId, $this->sensorId);
	}
	
}

?>