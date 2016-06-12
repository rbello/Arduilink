<?php

list($opcode, $nodeId, $sensorId, $sensorValue) = explode(';', $argv[2], 4);

echo "Record $sensorValue for sensor Arduino[$nodeId:$sensorId] -> Jeedom[{$argv[1]}]\n";

return;

require_once '../../../../core/php/core.inc.php';

foreach (eqLogic::byType('script') as $eq) {
	//echo "Equipment: " . $eq->getName() . "\n";
	foreach ($eq->getCmd('info') as $cmd) {
		//echo "\tCommand: " . $cmd->getName() . "\n";
		if ($cmd->getName() != "Température")
			continue;
		$cmd->setCollectDate('');
		$cmd->event(66);
	}
}

?>
