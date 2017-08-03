<?php

require_once '/var/www/html/core/php/core.inc.php';

$i = 0;

// Fetch equipments
foreach (eqLogic::byType('script') as $eq) {
	// Fetch commands
	foreach ($eq->getCmd('info') as $cmd) {
		echo "Device='{$eq->getName()}' Command='{$cmd->getName()}' ID={$cmd->getId()}\n";
		$i++;
	}
}

echo "Found: {$i} command(s)\n"

?>