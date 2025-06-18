#include <WiFi.h>
#include <WebServer.h>
#include <CytronMotorDriver.h>

// WiFi credentials
const char* ssid = "JLC Net";     // Replace with your WiFi name
const char* password = "321456987";  // Replace with your WiFi password

// Create web server on port 80
WebServer server(80);

// ===== Motor Definitions =====
CytronMD motorL(PWM_DIR, 12, 10);
CytronMD motorR(PWM_DIR, 6, 4);

// ===== Drive Speed =====
const int DRIVE_SPEED = 150;
int currentSpeed = DRIVE_SPEED;

// ===== Serial Bluetooth via HC-05 =====
// HC-05 TX -> ESP32 RX2 (usually GPIO16)
// HC-05 RX -> ESP32 TX2 (usually GPIO17)
#define HC05_RX 16
#define HC05_TX 17
HardwareSerial SerialBT(2);  // UART2

// Motor control functions
void driveMotors(int l, int r) { motorL.setSpeed(l); motorR.setSpeed(r); }
void cmdForward()   { driveMotors( currentSpeed,  currentSpeed); }
void cmdReverse()   { driveMotors(-currentSpeed, -currentSpeed); }
void cmdTurnLeft()  { driveMotors(-currentSpeed,  currentSpeed); }
void cmdTurnRight() { driveMotors( currentSpeed, -currentSpeed); }
void cmdStop()      { driveMotors(0, 0); }

// Robot status
String robotStatus = "idle";
int batteryLevel = 100; // Mock battery level

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize Bluetooth serial
  SerialBT.begin(9600, SERIAL_8N1, HC05_RX, HC05_TX); // HC-05 default baud rate is 9600
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Define server endpoints
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);
  server.on("/speed", handleSpeed);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}



// Enable CORS
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Web server handler functions
void handleForward() {
  enableCORS();
  cmdForward();
  robotStatus = "moving forward";
  server.send(200, "text/plain", "Moving Forward");
}

void handleBackward() {
  enableCORS();
  cmdReverse();
  robotStatus = "moving backward";
  server.send(200, "text/plain", "Moving Backward");
}

void handleLeft() {
  enableCORS();
  cmdTurnLeft();
  robotStatus = "turning left";
  server.send(200, "text/plain", "Turning Left");
}

void handleRight() {
  enableCORS();
  cmdTurnRight();
  robotStatus = "turning right";
  server.send(200, "text/plain", "Turning Right");
}

void handleStop() {
  enableCORS();
  cmdStop();
  robotStatus = "idle";
  server.send(200, "text/plain", "Stopped");
}

// Speed control endpoint
void handleSpeed() {
  enableCORS();
  if (server.hasArg("value")) {
    int speed = server.arg("value").toInt();
    if (speed >= 0 && speed <= 255) {
      currentSpeed = speed;
      server.send(200, "text/plain", "Speed updated to " + String(currentSpeed));
    } else {
      server.send(400, "text/plain", "Speed must be between 0 and 255");
    }
  } else {
    server.send(400, "text/plain", "Missing value parameter");
  }
}

// Status endpoint for getting robot information
void handleStatus() {
  enableCORS();
  String status = "{\"status\":\"" + robotStatus + "\",\"battery\":" + String(batteryLevel) + "}";
  server.send(200, "application/json", status);
}

