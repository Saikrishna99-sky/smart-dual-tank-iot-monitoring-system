const char HTML5[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wittelb 4G Gateway - IO Control</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: "Verdana", "Arial", sans-serif;
            background-color: #efefef;
        }
        .navbar {
            width: 100%; height: 50px;
            padding: 10px 0px;
            background-color: #FF7F00;
            color: #000000;
            position: fixed; top: 0; left: 0; right: 0;
            z-index: 1030;
        }
        .navtitle {
            float: left; height: 30px;
            font-size: 30px; font-weight: bold;
            line-height: 50px; padding-left: 20px;
        }
        ul {
            list-style-type: none;
            margin-top: 40px;
            overflow: hidden;
            background-color: #333;
        }
        li { float: left; border-right: 1px solid #bbb; }
        li:last-child { border-right: none; }
        li a {
            display: block; color: white;
            text-align: center; padding: 14px 16px;
            text-decoration: none;
        }
        li a:hover:not(.active) { background-color: #111; }
        .active { background-color: #04AA6D; }
        .container {
            max-width: 1200px;
            margin: 100px auto 20px auto;
            background-color: #fff;
            padding: 20px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .category {
            font-weight: bold; font-size: 22px;
            padding: 15px 10px 8px 10px;
            color: #000;
            border-left: 5px solid #FF7F00;
            margin: 20px 0 10px 0;
        }
        table {
            border-collapse: collapse;
            width: 100%; margin: 10px 0 20px 0;
            text-align: center;
        }
        table, th, td { border: 2px solid #ddd; }
        th, td { padding: 10px; }
        th { background-color: #f2f2f2; font-weight: bold; }

        /* Toggle Switch */
        .toggle-switch {
            position: relative; display: inline-block;
            width: 60px; height: 30px;
        }
        .toggle-switch input { opacity: 0; width: 0; height: 0; }
        .slider {
            position: absolute; cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #ccc;
            border-radius: 30px;
            transition: 0.3s;
        }
        .slider:before {
            position: absolute; content: "";
            height: 22px; width: 22px;
            left: 4px; bottom: 4px;
            background-color: white;
            border-radius: 50%;
            transition: 0.3s;
        }
        input:checked + .slider { background-color: #4CAF50; }
        input:checked + .slider:before { transform: translateX(30px); }

        /* DO Status label */
        .do-status-on  { color: #4CAF50; font-weight: bold; }
        .do-status-off { color: #f44336; font-weight: bold; }

        /* DI Value */
        .value-cell { font-weight: bold; font-size: 16px; }
        .value-0 { color: #f44336; }
        .value-1 { color: #4CAF50; }

        /* AI Progress bar */
        .ai-bar-wrap {
            width: 100%; background: #eee;
            border-radius: 6px; height: 18px;
            overflow: hidden; min-width: 80px;
        }
        .ai-bar {
            height: 18px; background: #FF7F00;
            border-radius: 6px;
            transition: width 0.3s;
        }
        .ai-val { font-weight: bold; font-size: 15px; }
        .ai-source { font-size: 11px; color: #888; }

        .btn {
            background-color: #4CAF50; border: none;
            color: white; padding: 10px 24px;
            font-size: 13px; cursor: pointer;
            border-radius: 8px; margin-top: 10px;
        }
        .btn:hover { background-color: #388e3c; }
        .center-button { text-align: center; margin: 15px 0; }
        .last-update {
            font-size: 12px; color: #888;
            text-align: right; padding: 4px 8px;
        }
    </style>
</head>
<body>
<header>
    <div class="navbar">
        <div class="navtitle">Wittelb 4G Gateway Configuration</div>
        <ul>
            <li><a href="cpp1">Network Configuration</a></li>
            <li><a href="cpp2">Serial Port</a></li>
            <li><a href="cpp3">Cloud Platform</a></li>
            <li><a href="cpp4">Modbus Master</a></li>
            <li><a href="cpp5">I/O Config</a></li>
            <li><a class="active" href="cpp6">IO Control</a></li>
            <li style="float:right"><a href="www.wittelb.com">About</a></li>
        </ul>
    </div>
</header>

<div class="container">

    <!-- ── DIGITAL OUTPUT ── -->
    <div class="category">Digital Output Control (4 DO)</div>
    <table>
        <thead>
            <tr>
                <th>Sr.No</th>
                <th>Output</th>
                <th>Name</th>
                <th>Control</th>
                <th>Status</th>
            </tr>
        </thead>
        <tbody id="doTable">
            <!-- filled by JS -->
        </tbody>
    </table>

    <!-- ── DIGITAL INPUT ── -->
    <div class="category">Digital Input Status (4 DI)</div>
    <table>
        <thead>
            <tr>
                <th>Sr.No</th>
                <th>Input</th>
                <th>Name</th>
                <th>Value</th>
                <th>Status</th>
            </tr>
        </thead>
        <tbody id="diTable">
            <!-- filled by JS -->
        </tbody>
    </table>

    <!-- ── ANALOG INPUT ── -->
    <div class="category">Analog Input Monitor (8 AI)</div>
    <table>
        <thead>
            <tr>
                <th>Sr.No</th>
                <th>Input</th>
                <th>Raw Value</th>
                <th>Level</th>
            </tr>
        </thead>
        <tbody id="aiTable">
            <!-- filled by JS -->
        </tbody>
    </table>

    <div class="last-update">Last update: <span id="lastUpdate">-</span></div>
</div>

<script>
// ── Config ──────────────────────────────────────────
const DO_COUNT = 4;
const DI_COUNT = 4;
const AI_COUNT = 8;
const AI_MAX   = 1023;   // 10-bit ADC max (Arduino analog / MCP3208)

// DO state tracker
let doState = [0, 0, 0, 0];

// ── Build DO table ───────────────────────────────────
function buildDOTable() {
    const tb = document.getElementById("doTable");
    tb.innerHTML = "";
    for (let i = 0; i < DO_COUNT; i++) {
        const tr = document.createElement("tr");
        tr.innerHTML =
            '<td>' + (i+1) + '</td>' +
            '<td>DO' + (i+1) + '</td>' +
            '<td id="doName' + i + '">-</td>' +
            '<td>' +
                '<label class="toggle-switch">' +
                    '<input type="checkbox" id="doToggle' + i + '" onchange="sendDO(' + i + ')">' +
                    '<span class="slider"></span>' +
                '</label>' +
            '</td>' +
            '<td id="doStatus' + i + '" class="do-status-off">OFF</td>';
        tb.appendChild(tr);
    }
}

// ── Build DI table ───────────────────────────────────
function buildDITable() {
    const tb = document.getElementById("diTable");
    tb.innerHTML = "";
    for (let i = 0; i < DI_COUNT; i++) {
        const tr = document.createElement("tr");
        tr.innerHTML =
            '<td>' + (i+1) + '</td>' +
            '<td>DI' + (i+1) + '</td>' +
            '<td id="diName' + i + '">-</td>' +
            '<td class="value-cell" id="diValue' + i + '">-</td>' +
            '<td id="diStatus' + i + '">-</td>';
        tb.appendChild(tr);
    }
}

// ── Build AI table ───────────────────────────────────
function buildAITable() {
    const tb = document.getElementById("aiTable");
    tb.innerHTML = "";
    for (let i = 0; i < AI_COUNT; i++) {
        const tr = document.createElement("tr");
        tr.innerHTML =
            '<td>' + (i+1) + '</td>' +
            '<td>AI' + (i+1) + '</td>' +
            '<td class="ai-val" id="aiValue' + i + '">-</td>' +
            '<td>' +
                '<div class="ai-bar-wrap">' +
                    '<div class="ai-bar" id="aiBar' + i + '" style="width:0%"></div>' +
                '</div>' +
            '</td>';
        tb.appendChild(tr);
    }
}

// ── Send DO command to ESP32 ─────────────────────────
function sendDO(idx) {
    const cb  = document.getElementById("doToggle" + idx);
    const val = cb.checked ? 1 : 0;
    doState[idx] = val;

    // Send "ai0:1" / "ai0:0" format (matches base board check_input)
    const cmd = "ai" + idx + ":" + val;
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/docontrol", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.send("cmd=" + cmd);
    console.log("Sent: " + cmd);
}

// ── Fetch /values and update UI ──────────────────────
function updateValues() {
    const xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            const d = JSON.parse(this.responseText);

            // DO
            for (let i = 0; i < DO_COUNT; i++) {
                const v = d["doValue" + i];
                if (v !== undefined) {
                    const cb = document.getElementById("doToggle" + i);
                    const st = document.getElementById("doStatus" + i);
                    if (cb) cb.checked = (v == 1);
                    if (st) {
                        st.innerText = v == 1 ? "ON" : "OFF";
                        st.className = v == 1 ? "do-status-on" : "do-status-off";
                    }
                }
            }

            // DI
            for (let i = 0; i < DI_COUNT; i++) {
                const v = d["diValue" + i];
                const ve = document.getElementById("diValue" + i);
                const se = document.getElementById("diStatus" + i);
                if (v !== undefined && ve) {
                    ve.innerText = v;
                    ve.className = "value-cell " + (v == 1 ? "value-1" : "value-0");
                    if (se) {
                        se.innerText = v == 1 ? "HIGH" : "LOW";
                        se.style.color = v == 1 ? "#4CAF50" : "#f44336";
                    }
                }
            }

            // AI
            for (let i = 0; i < AI_COUNT; i++) {
                const v = d["aiValue" + i];
                const ve = document.getElementById("aiValue" + i);
                const be = document.getElementById("aiBar" + i);
                if (v !== undefined && ve) {
                    ve.innerText = v;
                    const pct = Math.min(100, Math.round((v / AI_MAX) * 100));
                    if (be) be.style.width = pct + "%";
                }
            }

            document.getElementById("lastUpdate").innerText = new Date().toLocaleTimeString();
        }
    };
    xhr.open("GET", "values", true);
    xhr.send();
}

// ── Load names from /confi ───────────────────────────
function loadNames() {
    const xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            const d = JSON.parse(this.responseText);
            for (let i = 0; i < DO_COUNT; i++) {
                const el = document.getElementById("doName" + i);
                if (el && d["doName" + i]) el.innerText = d["doName" + i];
            }
            for (let i = 0; i < DI_COUNT; i++) {
                const el = document.getElementById("diName" + i);
                if (el && d["diName" + i]) el.innerText = d["diName" + i];
            }
        }
    };
    xhr.open("GET", "confi", true);
    xhr.send();
}

// ── Init ─────────────────────────────────────────────
buildDOTable();
buildDITable();
buildAITable();
loadNames();
updateValues();
setInterval(updateValues, 500);
</script>
</body>
</html>
)=====";
