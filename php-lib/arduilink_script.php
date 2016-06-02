<?php

if (sizeof($argv) < 2) {
	echo "MissingSensorIdArgument";
	return;
}

$sensorId = $argv[1];

include 'Arduilink.php';

// On charge la configuration de la BDD
include '../../../../core/config/common.config.php';

$sensor = Arduilink\Arduilink::getSensorInfo($sensorId);

?>
