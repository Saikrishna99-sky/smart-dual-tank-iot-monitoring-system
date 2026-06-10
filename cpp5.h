const char HTML4[] PROGMEM = R"=====(
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
            background-color:  #FF7F00;
            color: #000000;
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
        .btn {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 13px;
            margin: 0px 0px;
            cursor: pointer;
            border-radius: 10px;
        }
        .foot {
            font-family: "Verdana", "Arial", sans-serif;
            font-size: 20px;
            position: relative;
            height: 30px;
            text-align: center;
            color: #AAAAAA;
            line-height: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background-color: #fff;
            padding: 20px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
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
            border-right: 1px solid #bbb;
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
            background-color: #04AA6D;
        }
        .font {
            font-family: "Verdana", "Arial", sans-serif;
        }
        div {
            margin-bottom: 10px;
        }
        label {
            display: inline-block;
            width: 200px;
        }
        input {
            padding: 5px 10px;
        }
        select {
            width: 185px;
            padding: 5px 10px;
        }
        body {
            font-family: Arial, sans-serif;
            background-color: #efefef;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin: 20px 0;
            text-align: center;
        }
        table, th, td {
            border: 2px solid #ddd;
        }
        th, td {
            padding: 10px;
        }
        th {
            background-color: #f2f2f2;
        }
        #popup {
            display: none;
            position: relative;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            background-color: #fff;
            padding: 30px;
            border: 1px solid #ccc;
            box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2);
            z-index: 1;
            width: 50%;
            max-width: 500px;
            margin-top: 250px;
        }
        input[type="text"] {
            width: 60%;
            padding: 8px;
            margin-bottom: 10px;
            border: 1px solid #ccc;
        }
        button {
            padding: 5px 10px;
            cursor: pointer;
        }
        .editButton {
            background-color: #4CAF50;
            color: white;
            border: none;
        }
        .editButton:hover {
            background-color: #45a049;
        }
        .deleteButton {
            background-color: #f44336;
            color: white;
            border: none;
        }
        .deleteButton:hover {
            background-color: #d32f2f;
        }
        .center-button {
            text-align: center;
            margin-top: 20px;
        }
        .value-cell {
            font-weight: bold;
            font-size: 16px;
        }
        .value-0 {
            color: #f44336;
        }
        .value-1 {
            color: #4CAF50;
        }
    </style>
</head>
<body onload="initialize()">
<header>
    <div class="navbar fixed-top">
        <div>
            <div class="navtitle">Wittelb 4G Gateway Configuration</div><br>
            <ul style="margin-top:40px" style="float:left">
                <li><a href="cpp1">Network Configuration</a></li>
                <li><a href="cpp2">Serial Port</a></li>
                <li><a href="cpp3">Cloud Platform</a></li>
                <li><a href="cpp4">Modbus Master</a></li>
                <li><a class="active" href="cpp5">I/O Config </a></li>
                <li><a  href="cpp6">IO Control </a></li>
                <li style="float:right"><a href="www.wittelb.com">About</a></li>
            </ul>
        </div>
    </div>
</header>
<main class="container" style="margin-top:100px">
    <div class="category">Digital Input Configuration (8 DI)</div>
    <form action="/takeName" method="POST">
        <table id="dataTable">
            <thead>
                <tr>
                    <th>Sr.No</th>
                    <th>Digital Input</th>
                    <th>Name/Label</th>
                    <th>Current Value</th>
                    <th>Status</th>
                </tr>
            </thead>
            <tbody>
                <!-- Generate 8 rows for Digital Inputs -->
                <script>
                    const diNames = ["DI1", "DI2", "DI3", "DI4", "DI5", "DI6", "DI7", "DI8"];
                    for (let i = 0; i <= 7; i++) {
                        document.write('<tr>');
                        document.write('<td>' + (i+1) + '</td>');
                        document.write('<td>' + diNames[i] + '</td>');
                        document.write('<td><input type="text" id="ioNI' + i + '" name="nameInput' + i + '" placeholder="Enter ' + diNames[i] + ' Name"></td>');
                        document.write('<td class="value-cell" id="ioValue' + i + '">-</td>');
                        document.write('<td id="ioStatus' + i + '">-</td>');
                        document.write('</tr>');
                    }
                </script>
            </tbody>
        </table>
        <div class="center-button">
            <input type='submit' value='Save Names' class="btn">
        </div>
    </form>
</main>
<script>
    function initialize() {
        getData();
        setInterval(updateValues, 500); // Update values every 500 milliseconds
    }
    
  function getData() {
  var xmlhttp = new XMLHttpRequest();
  xmlhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myArr = JSON.parse(this.responseText);
      populateForm(myArr);
    }
  };
  xmlhttp.open("GET", "confi", true);  // load → confi
  xmlhttp.send();
}


    function updateValues() {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                var myArr = JSON.parse(this.responseText);
                populateValues(myArr);
            }
        };
        xmlhttp.open("GET", "values", true);
        xmlhttp.send();
    }

 function populateForm(arr) {
  for (let i = 0; i <= 7; i++) {
    const nameElement = document.getElementById("ioNI" + i);
    if (nameElement && arr["ioName" + i]) {
      nameElement.value = arr["ioName" + i];  // ioNI → ioName
    }
  }
}
    function populateValues(arr) {
        for (let i = 0; i <= 7; i++) {
            const valueElement = document.getElementById("ioValue" + i);
            const statusElement = document.getElementById("ioStatus" + i);
            
            if (valueElement && arr["ioValue" + i] !== undefined) {
                const value = arr["ioValue" + i];
                valueElement.innerText = value;
                
                // Color coding based on value
                if (value == 1) {
                    valueElement.className = "value-cell value-1";
                    if (statusElement) statusElement.innerText = "HIGH";
                    if (statusElement) statusElement.style.color = "#4CAF50";
                } else if (value == 0) {
                    valueElement.className = "value-cell value-0";
                    if (statusElement) statusElement.innerText = "LOW";
                    if (statusElement) statusElement.style.color = "#f44336";
                } else {
                    valueElement.className = "value-cell";
                    if (statusElement) statusElement.innerText = "-";
                    if (statusElement) statusElement.style.color = "#666";
                }
            }
        }
    }
</script>
</body>
</html>
)=====";
