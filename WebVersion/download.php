<?php
	if (!empty($_GET['file'])) {
		// Security, down allow to pass ANY PATH in your server
    		$fileName = basename($_GET['file']);
		echo $fileName;
	} else {
    		return;
	}
	$filePath = 'files/Images/' . $fileName;
	if (!file_exists($filePath)) {
    		return;
	}

	header('Content-Description: File Transfer');
	header('Content-Type: application/octet-stream');
	header('Content-Disposition: attachment; filename='.$fileName);
	header('Expires: 0');
	header('Cache-Control: must-revalidate');
	header('Pragma: public');
	header('Content-Length: ' . filesize($filePath));
	ob_clean();
	flush();
	//header("Content-Disposition: attachment; filename=$fileName");
	//header("Content-type: image/png");
	readfile($filePath);
	exit
?>
