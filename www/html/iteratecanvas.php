<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html lang="en">
<head>

   <meta http-equiv="X-UA-Compatible" content="IE=Edge">

   <title>Demonstrator of online usage of JSROOT</title>

   <!--  load JSROOT with 2D graphic -->
   <script src="http://root.cern.ch/js/3.6/scripts/JSRootCore.js?2d&onload=getOutput" type="text/javascript"></script>

   <script type='text/javascript'>

function sleep(milliseconds) {
  var start = new Date().getTime();
  for (var i = 0; i < 1e7; i++) {
    if ((new Date().getTime() - start) > milliseconds){
      break;
    }
  }
}
      
function getOutput() {
  // sleep(30000);

   setInterval(function(){
 getRequest(
      'writer.php', // URL for the PHP file
       drawOutput,  // handle successful request
       drawError    // handle error
  );
 
  return true;


   },30000);
 
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
    startGUI();
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
                return true;
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
      var addr = null;
         



          
           




      function updateGUI() {

         // here set of generated json files are used
         // One could specify addres of running THttpServer like http://localhost:8080/Canvases/c1/root.json.gz?compact=3
         // Or one can create root.json file in the application and place it on the webserver 
         // "dummy=xxxx" parameter used to prevent browser cashing
         // To run demo, one should generate rootXX.json files using demo.C macro

         var request_addr = "root0.json";
         var request_addr2 = "root1.json";

         if (addr!=null) request_addr = addr; 

         var req = JSROOT.NewHttpRequest(request_addr, 'object', function(histo) {
            if (!histo) {
               d3.select('#drawing').html("<h3>Can not get " + request_addr + " from the server</h3>"); 
               return;
            }
            if (mdi!=null) {
               mdi.Draw('abstract_item_name_', histo, drawopt); 
            } else { 
               JSROOT.redraw('drawing', histo, drawopt);
            } 
          });

         
          req.send(null); 
           if (addr!=null) request_addr2 = addr; 

         var req = JSROOT.NewHttpRequest(request_addr2, 'object', function(histo) {
            if (!histo) {
               d3.select('#drawing2').html("<h3>Can not get " + request_addr2 + " from the server</h3>"); 
               return;
            }
            if (mdi!=null) {
               mdi.Draw('abstract_item_name_', histo, drawopt); 
            } else { 
               JSROOT.redraw('drawing2', histo, drawopt);
            } 
          });

         
          req.send(null); 
      }

      function startGUI() {
         d3.select('html').style('height','100%');
         d3.select('body').style({'min-height':'100%', 'margin':'0px', "overflow" :"hidden"});

         var monitor = "";  //JSROOT.GetUrlOption("monitoring");
         if ((monitor == "") || (monitor==null)) 
            monitor = 1000;
         else
            monitor = parseInt(monitor);

         drawopt = JSROOT.GetUrlOption("opt");

         addr = JSROOT.GetUrlOption("addr");

         var layout = JSROOT.GetUrlOption("layout");
         if (layout!=null) mdi = new JSROOT.GridDisplay('drawing', layout);   

        // setInterval(updateGUI, monitor);
        updateGUI();

         JSROOT.RegisterForResize('drawing');
      }
   </script>

</head>
<!-- 
<body>
   <div id="drawing" style="position:absolute; left:1px; top:1px; bottom:1px; right:1px"></div>
</body> -->
<body>
<div style="width:1800 ">
  <div id="drawing"  style="width:800px; height:600px; float:left; "></div>
 <div id="drawing2"  style="width:800px; height:600px; float:left;" ></div>
</div>

</html>