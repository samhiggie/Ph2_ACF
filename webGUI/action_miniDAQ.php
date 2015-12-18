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
$formatted="miniDAQ";
$file=fopen("initialize.sh","w") or die("Unable to open file!");
$vcth;




if(isset($_POST['parallel']))
$formatted=$formatted." -p";

if($_POST['events']!="")
$formatted=$formatted." -e ".$_POST['events'];

if($_POST['ith']!="")
$formatted=$formatted." -d ".$_POST['events'];

$formatted=$formatted." -f ".$_POST['Hw_Description_File_'];



echo "<br/>";

fwrite($file, "#!/bin/bash"."\n"."cd ../"."\n"."source "."$(pwd)/setup.sh" ."\n".$formatted);
fclose($file);


$command =  preg_replace('/\s+/', '', $current_path.'/initialize.sh');
$handle = popen("source ".$command, "r");
$continue=TRUE;
$count=10;
print "<pre>";
while ((!feof($handle))&& $continue) {
    $data = fgets($handle);
    print $data;
    // echo $continue;

     //   if($count>65)
     //     $continue=FALSE;
     // echo $count;
     // $count++;
      
  	// if(isset($_POST['exec1'])==TRUE){
  	// BREAK;
   // }
   
}
print "</pre>";

//shell_exec("kill $(pidof calibrate)");


// $command =  preg_replace('/\s+/', '', $current_path.'/initialize.sh');
//  exec("source ".$command, $output);
//   foreach ( $output as $item ) {
//         echo $item . "<br/>";
//     }

?>

</body>
</html>  
