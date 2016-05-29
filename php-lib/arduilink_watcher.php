<?php

include 'Arduilink.php';

$lnk = new Arduilink\Watcher();

$lnk->addSensorListener(function ($sensor, $state) {
});

$lnk->addDataListener(function ($sensor, $value) {
	echo "Data $sensor = $value\n";
});

$lnk->run();

?>