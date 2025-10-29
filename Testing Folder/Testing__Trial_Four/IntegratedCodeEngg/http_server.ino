#include <WebServer.h>
#include <Arduino.h>
#include "state.h" // brings in enums + extern g_mode/g_cmd

// start manual if you like
void setupWebServer();
void handleWebServerClients();
void handleOpenBridge();
void handleCloseBridge();
void handleEmergencyStop();

// experiment with removing forward declaration and see if still works
extern bool bridge_open();
extern bool bridge_close();
extern bool stop();
extern SystemMode g_mode;
extern BridgeCmd g_cmd;

WebServer server(80);

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Bridge Control MVP</title>
  <style>
    body {
      font-family: sans-serif;
      color: white;
      padding: 1.25rem;
      background: whitesmoke;
    }
    .box {
      border: 0.5rem solid grey;
      padding: 1rem;
      margin-bottom: 1.25rem;
      background: #2d3748;
    }
    .title {
      font-weight: bold;
      margin-bottom: 0.75rem;
      text-align: center;
    }
    .bridge-status {
      height: 16rem;
    }
    .bridge-status-inner {
      border: 0.125rem solid grey;
      background: #4d5563;
      padding: 3.5rem;
      text-align: center;
      font-weight: bold;
      font-size: 1.75rem;
      margin: auto;
      margin-bottom: 1.25rem;
      width: 16rem;
    }
    .status-row {
      display: flex;
      justify-content: space-between;
    }
    .status.traffic, .status.marine {
      flex: 4;
      margin: 0 0.625rem;
      border: 0.00625rem solid grey;
      padding: 0.625rem;
      text-align: center;
      background: #4d5563;
    }
    .dot {
      display: inline-block;
      width: 0.75rem;
      height: 0.75rem;
      margin-left: 0.5rem;
      border-radius: 80%;
    }
    .dot.red    { background: #ff0000; box-shadow: 0 0 0.375rem #ff0000; }
    .dot.green  { background: #00ff00; box-shadow: 0 0 0.375rem #00ff00; }
    .dot.white  { background: white;   box-shadow: 0 0 0.375rem grey; }

    .bottom-row {
      display: flex;
      gap: 20px;
    }
    .marine, .manual {
      flex: 1;
    }
    .substatus {
      border: 0.0625rem solid grey;
      padding: 1rem;
      margin-bottom: 1rem;
      text-align: center;
      background: #4d5563;
    }
    .system-mode {
      border: 0.125rem solid grey;
      background: #4d5563;
      padding: 0.4rem;
      text-align: center;
    }
    .mode-row {
      display: flex;
      gap: 14px;
      justify-content: center;
      margin-top: .5rem;
    }
    .button {
      display: block;
      width: 100%;
      padding: 0.9rem;
      margin-bottom: 0.625rem;
      border: none;
      border-radius: 2rem;
      font-size: 1rem;
      font-weight: bold;
      cursor: pointer;
    }
    .button.green { background: #2ecc71; }
    .button.green:active { background: #a8f5c2; }
    .button.red { background: #e74c3c; font-size: 1.25rem; }
    .button.red:active { background: #f8a8a8; }
    .mode-btn {
      width: 6.25rem;
      border: none;
      border-radius: 2rem;
      padding: 0.8rem 0;
      font-size: 1rem;
      font-weight: 700;
      cursor: pointer;
      background: #f7f7f7;
      color: #0b1a2b;
      transition: transform 0.1s ease, box-shadow 0.2s ease, background 0.2s ease;
    }
    .mode-btn:hover {
      box-shadow: 0 0 12px rgba(200, 200, 200, 0.8);
      background: #ffffff;
    }
    .mode-btn:active {
      transform: scale(0.9);
      box-shadow: 0 0 16px rgba(0, 0, 0, 0.3);
      background: #e5e5e5;
    }
  </style>
</head>

<body>
  <div class="container">

    <!-- Bridge Status Panel -->
    <div class="box bridge-status">
      <div class="title">Bridge Control System</div>
      <div class="bridge-status-inner" id="bridgeStatus">Bridge Up</div>
      <div class="status-row">
        <div class="status traffic">
          Traffic Status
          <span class="dot red" id="trafficStatus"></span>
        </div>
        <div class="status marine">
          Marine Status
          <span class="dot white" id="marineStatus"></span>
        </div>
      </div>
    </div>

    <!-- Bottom row -->
    <div class="bottom-row">
      <!-- Marine Status -->
      <div class="box marine">
        <div class="title">Marine Status</div>
        <div class="substatus" id="shipDetected">
          Ship Detected
          <span class="dot red" id="shipDetectedLight"></span>
        </div>
        <div class="substatus" id="shipPassing">
          Ship Passing
          <span class="dot red" id="shipPassingLight"></span>
        </div>
        <div class="substatus" id="shipDeparted">
          Ship Departed
          <span class="dot green" id="shipDepartedLight"></span>
        </div>
      </div>

      <!-- Manual Control -->
      <div class="box manual">
        <div class="title">Manual Control</div>
        <button class="button green" id="openBridge">Open Bridge</button>
        <button class="button green" id="closeBridge">Close Bridge</button>
        <button class="button red" id="stopButton">EMERGENCY STOP</button>
        <div class="system-mode">
          System Mode
          <div class="mode-row">
            <button id="btnManualMode" class="mode-btn">Manual</button>
            <button id="btnAutoMode" class="mode-btn">Auto</button>
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
  // Bridge States
  const BridgeState = Object.freeze({
    UP: "Bridge Up",
    DOWN: "Bridge Down",
    MOVING: "Bridge Moving",
    HALT: "Bridge Halt"
  });

  // Elements
  const bridgeStatusElement = document.getElementById("bridgeStatus");
  const btnOpen  = document.getElementById("openBridge");
  const btnClose = document.getElementById("closeBridge");
  const btnStop  = document.getElementById("stopButton");
  const btnManualMode = document.getElementById("btnManualMode");
  const btnAutoMode   = document.getElementById("btnAutoMode");
  const trafficStatusDot = document.getElementById("trafficStatus");

  // Functions
  function setBridgeStatus(newState) {
    bridgeStatusElement.textContent = newState;

    //Update traffic light based on bridge state 
    if(newState === BridgeState.UP){
      //Bridge up, no traffic, red dot
      trafficStatusDot.className = "dot red"; 
    } else if (newState === BridgeState.DOWN){
      //Bridge down, traffic, green dot
      trafficStatusDot.className = "dot green"; 
    } else if (newState === BridgeState.HALT){
      //Bridge halt, no traffic, red dot
      trafficStatusDot.className = "dot red"; 
    } else if (newState === BridgeState.MOVING){
      //Bridge moving, no traffic, red dot
      trafficStatusDot.className = "dot red"; 
    }
  }

  function setModeUI(mode) {
    const isManual = (mode === 'manual');
    btnManualMode.classList.toggle('active', isManual);
    btnAutoMode.classList.toggle('active', !isManual);

    // Disable Open/Close in AUTO
    btnOpen.disabled  = !isManual;
    btnClose.disabled = !isManual;
  }

  function sendCommand(endpoint) {
    return fetch(endpoint, { method: 'POST' })
      .then(response => response.json())
      .then(data => console.log(data))
      .catch(error => console.error('Error:', error));
  }



  // Button Handlers
  btnOpen.addEventListener("click", () => {
    sendCommand("/api/open").then(()=> {
      setBridgeStatus(BridgeState.MOVING);
    }); 
  }); 

  btnOpen.addEventListener("click", () => {
    sendCommand("/api/close").then(()=> {
      setBridgeStatus(BridgeState.MOVING);
    }); 
  }); 

  btnOpen.addEventListener("click", () => {
    sendCommand("/api/stop").then(()=> {
      setBridgeStatus(BridgeState.HALT);
    }); 
  }); 



  // Mode Handlers
  btnManualMode.addEventListener("click", () => {
    sendCommand('/api/mode/manual').then(() => setModeUI('manual'));
  });
  btnAutoMode.addEventListener("click", () => {
    sendCommand('/api/mode/auto').then(() => setModeUI('auto'));
  });

  // Default to Manual
  setBridgeStatus(BridgeState.DOWN);
  setModeUI('manual');
  </script>
</body>
</html>
)rawliteral";

// ---------------- WebServer setup ----------------

void setupWebServer()
{
  Serial.println("http_server::setupWebServer() start");

  server.on("/", HTTP_GET, []()
            { server.send_P(200, "text/html", HTML_PAGE); });

  // Manual control endpoints
  server.on("/api/open", HTTP_POST, []()
            {
    Serial.println("[HTTP] /api/open");
    g_mode = MODE_MANUAL;
    g_cmd_manual  = CMD_OPEN;
    server.send(200, "application/json", "{\"status\":\"opening\"}"); });

  server.on("/api/close", HTTP_POST, []()
            {
    Serial.println("[HTTP] /api/close");
    g_mode = MODE_MANUAL;
    g_cmd_manual = CMD_CLOSE;
    server.send(200, "application/json", "{\"status\":\"closing\"}"); });

  server.on("/api/stop", HTTP_POST, []()
            {
    g_emergency  = true;         // <-- latch
    g_cmd_manual  = CMD_STOP;
    g_cmd_auto   = CMD_STOP;

    
    server.send(200, "application/json", "{\"status\":\"stopping\"}"); });

  // Mode switching
  server.on("/api/mode/auto", HTTP_POST, []()
            {
    g_emergency  = false;     // auto-clear emergency
    g_mode = MODE_AUTO;
    g_cmd_manual = CMD_IDLE;
    g_cmd_auto   = CMD_IDLE;

    server.send(200, "application/json", "{\"mode\":\"auto\"}"); });

  server.on("/api/mode/manual", HTTP_POST, []()
            {
    g_emergency  = false;     // auto-clear emergency
    g_mode = MODE_MANUAL;
    g_cmd_manual = CMD_IDLE; // start safe
    g_cmd_auto   = CMD_IDLE; // neuter auto
    server.send(200, "application/json", "{\"mode\":\"manual\"}"); });

  server.onNotFound([]()
                    { server.send(404, "text/plain", "Not found"); });

  server.begin();
  Serial.println("http_server::setupWebServer() end");
}

void handleWebServerClients()
{
  server.handleClient();
}

void handleOpenBridge()
{
  Serial.println("http_server::handleOpenBridge()");
  if (bridge_open())
  {
    server.send(200, "application/json", "{\"status\":\"opening\"}");
  }
  else
  {
    server.send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleCloseBridge()
{
  Serial.println("http_server::handleCloseBridge()");
  if (bridge_close())
  {
    server.send(200, "application/json", "{\"status\":\"closing\"}");
  }
  else
  {
    server.send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleEmergencyStop()
{
  Serial.println("http_server::handleStop()");
  if (stop())
  {
    server.send(200, "application/json", "{\"status\":\"stopping\"}");
  }
  else
  {
    server.send(500, "application/json", "{\"status\":\"error\"}");
  }
}
