#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// --- WIFI CREDENTIALS ---
const char* ssid = "MonkeyLabs";
const char* password = "12345678";

// --- HARDWARE SETTINGS ---
#define LASER_PIN 14 // D5 on NodeMCU
const int BIT_PERIOD = 50; // 50ms per bit (Adjust based on receiver speed)

ESP8266WebServer server(80);

// --- PART 1: HTML & CSS (Dark Theme UI) ---
const char html_part1[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Li-Fi Command Center</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600&display=swap" rel="stylesheet">
  <style>
    /* Black, Red & Purple Cyberpunk Theme */
    body { font-family: 'Inter', sans-serif; background-color: #000000; color: #e0e0e0; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
    
    .card { 
      background: #111111; 
      width: 90%; 
      max-width: 400px; 
      padding: 25px; 
      border-radius: 16px; 
      box-shadow: 0 0 30px rgba(142, 68, 173, 0.15); 
      text-align: center; 
      border: 1px solid #222;
    }
    
    h1 { margin-bottom: 25px; color: #ffffff; font-weight: 600; font-size: 24px; text-shadow: 0 0 10px rgba(142, 68, 173, 0.5); }
    
    /* Dark Theme Textarea */
    textarea { 
        width: 100%; 
        height: 100px; 
        padding: 12px 15px; 
        margin-bottom: 5px; 
        background-color: #050505;
        color: #d1d1d1;
        border: 1px solid #333; 
        border-radius: 8px; 
        font-family: monospace; 
        font-size: 16px; 
        box-sizing: border-box; 
        transition: 0.3s; 
        outline: none; 
        resize: vertical;
    }
    textarea:focus { border-color: #8e44ad; background-color: #000; box-shadow: 0 0 10px rgba(142, 68, 173, 0.2); }
    textarea::placeholder { color: #444; }
    
    .char-count { text-align: right; font-size: 12px; color: #666; margin-bottom: 15px; }

    .btn { width: 100%; padding: 12px; margin-bottom: 10px; border: none; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; transition: 0.2s; text-transform: uppercase; letter-spacing: 1px; }
    
    /* Purple Send Button */
    .btn-send { background: linear-gradient(135deg, #8e44ad, #6c3483); color: white; box-shadow: 0 4px 15px rgba(142, 68, 173, 0.3); }
    .btn-send:hover { background: linear-gradient(135deg, #9b59b6, #7d3c98); transform: translateY(-1px); }
    .btn-send:disabled { background: #222; color: #555; cursor: not-allowed; box-shadow: none; }
    
    /* Red Align Button */
    .btn-align { background-color: #1a1a1a; color: #e74c3c; border: 1px solid #e74c3c; }
    .btn-align:hover { background-color: #e74c3c; color: white; }
    .btn-align.active { background-color: #c0392b; color: white; border-color: #c0392b; box-shadow: 0 0 15px rgba(192, 57, 43, 0.5); } 
    
    #status { margin-top: 15px; font-size: 14px; color: #888; min-height: 20px; }
  </style>
</head>
<body>
<div class="card">
  <h1>Li-Fi Transmitter</h1>
  
  <textarea id="msgInput" placeholder="Type your message here...
Ex: Hello World! 123." autocomplete="off" oninput="updateCount()"></textarea>
  <div class="char-count" id="charCount">0 characters</div>

  <button id="sendBtn" class="btn btn-send" onclick="sendMessage()">Transmit Data</button>
  <button id="alignBtn" class="btn btn-align" onclick="toggleAlign()">Align Laser (OFF)</button>
  <div id="status">Ready to transmit</div>
</div>
)rawliteral";

// --- PART 2: JAVASCRIPT ---
const char html_part2[] PROGMEM = R"rawliteral(
<script>
  let isAligning = false;

  function updateCount() {
    const len = document.getElementById("msgInput").value.length;
    document.getElementById("charCount").innerText = len + " characters";
  }

  function sendMessage() {
    var msg = document.getElementById("msgInput").value;
    var sendBtn = document.getElementById("sendBtn");
    var alignBtn = document.getElementById("alignBtn");
    var status = document.getElementById("status");

    if(msg === "") {
      status.innerText = "Please enter a message!";
      status.style.color = "#e74c3c";
      return;
    }

    // Turn off alignment if it was on
    if(isAligning) {
       isAligning = false;
       alignBtn.innerText = "Align Laser (OFF)";
       alignBtn.classList.remove("active");
    }

    sendBtn.disabled = true;
    sendBtn.innerText = "Transmitting...";
    status.innerText = "Encoding & sending...";
    status.style.color = "#f39c12";

    var xhr = new XMLHttpRequest();
    // encodeURIComponent ensures symbols (+, &, ?, space) are sent correctly over URL
    xhr.open("GET", "/send?msg=" + encodeURIComponent(msg), true);
    
    xhr.onload = function() {
      if (xhr.status === 200) {
        status.innerText = "Transmission Complete!";
        status.style.color = "#27ae60";
        document.getElementById("msgInput").value = "";
        updateCount();
      } else {
        status.innerText = "Error sending message.";
        status.style.color = "#e74c3c";
      }
      sendBtn.disabled = false;
      sendBtn.innerText = "Transmit Data";
    };
    xhr.send();
  }

  function toggleAlign() {
    var alignBtn = document.getElementById("alignBtn");
    var status = document.getElementById("status");
    isAligning = !isAligning; 
    var stateVal = isAligning ? "1" : "0";
    
    if(isAligning) {
        alignBtn.innerText = "Stop Alignment (Laser ON)";
        alignBtn.classList.add("active");
        status.innerText = "Laser ON (Steady Beam)";
    } else {
        alignBtn.innerText = "Align Laser (OFF)";
        alignBtn.classList.remove("active");
        status.innerText = "Laser OFF";
    }

    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/align?state=" + stateVal, true);
    xhr.send();
  }
</script>
</body>
</html>
)rawliteral";

// --- LASER LOGIC (Universal ASCII) ---
// This function sends ANY standard ASCII character (0-255).
// This includes 'A', 'z', '0', '9', '!', '@', '\n', ' ' etc.
void sendChar(char c) {
  digitalWrite(LASER_PIN, LOW); // Start Bit (Active LOW for most laser modules)
  delay(BIT_PERIOD);
  
  // Send 8 bits of data (LSB first)
  for (int i = 0; i < 8; i++) {
    int bitToSend = (c >> i) & 1; 
    if (bitToSend == 1) digitalWrite(LASER_PIN, LOW);  // 1 = Laser ON
    else digitalWrite(LASER_PIN, HIGH);                // 0 = Laser OFF
    delay(BIT_PERIOD);
  }
  
  digitalWrite(LASER_PIN, HIGH); // Stop Bit (Laser OFF)
  delay(BIT_PERIOD * 2); // Wait 2 bit periods before next char
}

// --- SERVER HANDLERS ---
void handleRoot() {
  String page = String(FPSTR(html_part1)) + String(FPSTR(html_part2));
  server.send(200, "text/html", page);
}

void handleAlign() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "1") {
      digitalWrite(LASER_PIN, LOW); // ON
    } else {
      digitalWrite(LASER_PIN, HIGH); // OFF
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleSend() {
  if (server.hasArg("msg")) {
    String message = server.arg("msg");
    
    // Ensure Alignment is OFF before starting data
    digitalWrite(LASER_PIN, HIGH); 
    
    Serial.print("Sending: ");
    Serial.println(message); // Debug to Serial Monitor

    for (int i = 0; i < message.length(); i++) {
      sendChar(message[i]);
    }
    
    digitalWrite(LASER_PIN, HIGH); // Ensure OFF after done

    server.send(200, "text/plain", "Sent");
  }
}

void setup() {
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, HIGH); // Start OFF

  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Li-Fi Transmitter Ready. IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/align", handleAlign);

  server.begin();
}

void loop() {
  server.handleClient();
}