// Root page - serves the enhanced control interface
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32 Robot Control</title>
  <style>
    :root {
      --primary: #4ade80;
      --primary-dark: #22c55e;
      --secondary: #3b82f6;
      --secondary-dark: #2563eb;
      --danger: #ef4444;
      --warning: #f59e0b;
      --dark: #1f2937;
      --light: #f9fafb;
      --gray: #9ca3af;
      --radius: 12px;
    }
    
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f3f4f6;
      color: var(--dark);
      line-height: 1.6;
      padding: 0;
      margin: 0;
      min-height: 100vh;
    }
    
    .container {
      max-width: 500px;
      margin: 0 auto;
      padding: 20px;
    }
    
    .header {
      text-align: center;
      margin-bottom: 1.5rem;
      padding: 1rem;
      background: linear-gradient(135deg, var(--primary), var(--primary-dark));
      color: white;
      border-radius: var(--radius);
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
    }
    
    .header h1 {
      font-size: 1.8rem;
      font-weight: 600;
      margin: 0;
    }
    
    .card {
      background-color: white;
      border-radius: var(--radius);
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
      padding: 1.5rem;
      margin-bottom: 1.5rem;
      overflow: hidden;
    }
    
    .control-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
    }
    
    .center-cell {
      grid-column: 2;
    }
    
    .button {
      padding: 15px 10px;
      font-size: 1.1rem;
      font-weight: 600;
      text-align: center;
      border-radius: var(--radius);
      background-color: var(--primary);
      color: white;
      border: none;
      cursor: pointer;
      user-select: none;
      touch-action: manipulation;
      transition: all 0.2s ease;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      display: flex;
      align-items: center;
      justify-content: center;
    }
    
    .button:active {
      transform: translateY(2px);
      box-shadow: 0 2px 3px rgba(0, 0, 0, 0.1);
    }
    
    .button.stop {
      background-color: var(--danger);
    }
    
    .button svg {
      width: 24px;
      height: 24px;
      margin-right: 5px;
    }
    
    .speed-control {
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 1.5rem 0;
    }
    
    .speed-button {
      width: 40px;
      height: 40px;
      font-size: 1.5rem;
      background-color: var(--secondary);
      color: white;
      border: none;
      border-radius: 50%;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
      transition: all 0.2s ease;
    }
    
    .speed-button:active {
      transform: scale(0.95);
      box-shadow: 0 1px 2px rgba(0, 0, 0, 0.2);
    }
    
    .speed-value {
      font-size: 1.2rem;
      font-weight: bold;
      width: 60px;
      text-align: center;
      margin: 0 15px;
      background-color: white;
      padding: 5px;
      border-radius: var(--radius);
      box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.1);
    }
    
    .status-panel {
      display: flex;
      flex-direction: column;
      gap: 1rem;
    }
    
    .status-item {
      display: flex;
      flex-direction: column;
    }
    
    .status-label {
      font-weight: 600;
      margin-bottom: 5px;
      display: flex;
      align-items: center;
    }
    
    .status-label svg {
      width: 18px;
      height: 18px;
      margin-right: 5px;
    }
    
    .battery-bar {
      height: 12px;
      background-color: #e0e0e0;
      border-radius: 6px;
      overflow: hidden;
    }
    
    .battery-level {
      height: 100%;
      border-radius: 6px;
      transition: width 0.5s ease, background-color 0.5s ease;
    }
    
    .keyboard-guide {
      display: grid;
      grid-template-columns: auto 1fr;
      gap: 0.5rem 1rem;
      align-items: center;
    }
    
    .keyboard-guide-title {
      grid-column: span 2;
      margin-bottom: 0.5rem;
      font-weight: 600;
      display: flex;
      align-items: center;
    }
    
    .keyboard-guide-title svg {
      width: 18px;
      height: 18px;
      margin-right: 5px;
    }
    
    .key {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      min-width: 30px;
      height: 30px;
      padding: 0 8px;
      background-color: white;
      border-radius: 5px;
      font-family: monospace;
      font-weight: bold;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1), 0 0 0 1px rgba(0, 0, 0, 0.05);
    }
    
    .status-badge {
      display: inline-block;
      padding: 3px 8px;
      border-radius: 12px;
      font-size: 0.85rem;
      font-weight: 600;
      text-transform: capitalize;
      background-color: var(--gray);
      color: white;
    }
    
    .status-badge.moving-forward { background-color: var(--primary); }
    .status-badge.moving-backward { background-color: var(--warning); }
    .status-badge.turning-left, .status-badge.turning-right { background-color: var(--secondary); }
    .status-badge.idle { background-color: var(--gray); }
    
    @media (max-width: 500px) {
      .container {
        padding: 10px;
      }
      
      .card {
        padding: 1rem;
      }
      
      .button {
        padding: 12px 8px;
        font-size: 1rem;
      }
    }
    
    /* Dark mode support */
    @media (prefers-color-scheme: dark) {
      body {
        background-color: #111827;
        color: white;
      }
      
      .card {
        background-color: #1f2937;
      }
      
      .speed-value {
        background-color: #374151;
        color: white;
      }
      
      .key {
        background-color: #374151;
        color: white;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>ESP32 Robot Control</h1>
    </div>
    
    <div class="card">
      <div class="control-grid">
        <div class="cell"></div>
        <div class="cell center-cell">
          <button class="button" id="forward-btn">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M12 5l7 7-7 7M5 12h14"></path>
            </svg>
            Forward
          </button>
        </div>
        <div class="cell"></div>
        
        <div class="cell">
          <button class="button" id="left-btn">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M19 12H5M12 19l-7-7 7-7"></path>
            </svg>
            Left
          </button>
        </div>
        <div class="cell center-cell">
          <button class="button stop" id="stop-btn">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <rect x="6" y="6" width="12" height="12" rx="1"></rect>
            </svg>
            Stop
          </button>
        </div>
        <div class="cell">
          <button class="button" id="right-btn">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M5 12h14M12 5l7 7-7 7"></path>
            </svg>
            Right
          </button>
        </div>
        
        <div class="cell"></div>
        <div class="cell center-cell">
          <button class="button" id="backward-btn">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M12 19l-7-7 7-7M5 12h14"></path>
            </svg>
            Backward
          </button>
        </div>
        <div class="cell"></div>
      </div>
      
      <div class="speed-control">
        <button class="speed-button" id="speed-down">−</button>
        <div class="speed-value" id="speed-display">150</div>
        <button class="speed-button" id="speed-up">+</button>
      </div>
    </div>
    
    <div class="card">
      <div class="status-panel">
        <div class="status-item">
          <div class="status-label">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <circle cx="12" cy="12" r="10"></circle>
              <path d="M12 16v-4M12 8h.01"></path>
            </svg>
            Status:
          </div>
          <span class="status-badge idle" id="robot-status">Idle</span>
        </div>
        
        <div class="status-item">
          <div class="status-label">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <rect x="6" y="7" width="12" height="10" rx="2"></rect>
              <line x1="12" y1="4" x2="12" y2="7"></line>
            </svg>
            Battery: <span id="battery-percent">100%</span>
          </div>
          <div class="battery-bar">
            <div class="battery-level" id="battery-level" style="width:100%; background-color:var(--primary);"></div>
          </div>
        </div>
      </div>
    </div>
    
    <div class="card">
      <div class="keyboard-guide">
        <div class="keyboard-guide-title">
          <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <rect x="2" y="4" width="20" height="16" rx="2" ry="2"></rect>
            <path d="M6 8h.01M10 8h.01M14 8h.01M18 8h.01M8 12h.01M12 12h.01M16 12h.01M7 16h10"></path>
          </svg>
          Keyboard Controls
        </div>
        <span class="key">W</span> <span>Forward</span>
        <span class="key">S</span> <span>Backward</span>
        <span class="key">A</span> <span>Left</span>
        <span class="key">D</span> <span>Right</span>
        <span class="key">Space</span> <span>Stop</span>
        <span class="key">+</span> <span>Increase Speed</span>
        <span class="key">-</span> <span>Decrease Speed</span>
      </div>
    </div>
  </div>

  <script>
    let currentSpeed = 150;
    let batteryLevel = 100;
    let robotStatus = 'idle';
    
    // Control buttons
    const forwardBtn = document.getElementById('forward-btn');
    const backwardBtn = document.getElementById('backward-btn');
    const leftBtn = document.getElementById('left-btn');
    const rightBtn = document.getElementById('right-btn');
    const stopBtn = document.getElementById('stop-btn');
    
    // Speed controls
    const speedDisplay = document.getElementById('speed-display');
    const speedUpBtn = document.getElementById('speed-up');
    const speedDownBtn = document.getElementById('speed-down');
    
    // Status elements
    const statusDisplay = document.getElementById('robot-status');
    const batteryDisplay = document.getElementById('battery-percent');
    const batteryLevelBar = document.getElementById('battery-level');
    
    // Button event handlers
    function setupButton(button, action) {
      // For touch devices
      button.addEventListener('touchstart', (e) => {
        e.preventDefault();
        fetch('/' + action)
          .then(() => updateButtonState(button, true));
      });
      
      button.addEventListener('touchend', (e) => {
        e.preventDefault();
        fetch('/stop')
          .then(() => updateButtonState(button, false));
      });
      
      // For mouse devices
      button.addEventListener('mousedown', () => {
        fetch('/' + action)
          .then(() => updateButtonState(button, true));
      });
      
      button.addEventListener('mouseup', () => {
        fetch('/stop')
          .then(() => updateButtonState(button, false));
      });
      
      button.addEventListener('mouseleave', () => {
        if (button.classList.contains('active')) {
          fetch('/stop')
            .then(() => updateButtonState(button, false));
        }
      });
      
      // Prevent context menu on long press
      button.addEventListener('contextmenu', (e) => {
        e.preventDefault();
      });
    }
    
    function updateButtonState(button, isActive) {
      const allButtons = [forwardBtn, backwardBtn, leftBtn, rightBtn];
      
      // Reset all buttons
      allButtons.forEach(btn => {
        btn.classList.remove('active');
        btn.style.backgroundColor = '';
      });
      
      // Set active state for current button
      if (isActive) {
        button.classList.add('active');
        button.style.backgroundColor = 'var(--primary-dark)';
      }
    }
    
    setupButton(forwardBtn, 'forward');
    setupButton(backwardBtn, 'backward');
    setupButton(leftBtn, 'left');
    setupButton(rightBtn, 'right');
    
    // Stop button just needs a click
    stopBtn.addEventListener('click', () => {
      fetch('/stop').then(() => {
        // Reset all buttons
        updateButtonState(null, false);
      });
    });
    
    // Speed control with visual feedback
    speedUpBtn.addEventListener('click', () => {
      if (currentSpeed < 255) {
        currentSpeed += 10;
        updateSpeed();
        
        // Visual feedback
        speedUpBtn.style.transform = 'scale(0.95)';
        setTimeout(() => {
          speedUpBtn.style.transform = '';
        }, 100);
      }
    });
    
    speedDownBtn.addEventListener('click', () => {
      if (currentSpeed > 10) {
        currentSpeed -= 10;
        updateSpeed();
        
        // Visual feedback
        speedDownBtn.style.transform = 'scale(0.95)';
        setTimeout(() => {
          speedDownBtn.style.transform = '';
        }, 100);
      }
    });
    
    function updateSpeed() {
      speedDisplay.textContent = currentSpeed;
      fetch('/speed?value=' + currentSpeed);
      
      // Visual feedback for speed change
      speedDisplay.style.transform = 'scale(1.1)';
      setTimeout(() => {
        speedDisplay.style.transform = '';
      }, 200);
    }
    
    // Keyboard controls with visual feedback
    document.addEventListener('keydown', (e) => {
      if (e.repeat) return; // Prevent repeat events when key is held
      
      switch(e.key.toUpperCase()) {
        case 'W': 
          fetch('/forward');
          updateButtonState(forwardBtn, true);
          break;
        case 'S': 
          fetch('/backward');
          updateButtonState(backwardBtn, true);
          break;
        case 'A': 
          fetch('/left');
          updateButtonState(leftBtn, true);
          break;
        case 'D': 
          fetch('/right');
          updateButtonState(rightBtn, true);
          break;
        case ' ': 
          fetch('/stop');
          updateButtonState(null, false);
          break;
        case '+': 
        case '=': 
          if (currentSpeed < 255) {
            currentSpeed += 10;
            updateSpeed();
          }
          break;
        case '-': 
        case '_': 
          if (currentSpeed > 10) {
            currentSpeed -= 10;
            updateSpeed();
          }
          break;
      }
    });
    
    document.addEventListener('keyup', (e) => {
      switch(e.key.toUpperCase()) {
        case 'W':
        case 'S':
        case 'A':
        case 'D':
          fetch('/stop');
          updateButtonState(null, false);
          break;
      }
    });
    
    // Status updates with animations
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          robotStatus = data.status;
          batteryLevel = data.battery;
          
          // Update status badge
          statusDisplay.textContent = robotStatus.charAt(0).toUpperCase() + robotStatus.slice(1);
          
          // Remove all status classes
          statusDisplay.className = 'status-badge';
          
          // Add specific status class
          statusDisplay.classList.add(robotStatus.replace(' ', '-'));
          
          // Update battery display
          batteryDisplay.textContent = batteryLevel + '%';
          
          // Update battery bar with animation
          batteryLevelBar.style.width = batteryLevel + '%';
          
          // Change battery color based on level
          if (batteryLevel > 60) {
            batteryLevelBar.style.backgroundColor = 'var(--primary)';
          } else if (batteryLevel > 30) {
            batteryLevelBar.style.backgroundColor = 'var(--warning)';
          } else {
            batteryLevelBar.style.backgroundColor = 'var(--danger)';
          }
        })
        .catch(error => {
          console.error('Error fetching status:', error);
        });
    }
    
    // Update status every second
    setInterval(updateStatus, 1000);
    
    // Initial status update
    updateStatus();
  </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void loop() {
  // Handle client requests from web server
  server.handleClient();
  
  // Handle Bluetooth commands
//   if (SerialBT.available()) {
//     char c = toupper(SerialBT.read());
//     if      (c == 'W') cmdForward();
//     else if (c == 'S') cmdReverse();
//     else if (c == 'A') cmdTurnLeft();
//     else if (c == 'D') cmdTurnRight();
//     else if (c == 'X') cmdStop();
//     else if (c == '+' && currentSpeed < 255) currentSpeed += 10;
//     else if (c == '-' && currentSpeed > 10) currentSpeed -= 10;

//     Serial.print("Received Bluetooth: ");
//     Serial.println(c);
//   }
  
  // Simulate battery drain (very slow)
  if (random(1000) == 1 && batteryLevel > 0) {
    batteryLevel--;
  }
}
