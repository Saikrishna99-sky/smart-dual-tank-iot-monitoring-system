const char HTML2[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Wittelb-4G-Gateway</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #efefef;
      margin: 0;
      padding: 0;
    }
    .navbar {
      width: 100%;
      height: 50px;
      margin: 0;
      padding: 10px 0px;
      background-color: #FF7F00;
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
    .container {
      max-width: 800px;
      margin: 100px auto 20px;
      background-color: #fff;
      padding: 30px;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
    }
    h2 {
      text-align: center;
      color: #333;
      margin-bottom: 30px;
    }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 5px;
      font-weight: bold;
      color: #555;
    }
    input[type="text"],
    input[type="password"],
    input[type="number"] {
      width: 100%;
      padding: 10px;
      border: 2px solid #ddd;
      border-radius: 4px;
      font-size: 14px;
      box-sizing: border-box;
    }
    input:focus {
      border-color: #FF7F00;
      outline: none;
    }
    button {
      width: 100%;
      background-color: #4CAF50;
      color: white;
      padding: 12px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
      margin-top: 20px;
    }
    button:hover {
      background-color: #45a049;
    }
    
    /* Toggle Switch Styles */
    .switch-container {
      background-color: #f9f9f9;
      padding: 20px;
      border-radius: 8px;
      margin-bottom: 25px;
      border: 2px solid #FF7F00;
    }
    .switch-label {
      font-size: 18px;
      font-weight: bold;
      color: #333;
      margin-bottom: 15px;
      display: block;
    }
    .switch-wrapper {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 20px;
    }
    .switch-text {
      font-size: 16px;
      font-weight: bold;
      color: #666;
    }
    .switch-text.active {
      color: #4CAF50;
    }
    
    /* The switch - the box around the slider */
    .switch {
      position: relative;
      display: inline-block;
      width: 80px;
      height: 40px;
    }
    
    /* Hide default HTML checkbox */
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    
    /* The slider */
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #2196F3;
      transition: .4s;
      border-radius: 34px;
    }
    
    .slider:before {
      position: absolute;
      content: "";
      height: 32px;
      width: 32px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }
    
    input:checked + .slider {
      background-color: #FF7F00;
    }
    
    input:checked + .slider:before {
      transform: translateX(40px);
    }
    
    .network-info {
      text-align: center;
      margin-top: 10px;
      font-size: 14px;
      color: #666;
    }
  </style>
</head>
<body onload="loadConfig()">
  <header>
    <div class="navbar fixed-top">
      <div>
        <div class="navtitle">Wittelb 4G Gateway Configuration</div>
        <br>
        <ul style="margin-top:40px">
          <li><a href="cpp1">Network Configuration</a></li>
          <li><a href="cpp2">Serial Port</a></li>
          <li><a class="active" href="cpp3">Cloud Platform</a></li>
          <li><a href="cpp4">Modbus Master</a></li>
          <li><a href="cpp5">I/O Config</a></li>
          <li><a  href="cpp6">IO Control </a></li>
          <li style="float:right"><a href="www.wittelb.com">About</a></li>
        </ul>
      </div>
    </div>
  </header>

  <div class="container">
    <h2>Cloud Platform Configuration</h2>
    
    <!-- Network Mode Switch -->
    <div class="switch-container">
      <label class="switch-label">Network Mode</label>
      <div class="switch-wrapper">
        <span class="switch-text" id="wifi-text">WiFi</span>
        <label class="switch">
          <input type="checkbox" id="network_mode" checked>
          <span class="slider"></span>
        </label>
        <span class="switch-text active" id="4g-text">4G</span>
      </div>
      <div class="network-info">
        <span id="network-status">Current Mode: <strong>4G Network</strong></span>
      </div>
    </div>
    
    <form action="/cluod" method="POST" id="cloudForm">
      <input type="hidden" name="network_mode" id="network_mode_value" value="4g">
      
      <div class="form-group">
        <label for="domain">Host IP or Domain:</label>
        <input type="text" id="domain" name="Host IP or Domain" 
               placeholder="e.g., broker.emqx.io" required>
      </div>

      <div class="form-group">
        <label for="port">Port:</label>
        <input type="number" id="port" name="Port" 
               placeholder="e.g., 1883" required>
      </div>

      <div class="form-group">
        <label for="pubtopic">Publish Topic:</label>
        <input type="text" id="pubtopic" name="Publish Topic" 
               placeholder="e.g., device/data" required>
      </div>

      <div class="form-group">
        <label for="pubtopic_sd">Publish Topic (SD Card):</label>
        <input type="text" id="pubtopic_sd" name="Publish Topic_sd" 
               placeholder="e.g., device/sd_data" required>
      </div>

      <div class="form-group">
        <label for="subtopic">Subscribe Topic:</label>
        <input type="text" id="subtopic" name="Subscribe Topic" 
               placeholder="e.g., device/control" required>
      </div>

      <div class="form-group">
        <label for="username">User Name:</label>
        <input type="text" id="username" name="User Name" 
               placeholder="MQTT Username">
      </div>

      <div class="form-group">
        <label for="password">Password:</label>
        <input type="password" id="password" name="Password" 
               placeholder="MQTT Password">
      </div>

      <button type="submit">Save Configuration</button>
    </form>
  </div>

  <script>
    // Toggle switch functionality
    const networkSwitch = document.getElementById('network_mode');
    const wifiText = document.getElementById('wifi-text');
    const fourGText = document.getElementById('4g-text');
    const networkStatus = document.getElementById('network-status');
    const networkModeValue = document.getElementById('network_mode_value');

    networkSwitch.addEventListener('change', function() {
      if (this.checked) {
        // 4G Mode
        fourGText.classList.add('active');
        wifiText.classList.remove('active');
        networkStatus.innerHTML = 'Current Mode: <strong>4G Network</strong>';
        networkModeValue.value = '4g';
      } else {
        // WiFi Mode
        wifiText.classList.add('active');
        fourGText.classList.remove('active');
        networkStatus.innerHTML = 'Current Mode: <strong>WiFi Network</strong>';
        networkModeValue.value = 'wifi';
      }
    });

    // Load saved configuration
    function loadConfig() {
      fetch('/confi')
        .then(response => response.json())
        .then(data => {
          // Populate form fields
          if (data.Domain) document.getElementById('domain').value = data.Domain;
          if (data.Po) document.getElementById('port').value = data.Po;
          if (data.PT0) document.getElementById('pubtopic').value = data.PT0;
          if (data.PT1) document.getElementById('pubtopic_sd').value = data.PT1;
          if (data.ST) document.getElementById('subtopic').value = data.ST;
          if (data.UN) document.getElementById('username').value = data.UN;
          if (data.Pas) document.getElementById('password').value = data.Pas;
          
          // Set network mode
          if (data.network_mode) {
            if (data.network_mode === '4g') {
              networkSwitch.checked = true;
              fourGText.classList.add('active');
              wifiText.classList.remove('active');
              networkStatus.innerHTML = 'Current Mode: <strong>4G Network</strong>';
              networkModeValue.value = '4g';
            } else {
              networkSwitch.checked = false;
              wifiText.classList.add('active');
              fourGText.classList.remove('active');
              networkStatus.innerHTML = 'Current Mode: <strong>WiFi Network</strong>';
              networkModeValue.value = 'wifi';
            }
          }
        })
        .catch(error => {
          console.error('Error loading configuration:', error);
        });
    }
  </script>
</body>
</html>
)=====";
