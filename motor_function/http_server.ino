
#include <WebServer.h>

// experiment with removing forward declaration and see if still works
bool bridge_open();
bool bridge_close();
bool stop();

WebServer server(80);

const char HTML_PAGE[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Bridge Control MVP</title>
  <style>
    body {
      font-family: sans-serif;
      color: white;
      padding: 1.25rem;
      background:whitesmoke;
    }
    .box {
      border:  0.5rem solid grey;
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
    .status.traffic {
      flex: 4;
      margin: 0 0.625rem;
      border: 0.00625rem solid grey;
      padding: 0.625rem;
      text-align: center;
    }
    .status.marine {
      flex: 4;
      margin: 0 0.625rem;
      border: 0.00625rem solid grey;
      padding: 0.625rem;
      text-align: center;
    }
    .status.traffic { background: #4d5563; }
    .status.marine { background: #4d5563; }
    .dot {
        display: inline-block;
        width: 0.75rem;
        height: 0.75rem;
        margin-left: 0.5rem;
        border-radius: 80%;
    }
    .dot.red {
        background: #ff0000;
        box-shadow: 0 0 0.375rem #ff0000;
    }
    .dot.green {
        background: #00ff00;
        box-shadow: 0 0 0.375rem #00ff00;
    }
    .dot.white {
        background: white;
        box-shadow: 0 0 0.375rem grey;
    }












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
    }
    .system-mode{
      border: 0.125rem solid grey;
      background: #4d5563;

    }
    .substatus.detected { background:#4d5563; }
    .substatus.passing { background: #4d5563; }
    .substatus.departed { background:#4d5563; }












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
    .button.green:active{
        background: #a8f5c2; 
    }
    .button.red { background: #e74c3c; font-size: 1.25rem; }
    .button.red:active{
        background: #f8a8a8; 
    }
    .system-mode {
      border: 0.0625rem solid grey;
      padding: 0.4rem;
      text-align: center;
    }
    .mode-options{
      display:flex;
      justify-content: center;
      align-items: center;
      gap:3.5rem
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
            <span class = "dot red" id="trafficStatus"></span>
        </div>
        <div class="status marine">
            Marine Status
            <span class = "dot white" id="marineStatus"></span>
        </div>
      </div>
    </div>

    <!-- Bottom row -->
    <div class="bottom-row">
      <!-- Marine Status -->
      <div class="box marine">
        <div class="title">Marine Status</div>
        <div class="substatus detected" id="shipDetected">
            Ship Detected
            <span class = "dot red" id="shipDetectedLight"></span>
        </div>
        <div class="substatus passing" id="shipPassing">
            Ship Passing
            <span class = "dot red" id="shipPassingLight"></span>
        </div>
        <div class="substatus departed" id="shipDeparted">
            Ship Departed
            <span class = "dot green" id="shipDepartedLight"></span>
        </div>
      </div>

      <!-- Manual Control -->
      <div class="box manual">
        <div class="title">Manual Control</div>
        <button class="button green" id="openBridge">Open Bridge</button>
        <button class="button green" id="closeBridge">Close Bridge</button>
        <button class="button red" id="stopButton"> EMERGENCY STOP</button>
        <div class="system-mode">
            System Mode
            <div class="mode-options">
                <div class="mode-option" id="modeManual">Manual<span class = "dot white" id="manualLight"></span> </div>
                <div class="mode-option" id="modeAuto">Auto <span class = "dot green" id="autoLight"></span></div>
            </div>
        </div>
      </div>
    </div>

  </div>
  















  <script>
  // Bridge States 
  // Making objects immutable 
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
  const shipDetectedLight = document.getElementById("shipDetectedLight"); 
  const shipPassingLight = document.getElementById("shipPassingLight"); 
  

  // Functions 

  //Making UI Aligns with the Mutable State 
  function setBridgeStatus(newState) {
    bridgeStatusElement.textContent = newState;
  }

  // Button handlers 
  btnOpen.addEventListener("click", () => {
    sendCommand("/api/open");
  });

  btnClose.addEventListener("click", () => {
    sendCommand("/api/close");
  });

  btnStop.addEventListener("click", () => {
    sendCommand("/api/stop");
  });

  function sendCommand(endpoint) {
    fetch(endpoint, { method: 'POST' })
      .then(response => response.json())
      .then(data => console.log(data))
      .catch(error => console.error('Error:', error));
  }

  </script>
</body>

</html>
)rawliteral";

void setupWebServer()
{
  // When someone requests the root of a website
  // Then, return HTML
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", HTML_PAGE); });

  // API endpoints
  server.on("/api/open", HTTP_POST, handleOpenBridge);
  server.on("/api/close", HTTP_POST, handleCloseBridge);
  server.on("/api/stop", HTTP_POST, handleEmergencyStop);
  // server.on("/api/status", HTTP_GET, handleGetStatus);

  // Handle 404 - invalid endpoint or URLs
  server.onNotFound([]()
                    { server.send(404, "text/plain", "Not found"); });
  server.begin();
}

void handleWebServerClients()
{
  server.handleClient();
}

void handleOpenBridge()
{
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
  if (stop())
  {
    server.send(200, "application/json", "{\"status\":\"stopping\"}");
  }
  else
  {
    server.send(500, "application/json", "{\"status\":\"error\"}");
  }
}
