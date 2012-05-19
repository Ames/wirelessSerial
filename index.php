<html>
<head>
<title>Wireless Serial Proxy</title>
<link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
<?php

$fileName = "mainpage.text";
$fh = fopen($fileName, 'r');
$theText = fread($fh, filesize($fileName));
fclose($fh);

include_once "markdown.php";

echo Markdown($theText);

?>
</body>
</html>
