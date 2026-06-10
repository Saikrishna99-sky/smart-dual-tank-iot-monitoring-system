 const char HTML[] PROGMEM = R"=====(
 <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wittelb 4G Gateway Configuration</title>
    
      <style>
    .navbar {
      width: 100%;
      height: 50px;
      margin: 0;
      padding: 10px 0px;
      background-color:   #FF7F00;
      color: #000000;
      border-bottom: 5px solid #293578;
    }
    .fixed-top {
      position: fixed;
      top: 0;
      right: 0;
      left: 0;
      z-index: 1030;
    }
    .navtitle {
      float: left;
      height: 30px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 30px;
      font-weight: bold;
      line-height: 50px;
      padding-left: 20px;
    }
   .navheading {
     position: fixed;
     left: 65%;
     height: 30px;
     font-family: "Verdana", "Arial", sans-serif;
     font-size: 15px;
     font-weight: bold;
     line-height: 20px;
     padding-right: 20px;
   }
   .navdata {
      justify-content: flex-end;
      position: fixed;
      left: 71%;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 15px;
      font-weight: bold;
      line-height: 20px;
      padding-right: 20px;
   }
    .category {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: bold;
      font-size: 25px;
      line-height: 50px;
      padding: 20px 10px 0px 10px;
      color: #000000;
    }
    .heading {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: normal;
      font-size: 28px;
      text-align: left;
    }
  
    .button {
      background-color: #4CAF50;
      color: white;
      padding: 10px 15px;
      margin-top: 10px;
      border: none;
      cursor: pointer;
    }
    .foot {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      position: relative;
      height:   30px;
      text-align: center;   
      color: #AAAAAA;
      line-height: 20px;
    }
    .container {
      max-width:  2200px;
      margin: 0 auto;
    }
     .navbarb {
      width:100%;
      height: 32px;
      margin: 0;
      padding: 10px 0px;
      background-color: #FFFFFF;
      color: #000000;
      border-bottom: 1px solid #293578;
       border-top: 1px solid #293578;
       font-family: "Verdana", "Arial", sans-serif;
    }
       ul {
  list-style-type: none;
  margin: 0;
  padding: 0;
  overflow: hidden;
  background-color: #333;
}

li {
  float: left;
  border-right:1px solid #bbb;
}

li:last-child {
  border-right: none;
}

li a {
  display: block;
  color: white;
  text-align: center;
  padding: 14px 16px;
  font-family: "Verdana", "Arial", sans-serif;
  text-decoration: none;
}

li a:hover:not(.active) {
  background-color: #111;
}

.active {
  background-color:#04AA6D ;
}
    
    .font{
   font-family: "Verdana", "Arial", sans-serif;
    
    }
     div {
        margin-bottom: 10px;
      }
      label {
        display: inline-block;
        width: 100px;
        
      }
      input {
        padding: 5px 10px;
      }
    select{
        width: 185px;
        padding: 5px 10px;
      }
    
  </style>
</head>
<body>
 <body style="background-color: #efefef" onload="getSID()">
     <header>
         <div class="navbar fixed-top">
         <div class="container">
         <div  class="navtitle">Wittelb 4G Gateway Configuration </div>
         <br>
         
  <ul style="margin-top:40px">
  <li><a class="active" href="cpp1">Network Configuration</a></li>
   <li><a href="cpp2">Serial Port</a></li>
  <li><a href="cpp3">Cluod Platform</a></li>
  <li><a href="cpp4">Modbus Master</a></li>
  <li><a  href="cpp5">I/O Config </a></li>
  <li><a  href="cpp6">IO Control </a></li>
  <li style="float:right"><a href="www.wittelb.com">About</a></li>
</ul>

          </div>
      </div>
    </header>
    
   <!-- Network Configuration -->
     <main class="container" style="margin-top:100px">
      <div class="category">WiFi Configuration</div>
      <div style="border-radius: 10px !important;">
    <form method='POST' action='/save'>
    <label class="font" for="ssid" style="margin-top:20px" >SSID:</label>
       <input type='text' id="ssid" name='ssid' ><br>
       <label class="font" for="password" style="margin-top:20px" >Password:</label>
        <input type='password' name='password'style="margin-top:20px"><br>
        <label class="font" for="DeviceID" style="margin-top:20px" >Device ID:</label>
        <input type='DeviceID' id="DeviceID" name='DeviceID'style="margin-top:20px"><br>
        <label class="font" for="data_freq" style="margin-top:20px" >Data Frequancy:</label>
        <input type='data_freq' id="data_freq" name='data_freq'style="margin-top:20px"><br>
        <label for="quantity"> SD Data Upload Time:</label>
        <input type="number" id="quantity" name="quantity" min="1" max="23"style="margin-top:20px"><br>
       <input type='submit' value='Submit' style="margin-top:20px" class="button">         
    </form>

<script>
    function getSID() {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var myArr = JSON.parse(this.responseText);
            myFunction(myArr);
          }
        };
      xmlhttp.open("GET","confi" , true);
      xmlhttp.send();
    }    
        function myFunction(arr) {
         var SID = "";
         var DID = "";
         var DF = "";
         var QT = "";
         SID= arr["ssid"];
          DID= arr["DeviceID"];
          DF= arr["data_freq"];//
          QT= arr["quantity"];
         document.getElementById("DeviceID").value =DID;
         document.getElementById("data_freq").value =DF;
         document.getElementById("ssid").value =SID;
          document.getElementById("quantity").value =QT;
        }

   </script>
 
</body>
</html>



)=====";





 
