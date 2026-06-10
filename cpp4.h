const char HTML3[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Wittelb-4G-Gateway</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f0f0f0;
      margin: 0; padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container {
      background-color: #fff;
      padding: 100px 40px 40px 40px;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
      width: auto;
      max-width: 2200px;
      margin-top: 100px;
      margin-bottom: 40px;
    }
    h2 { text-align: center; }
    table {
      border-collapse: collapse;
      width: 100%;
      margin: 20px auto;
      text-align: center;
    }
    table, th, td { border: 1px solid #ddd; }
    th, td { padding: 10px; }
    th { background-color: #f2f2f2; }
    button {
      background-color: #4CAF50;
      color: white;
      padding: 10px 15px;
      margin-top: 10px;
      border: none;
      cursor: pointer;
    }
    button:hover { background-color: #45a049; }
    .delete { background-color: #f44336; }
    .delete:hover { background-color: #e53935; }
    .container1 { max-width: 2200px; margin: 0 auto; }
    .navbar {
      width: 100%; height: 50px;
      margin: 0; padding: 10px 0px;
      background-color: #FF7F00; color: #000000;
    }
    .fixed-top { position: fixed; top: 0; right: 0; left: 0; z-index: 1030; }
    .navtitle {
      float: left; height: 30px;
      font-family: "Verdana","Arial",sans-serif;
      font-size: 30px; font-weight: bold;
      line-height: 50px; padding-left: 20px;
    }
    ul { list-style-type:none; margin:0; padding:0; overflow:hidden; background-color:#333; }
    li { float:left; border-right:1px solid #bbb; }
    li:last-child { border-right:none; }
    li a {
      display:block; color:white; text-align:center;
      padding:14px 16px; font-family:"Verdana","Arial",sans-serif;
      text-decoration:none;
    }
    li a:hover:not(.active) { background-color:#111; }
    .active { background-color:#04AA6D; }
    input[type="text"] {
      padding: 5px; margin-bottom: 10px;
      border: 2px solid #ccc; width: 100px;
    }
    select {
      padding: 5px; border: 2px solid #ccc;
      background-color: #fff; font-size: 13px;
      cursor: pointer; min-width: 160px;
    }
    select:focus { outline: none; border-color: #4CAF50; }
    #statusMsg {
      text-align: center;
      font-weight: bold;
      padding: 8px;
      margin-top: 10px;
      display: none;
      border-radius: 4px;
    }
    .msg-ok  { background: #d4edda; color: #155724; }
    .msg-err { background: #f8d7da; color: #721c24; }
  </style>
</head>
<body style="background-color:#efefef" onload="getData()">
  <header>
    <div class="navbar fixed-top">
      <div class="container1">
        <div class="navtitle">Wittelb 4G Gateway Configuration</div>
        <br>
        <ul style="margin-top:40px">
          <li><a href="cpp1">Network Configuration</a></li>
          <li><a href="cpp2">Serial Port</a></li>
          <li><a href="cpp3">Cloud Platform</a></li>
          <li><a class="active" href="cpp4">Modbus Master</a></li>
          <li><a href="cpp5">I/O Config</a></li>
          <li><a  href="cpp6">IO Control </a></li>
          <li style="float:right"><a href="www.wittelb.com">About</a></li>
        </ul>
      </div>
    </div>
  </header>

  <div class="container">
    <h2>Modbus Master Configuration</h2>
    <div id="statusMsg"></div>
    <table id="dataTable">
      <thead>
        <tr>
          <th>Name</th>
          <th>Slave Address</th>
          <th>Register</th>
          <th>Function Code</th>
          <th>Data Type</th>
          <th>Actions</th>
        </tr>
      </thead>
      <tbody id="tableBody">
      </tbody>
    </table>
    <button type="button" onclick="addRow()">Add Row</button>
    <button type="button" onclick="submitForm()">Submit</button>
  </div>

  <script>
    // ── helper: ek naya <tr> banao ──────────────────────────────────────────
    function makeRow(name, sa, rg, fc, dt) {
      var tr = document.createElement('tr');
      tr.innerHTML =
        '<td><input type="text" placeholder="Name"      value="' + (name||'') + '"></td>' +
        '<td><input type="text" placeholder="Slave ID"  value="' + (sa  ||'') + '"></td>' +
        '<td><input type="text" placeholder="Reg No"    value="' + (rg  ||'') + '"></td>' +
        '<td><input type="text" placeholder="Func Code" value="' + (fc  ||'') + '"></td>' +
        '<td><select>' +
          '<option value="0">UINT16</option>' +
          '<option value="1">Float32 Big Endian</option>' +
          '<option value="2">Float32 Little Endian</option>' +
          '<option value="3">Float32 BE Byte Swap</option>' +
          '<option value="4">Float32 LE Byte Swap</option>' +
        '</select></td>' +
        '<td><button type="button" class="delete" onclick="deleteRow(this)">Delete</button></td>';

      // datatype select set karo
      var sel = tr.querySelector('select');
      sel.value = String(dt !== undefined && dt !== null ? dt : 0);
      return tr;
    }

    // ── row add ─────────────────────────────────────────────────────────────
    function addRow() {
      document.getElementById('tableBody').appendChild(makeRow());
    }

    // ── row delete ──────────────────────────────────────────────────────────
    // FIX: rowCount variable hata diya — DOM se count hota hai ab
    function deleteRow(btn) {
      var row = btn.closest('tr');
      row.parentNode.removeChild(row);
    }

    // ── load saved data ─────────────────────────────────────────────────────
    function getData() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState !== 4 || this.status !== 200) return;
        var arr = JSON.parse(this.responseText);
        var rc  = parseInt(arr["rowCount"]) || 0;
        var tbody = document.getElementById('tableBody');
        tbody.innerHTML = '';        // pehle saaf karo
        if (rc === 0) { addRow(); return; }
        for (var i = 1; i <= rc; i++) {
          tbody.appendChild(makeRow(
            arr["NI" + i],
            arr["SA" + i],
            arr["RG" + i],
            arr["FC" + i],
            arr["DT" + i]
          ));
        }
      };
      xhr.open("GET", "load", true);
      xhr.send();
    }

    // ── submit ──────────────────────────────────────────────────────────────
    // FIX: ID-based reading hata diya — ab DOM position se read hota hai
    function submitForm() {
      var tbody = document.getElementById('tableBody');
      var rows  = tbody.getElementsByTagName('tr');
      var data  = { rowCount: rows.length };

      for (var i = 0; i < rows.length; i++) {
        var n      = i + 1;
        var inputs = rows[i].getElementsByTagName('input');
        var sel    = rows[i].getElementsByTagName('select')[0];

        // inputs: 0=name, 1=slaveAddress, 2=register, 3=functionCode
        data["name"         + n] = inputs[0] ? inputs[0].value.trim() : "";
        data["slaveAddress" + n] = inputs[1] ? inputs[1].value.trim() : "";
        data["register"     + n] = inputs[2] ? inputs[2].value.trim() : "";
        data["functionCode" + n] = inputs[3] ? inputs[3].value.trim() : "";
        data["datatype"     + n] = sel       ? sel.value              : "0";
      }

      showMsg("Saving " + rows.length + " rows...", "");

      fetch('/submit', {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify(data)
      })
      .then(function(r) {
        if (r.ok) showMsg("Saved successfully (" + rows.length + " rows)", "ok");
        else      r.text().then(function(t){ showMsg("Error: " + t, "err"); });
      })
      .catch(function(e) { showMsg("Network error: " + e, "err"); });
    }

    // ── status message ───────────────────────────────────────────────────────
    function showMsg(text, type) {
      var el = document.getElementById('statusMsg');
      el.textContent   = text;
      el.className     = type === "ok" ? "msg-ok" : type === "err" ? "msg-err" : "";
      el.style.display = "block";
    }
  </script>
</body>
</html>
)=====";
