
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", onload);

function onload(event) {
  initWebSocket();
}

function initWebSocket() {
  console.log("Trying to open a WebSocket connection…");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log("Connection opened");
  appendLog('WebSocket: Connection opened');
}

function onClose(event) {
  console.log("Connection closed");
  appendLog('WebSocket: Connection closed');
  setTimeout(initWebSocket, 2000);
}

function sendCommand(element, command) {
  console.log("Send command: " + command);
  websocket.send(command);
}

function onMessage(event) {
  console.log(event.data);
  var msg = event.data;

  // Append incoming messages to the log area and auto-scroll
  // if (shouldLog(msg)) appendLog('RX: ' + msg);

  if (msg === "RESTARTING") {
    var status = document.getElementById('jsonStatus');
    if (status) status.innerHTML = 'Device is restarting...';
    return;
  }

  if (msg === "JSON_UPDATED") {
    var status = document.getElementById('jsonStatus');
    if (status) status.innerHTML = 'JSON configuration updated successfully.';
    return;
  }
  if (msg.indexOf("LOG_ENTRY") >= 0) {
    var logArea = document.getElementById('logArea');
    msg = msg.substring(10); // Remove the prefix
    // if (logArea) logArea.innerHTML+= msg + '\n';
    appendLog(msg);
    return;
  }
  if (msg.indexOf("JSON_CONFIG") >= 0) {
    msg = msg.substring(12); // Remove the prefix

    var myObj = JSON.parse(msg);

    // Populate the raw JSON editor (if present) with a pretty-printed document
    var editor = document.getElementById('jsonEditor');
    if (editor) {
      try {
        editor.value = JSON.stringify(myObj, null, 2);
        var status = document.getElementById('jsonStatus');
        if (status) status.innerHTML = '';
      } catch (e) {
        if (status) status.innerHTML = 'Error formatting JSON: ' + e.message;
      }
    }
  }

}

// Append a line to the read-only log area and keep it scrolled to the bottom
function appendLog(text) {
  try {
    var area = document.getElementById('logArea');
    if (!area) return;
    area.value += text + '\n';
    // Scroll to bottom only if auto-scroll is enabled
    var auto = document.getElementById('autoScroll');
    if (!auto || auto.checked) {
      area.scrollTop = area.scrollHeight;
    }
  } catch (e) {
    console.log('appendLog error:', e);
  }
}

// Decide whether a received message should be appended to the log
function shouldLog(msg) {
  try {
    var showAll = document.getElementById('showAllLogs');
    if (showAll && showAll.checked) return true;
    if (!msg) return false;
    // Allowlist prefixes and exact messages
    var prefixes = ['LOG:', 'ERR:', 'ERROR', 'JSON_CONFIG:', 'SET_JSON:', 'GET_JSON:', 'RESTART', 'JSON_UPDATED', 'JSON_CONFIG'];
    for (var i = 0; i < prefixes.length; i++) {
      if (msg.indexOf(prefixes[i]) === 0) return true;
    }
    // Also allow short status words
    var exact = ['RESTARTING', 'JSON_UPDATED'];
    for (var j = 0; j < exact.length; j++) {
      if (msg === exact[j]) return true;
    }
    return false;
  } catch (e) {
    return true;
  }
}

function clearLog() {
  var area = document.getElementById('logArea');
  if (area) area.value = '';
}

function onLogSettingChange() {
  // placeholder in case we want to persist or react to setting changes
}

function getJSON() {
  var status = document.getElementById('jsonStatus');
  try {
    websocket.send('GET_JSON:');
    if (status) status.innerHTML = 'get JSON';
  } catch (e) {
    if (status) status.innerHTML = 'Error requesting JSON: ' + e.message;
  }

}

function sendJSON() {
  var editor = document.getElementById('jsonEditor');
  var status = document.getElementById('jsonStatus');
  if (!editor) return;
  var txt = editor.value;
  try {
    // Validate JSON before sending
    var parsed = JSON.parse(txt);
    // Send the JSON string to the server. Prefix used so server can distinguish (adjust server if needed)
    websocket.send('SET_JSON:' + JSON.stringify(parsed));
    if (status) status.innerHTML = 'JSON sent';
  } catch (e) {
    if (status) status.innerHTML = 'Invalid JSON: ' + e.message;
  }
}

function formatJSON() {
  var editor = document.getElementById('jsonEditor');
  var status = document.getElementById('jsonStatus');
  if (!editor) return;
  try {
    var parsed = JSON.parse(editor.value);
    editor.value = JSON.stringify(parsed, null, 2);
    if (status) status.innerHTML = 'Formatted';
  } catch (e) {
    if (status) status.innerHTML = 'Invalid JSON: ' + e.message;
  }
}
