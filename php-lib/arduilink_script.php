<?php

if (sizeof($argv) < 2) {
	echo "MissingSensorIdArgument";
	return;
}

$sensorId = $argv[1];

include 'Arduilink.php';

$sensor = Arduilink::getSensorInfo($sensorId);

?>
