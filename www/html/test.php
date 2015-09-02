<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html lang="en">
<head >

   <meta http-equiv="X-UA-Compatible" content="IE=Edge">

   <title>Demonstrator of online usage of JSROOT</title>

   <!--  load JSROOT with 2D graphic -->
   <script src="http://root.cern.ch/js/3.6/scripts/JSRootCore.js?2d" type="text/javascript"></script>

   <script type='text/javascript'>
function getOutput() {
   setInterval(function(){
 getRequest(
      'prova1.php', // URL for the PHP file
       drawOutput,  // handle successful request
       drawError    // handle error
  );
  createGUI();
  return true;


   },3000);
 
}  
// handles drawing an error message
function drawError() {
    var container = document.getElementById('output');
    container.innerHTML = 'Bummer: there was an error!';
}
// handles the response, adds the html
function drawOutput(responseText) {
    var container = document.getElementById('output');
    container.innerHTML = responseText;
}
// helper function for cross-browser request object
function getRequest(url, success, error) {
    var req = false;
    try{
        // most browsers
        req = new XMLHttpRequest();
    } catch (e){
        // IE
        try{
            req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch(e) {
            // try an older version
            try{
                req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch(e) {
                return false;
            }
        }
    }
    if (!req) return false;
    if (typeof success != 'function') success = function () {};
    if (typeof error!= 'function') error = function () {};
    req.onreadystatechange = function(){
        if(req.readyState == 4) {
            return req.status === 200 ? 
                success(req.responseText) : error(req.status);
        }
    }
    req.open("GET", url, true);
    req.send(null);
   
    return req;
}


      var mdi = null;
      var cnt = 0;
      var drawopt = null;
     
     //  var request_addr = "http://127.0.0.1:8082/Canvases/Fe0_Cbc0_Calibration/root.json";


      function createGUI() {
         // json file stored in same folder, absolute address can be used as well
           
         var addr = "provami1.json"
    var addr2 = "provami2.json"
        var req = JSROOT.NewHttpRequest(addr, 'object', function(obj) {
          JSROOT.draw("drawing", obj, "hist");
        });

         req.send(null);
      var req = JSROOT.NewHttpRequest(addr2, 'object', function(obj) {
          JSROOT.draw("drawing2", obj, "hist");
        });
         req.send(null);
      
      }
   </script>
</head>

<body>
<div style="width:1800 ">
  <div id="drawing"  style="width:800px; height:600px; float:left; "></div>
 <div id="drawing2"  style="width:800px; height:600px; float:left;" ></div>
</div>

</body>
<a href="#" onclick="return getOutput();"> test </a>
<div id="output">waiting for action</div>
</html>
