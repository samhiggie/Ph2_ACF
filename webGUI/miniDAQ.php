<!DOCTYPE html>
<html>
<head>
<title>Border Tabs - Css</title>
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<meta name="generator" content="HAPedit 3.0">
<style type="text/css">
html,body{margin:0;padding:0}
body{font: 100.01% "Trebuchet MS",Arial,sans-serif}
div#header{background-color:#9CF}
div#header h1{margin:0;line-height:70px;margin-left:20px}

div#navigation{background-color: #9cf;border-bottom: 1px solid #787878;padding-left: 20px}
div#navigation ul{list-style-type: none;margin: 0;padding: 0;white-space: nowrap}
div#navigation li{display: inline;margin: 0;padding: 0}
div#navigation li a{text-decoration: none;border: 1px solid #787878;padding: 0px 0.3em;
    background: #ccc;color: #036}
div#navigation li a:hover{background-color: #f0f0f0}
div#navigation li#activelink a{border-bottom: 1px solid #fff;background-color: #fff;color: #603}
div#allign{
	position: absolute;
    right: 600px;
    }
</style>
</head>
<body>
<div id="header"><h1></h1></div>
<div id="navigation">
<ul>
   
    <li><a href="fpgaconfig.php">FPGAconfig</a></li>
    <li id="activelink"><a href="miniDAQ.php">MiniDAQ</a></li>
    <li><a href="datatest.php">Datatest</a></li>
    <li><a href="calibration.php">Calibration</a></li>
    <li><a href="commission.php">Commission</a></li>
</ul>
</div>



<div id="allign">
 <br>
      <iframe id="frame" name="my_iframe"  height="520" width="1100" allign="right"  frameBorder="0" ></iframe>
</div>
<form action="action_miniDAQ.php" method=post  target="my_iframe">

<br><br>
<input type="checkbox" name="parallel" value="yes">Acquisition running in a separate thread
<br><br>
Hw Description File:<input type="textfield" name="Hw Description File " value="settings/Calibration2CBC.xml">
<br><br>
Number of events:<input type="textfield" name="events" value="10" >
<br><br>
Print every i-th event:<input type="textfield" name="ith" value="10" >
<br><br>

<input type="submit" name="exec" value="Submit">

<br><br>

</form> 
<form action="" method="GET">
<input type="submit" value="STOP" name="STOP">
</form>

<?php 
   
   if (isset($_GET["STOP"]))
   {
      shell_exec("kill $(pidof minidaq)");
      
    }
?>
<!-- <form action="stop.php" method=get >
  <input type="submit" name="stop" value="STOP">
</form> -->
<script type="text/javascript" ></script>
    <script>
      $(document).ready(function(){
        var locations = ["", "miniDAQ.php"];
        var len = locations.length;
        var iframe = $('#frame');
        var i = 0;
        setInterval(function () {
            iframe.attr('src', locations[++i % len]);
        }, 30000);
      });
    </script>


</body>
</html> 
