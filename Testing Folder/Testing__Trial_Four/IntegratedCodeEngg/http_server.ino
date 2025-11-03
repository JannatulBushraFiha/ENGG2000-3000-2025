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
extern volatile bool         g_emergency;
extern volatile BridgeCmd    g_cmd_manual;
extern volatile BridgeCmd    g_cmd_auto;
extern bool bridge_open();
extern bool bridge_close();
extern bool stop();
extern SystemMode g_mode;
extern BridgeCmd g_cmd;
extern float g_distance_cm; 
extern float distanceCm1, distanceCm2;
extern const char* marineStatusToString(MarineStatus s);



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
    .ship, .manual {
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
    @keyframes blink-animation{
      100% {
        opacity: 1;
      }
      50% {
        opacity: 0;
      }
      100% {
        opacity: 1;
      }
    }
    .flashing-element {
      animation: blink-animation 0.5s infinite; 
    }
    @keyframes blink-text{
      100% {
        opacity: 1;
      }
      50% {
        opacity: 0;
      }
      100% {
        opacity: 1;
      }
    }
    .flashing-element-text {
      animation: blink-text 1.35s infinite; 
    }
    @keyframes blink-animation-ship{
      100% {
        opacity: 1;
      }
      50% {
        opacity: 0;
      }
      100% {
        opacity: 1;
      }
    }
    .flashing-element-shipdetected{
      animation: blink-animation-ship 0.5s infinite;
    }
    .flashing-element-shippassing{
      animation: blink-animation-ship 0.5s infinite;
    }
    .flashing-element-shipdeparted{
      animation: blink-animation-ship 0.5s infinite;
    }
  </style>
</head>

<body>
  <div class="container">

    <!-- Bridge Status Panel -->
    <div class="box bridge-status">
      <div class="title">Bridge Control System</div>
      <div class="bridge-status-inner"><span id="bridgeStatus">Bridge Up</span></div>
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
      <div class="box ship">
        <div class="title">Ship Status</div>
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
  const MarineState = Object.freeze({
    DETECTED: "Ship Detected",
    PASSING: "Ship Passing", 
    DEPARTED: "Ship Departed"
  }); 

  //Hard-coded moving time for 4 seconds
  //Change when the timer variable is made and updated
  const MOVING_DURATION_MS = 30000; 

  // Elements
  const bridgeStatusElement = document.getElementById("bridgeStatus");
  const btnOpen  = document.getElementById("openBridge");
  const btnClose = document.getElementById("closeBridge");
  const btnStop  = document.getElementById("stopButton");
  const btnManualMode = document.getElementById("btnManualMode");
  const btnAutoMode   = document.getElementById("btnAutoMode");
  const trafficStatusDot = document.getElementById("trafficStatus");
  const marineStatusDot = document.getElementById("marineStatus");
  const shipDetectedDot = document.getElementById("shipDetectedLight");
  const shipPassingDot = document.getElementById("shipPassingLight");
  const shipDepartedDot = document.getElementById("shipDepartedLight");

  // Functions
  function setBridgeStatus(newState) {
    bridgeStatusElement.textContent = newState;
    //Don't flash until bridge starts moving
    trafficStatusDot.classList.remove("flashing-element");
    bridgeStatusElement.classList.remove("flashing-element-text");

    //Update traffic light based on bridge state 
    if(newState === BridgeState.UP){
      //Bridge up, no traffic, traffic red dot, marine safe to go
      trafficStatusDot.className = "dot red"; 
      marineStatusDot.className = "dot green";
    } else if (newState === BridgeState.DOWN){
      //Bridge down, traffic, traffic green dot, marine not safe to go
      trafficStatusDot.className = "dot green"; 
      marineStatusDot.className = "dot red";
    } else if (newState === BridgeState.HALT){
      //Bridge halt, no traffic & marine, traffic red dot, marine red dot
      trafficStatusDot.className = "dot red"; 
      marineStatusDot.className = "dot red";
    } else if (newState === BridgeState.MOVING){
      //Bridge moving, no traffic & marine, traffic & marine red dot
      trafficStatusDot.className = "dot red"; 
      marineStatusDot.className = "dot red";
      //Start flashing while moving
      trafficStatusDot.classList.add("flashing-element");
      bridgeStatusElement.classList.add("flashing-element-text");
    }
  }

  function setShipStatus(newState){
    shipDetectedDot.className = "dot white"; 
    shipPassingDot.className = "dot white";
    shipDepartedDot.className = "dot white"; 
    shipDetectedDot.classList.remove("flashing-element-shipdetected");
    shipPassingDot.classList.remove("flashing-element-shippassing");
    shipDepartedDot.classList.remove("flashing-element-shipdeparted");

    //handle CLEAR
    if (!newState || newState === 'CLEAR'){
      return; 
    }

    if(newState === MarineState.DETECTED){
      shipDetectedDot.className = "dot red";
      shipDetectedDot.classList.add("flashing-element-shipdetected");
    } else if (newState === MarineState.PASSING){
      shipPassingDot.className = "dot red"; 
      shipPassingDot.classList.add("flashing-element-shippassing");
    } else if (newState === MarineState.DEPARTED){
      shipDepartedDot.className = "dot green"; 
      shipDepartedDot.classList.add("flashing-element-shipdeparted");
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
      //hard-coded moving time after "open bridge", change later
      setTimeout(() => {
        setBridgeStatus(BridgeState.UP);
      }, MOVING_DURATION_MS);
    }); 
  }); 

  btnClose.addEventListener("click", () => {
    sendCommand("/api/close").then(()=> {
      setBridgeStatus(BridgeState.MOVING);
      //hard-coded moving time after "close bridge", change later 
      setTimeout(() => {
        setBridgeStatus(BridgeState.DOWN);
      }, MOVING_DURATION_MS);
    }); 
  }); 

  btnStop.addEventListener("click", () => {
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

  //MAP API for Ship DETECTED/PASSING/DEPARTED/CLEAR 
  function mapApiStatusToUI(s){
    switch (s) {
      case 'DETECTED': return MarineState.DETECTED;
      case 'PASSING':  return MarineState.PASSING;
      case 'DEPARTED': return MarineState.DEPARTED;
      case 'CLEAR':    return 'CLEAR';
      default:         return null;
    }
  }

  async function refreshMarine(){
    try {
      const r = await fetch('/api/marine', { cache: 'no-store' });
      if (!r.ok) throw new Error('HTTP ' + r.status);
      const d = await r.json();

      // Update mode buttons if you like (AUTO disables Open/Close)
      if (d.mode === 'AUTO') setModeUI('auto'); else if (d.mode === 'MANUAL') setModeUI('manual');

      // Update the Ship Status lights
      const uiState = mapApiStatusToUI(d.marine_status);
      setShipStatus(uiState);

      // Optional: color the small marine dot at top row
      // Red when DETECTED/PASSING; green on CLEAR/DEPARTED
      if (uiState === MarineState.DETECTED || uiState === MarineState.PASSING) {
        marineStatusDot.className = "dot red";
      } else {
        marineStatusDot.className = "dot green";
      }
    } catch (e) {
      // On error, leave existing UI as-is or show a neutral state
      console.warn('refreshMarine failed:', e);
    }
  }

  // Poll every 250 ms
  setInterval(refreshMarine, 250);
  refreshMarine();

  // Default to Manual
  setBridgeStatus(BridgeState.DOWN);
  setModeUI('manual');
  </script>
</body>
</html>
)rawliteral";

// ---------------- WebServer setup ----------------

// Enum -> readable text for JSON


// Format float or "null" (keeps JSON clean when readings are invalid)
static String f_or_null(float v, unsigned int dp = 1) {
  if (isnan(v) || v <= 0) return F("null");
  String s; s.reserve(12);
  s += String(v, dp);
  return s;
}

static void handleApiMarine() {
  String json;
  json.reserve(256);
  json += F("{\"d1\":");  json += f_or_null(distanceCm1);
  json += F(",\"d2\":");  json += f_or_null(distanceCm2);
  json += F(",\"nearest_cm\":"); json += f_or_null(g_distance_cm);
  json += F(",\"marine_status\":\""); json += marineStatusToString(g_marine_status); json += '\"';
  json += F(",\"mode\":\""); json += (g_mode == MODE_AUTO ? "MANUAL" : "MANUAL"); // placeholder
  json.remove(json.length()-7); // remove the wrong "MANUAL"
  json += (g_mode == MODE_AUTO ? "AUTO" : "MANUAL"); // correct mode insert
  json += F("\"}");

  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.send(200, "application/json", json);
}



void setupWebServer()
{
  Serial.println("http_server::setupWebServer() start");

  //Register marine endpoints
  server.on("/api/marine", HTTP_GET, handleApiMarine);

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

  server.on("/api/status", HTTP_GET, []() {
    String status = "unknown";
    String trafficLight = "red"; 


    //Recommended to add a timer as a tracking condition
    //check if emergency stop is active. 
    if (g_emergency){
      status = "halt"; 
      trafficLight = "red"; 
    }
    //check if the bridge is currently executing a command, opening or closing in both auto and manual
    //opening 
    else if (g_cmd_manual == CMD_OPEN || g_cmd_auto == CMD_OPEN){
      status = "moving"; 
      trafficLight = "red"; 
    } 
    //closing
    else if (g_cmd_manual == CMD_CLOSE || g_cmd_auto == CMD_CLOSE){
      status = "moving";
      trafficLight = "red";
    }
    else{
      if (g_distance_cm > 0 && g_distance_cm < 22.0f){
        status = "up";
        trafficLight = "red"; 
      }
      else {
        status = "down"; 
        trafficLight = "green";
      }
    }

    String json = "{\"bridge\":\"" + status + "\",\"traffic\":\"" + trafficLight + "\"}";
    server.send(200, "application/json", json);
  });


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