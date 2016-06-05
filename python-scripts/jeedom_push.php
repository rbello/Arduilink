<?php

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
