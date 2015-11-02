<html>
<body>


<?php
ob_implicit_flush(true);
ob_end_flush();
set_include_path(get_include_path() . PATH_SEPARATOR . 'amassett/Ph2_ACF/www/html');
error_reporting(E_ALL);
ini_set('display_errors', 1);
 
$current_path=shell_exec("pwd");
$command=null;
//This is the string that calls the binary file
$formatted="fpgaconfig";
$file=fopen("initialize.sh","w") or die("Unable to open file!");

if(isset($_POST['image']))
$formatted=$formatted." -i ".$_POST['image'];

if(isset($_POST['jump']))
$formatted=$formatted." -j ".$_POST['jump'];

if($_POST['bitstream']!="")
$formatted=$formatted." -f ".$_POST['bitstream'];





echo "<br/>";
echo $formatted;
//write the shell script setup.sh
fwrite($file, "#!/bin/bash"."\n"."cd ../"."\n"."source "."$(pwd)/setup.sh" ."\n".$formatted);
fclose($file);

//delete the spaces in the string and save the command for run the shell script in a variable
$command =  preg_replace('/\s+/', '', $current_path.'/initialize.sh');
$handle = popen("source ".$command, "r");
$continue=TRUE;

//this part is os for printing in real time
print "<pre>";
while ((!feof($handle))&& $continue) {
    $data = fgets($handle);
    print $data;
   
   
}
print "</pre>";



?>

</body>
</html>  
