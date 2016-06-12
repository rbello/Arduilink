<?php

list($opcode, $nodeId, $sensorId, $sensorValue) = explode(';', $argv[2], 4);

require_once '../../../../core/php/core.inc.php';

// Fetch equipments
foreach (eqLogic::byType('script') as $eq) {
	
	// Fetch commands
	foreach ($eq->getCmd('info') as $cmd) {
		if ($cmd->getId() == $argv[1]) {
			echo "Pushed value $sensorValue for sensor Arduino[$nodeId:$sensorId] -> {$eq->getName()} ({$cmd->getName()})\n";
			$cmd->setCollectDate('');
			$cmd->event(intval($sensorValue));
			return;
		}
	}
	
}

echo "Error: eqLogic device {$argv[1]} not found\n";

?>
