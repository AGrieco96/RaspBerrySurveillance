<?php
   	// Security, down allow to pass ANY PATH in your server
   	if (!empty($_POST['src'])) {
    	// Security, down allow to pass ANY PATH in your server
    	$angle = basename($_POST['src']);
	} else {
    	return;
	}
	echo $angle;
	$cmd = exec("sudo python2 /var/www/html/setAngle.py ".$angle);
	//$cmd = shell_exec('ls');
	echo $cmd;
	echo $angle
?>
