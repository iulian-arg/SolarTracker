
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

  if (msg === "RESTARTING") {
    var status = document.getElementById('configStatus');
    if (status) status.innerHTML = 'Device is restarting...';
    return;
  }

  if (msg === "config_UPDATED") {
    var status = document.getElementById('configStatus');
    if (status) status.innerHTML = 'config configuration updated successfully.';
    return;
  }
  if (msg.indexOf("LOG_ENTRY") >= 0) {
    var logArea = document.getElementById('logArea');
    msg = msg.substring(10); // Remove the prefix
    // if (logArea) logArea.innerHTML+= msg + '\n';
    appendLog(msg);
    return;
  }
  if (msg.indexOf("config_CONFIG") >= 0) {
    msg = msg.replace("config_CONFIG:", "");


    var editor = document.getElementById('configEditor');
    if (editor) {
      try {
        editor.value = msg;
        var status = document.getElementById('configStatus');
        if (status) status.innerHTML = '';
      } catch (e) {
        if (status) status.innerHTML = 'Error formatting config: ' + e.message;
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
    var prefixes = ['LOG:', 'ERR:', 'ERROR', 'config_CONFIG:', 'SET_config:', 'GET_config:', 'RESTART', 'config_UPDATED', 'config_CONFIG'];
    for (var i = 0; i < prefixes.length; i++) {
      if (msg.indexOf(prefixes[i]) === 0) return true;
    }
    // Also allow short status words
    var exact = ['RESTARTING', 'config_UPDATED'];
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

function getconfig() {
  var status = document.getElementById('configStatus');
  try {
    websocket.send('GET_config:');
    if (status) status.innerHTML = 'get config';
  } catch (e) {
    if (status) status.innerHTML = 'Error requesting config: ' + e.message;
  }

}

function sendconfig() {
  var editor = document.getElementById('configEditor');
  var status = document.getElementById('configStatus');
  if (!editor) return;
  var txt = editor.value;
  try {
    websocket.send('SET_config:' + txt);
    if (status) status.innerHTML = 'config sent';
  } catch (e) {
    if (status) status.innerHTML = 'Invalid config: ' + e.message;
  }
}
