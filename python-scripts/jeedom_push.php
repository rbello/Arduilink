<?php

// Check arguments
if (sizeof($argv) < 3) {
	echo "Usage: jeedom_push.php <eqLogicID> <value>\n";
	return;
}

// Extract arguments
$eqLogicID = $argv[1];
$value = $argv[2];

// Cast value
if (ctype_digit($value)) $value = intval($value);
else if (is_numeric($value)) $value = floatval($value);
else if (strtolower($value) == 'true') $value = 1;
else if (strtolower($value) == 'false') $value = 0;
else if (strtolower($value) == 'null') $value = null;

// Load jeedom core
require_once '/var/www/html/core/php/core.inc.php';

// Fetch equipments
foreach (eqLogic::byType('script') as $eq) {
	
	// Fetch commands
	foreach ($eq->getCmd('info') as $cmd) {
		if ($cmd->getId() != $eqLogicID) continue;
		
		echo date('[Y/m/d H:i:s]') . ' Pushed value ' . var_export($value, true) . " to virtual sensor {$eq->getName()}->{$cmd->getName()} ({$eqLogicID})\n";
		
		$cmd->setCollectDate('');
		$cmd->event($value);
		
		return;
		
	}
	
}

echo "Error: eqLogic device {$argv[1]} not found\n";

?>