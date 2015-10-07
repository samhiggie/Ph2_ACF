

<?php
$content = file_get_contents("http://127.0.0.1:8082/Canvases/Fe0_Cbc0_Calibration/root.json");
$content1 = file_get_contents("http://127.0.0.1:8082/Canvases/Fe0_Cbc1_Calibration/root.json");
$file="root0.json";
$file1="root1.json";

file_put_contents($file, $content);
file_put_contents($file1, $content1);
?>