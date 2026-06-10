const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const readline = require('readline');

const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');
const helmet = require('helmet');

const net = require('net'); // for QEMU TCP communication


const portPath = 'COM3'; 
const serialPort = new SerialPort({ 
    path: portPath, 
    baudRate: 115200 
});

const parser = serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

const app = express();
app.use(helmet());

const corsOptions = {
  origin: 'http://localhost:3000',
  methods: ['GET', 'POST']
};
app.use(cors(corsOptions));

const server = http.createServer(app);
const io = new Server(server, { cors: corsOptions });


//=== QEMU tcp bridge set up===
let secureWorldSocket = null;

const tcpServer = net.createServer((socket) => {
  console.log('[gateway] optee on qemu connected via TCP');
  secureWorldSocket = socket;

  // listen for signed data coming back from optee
  socket.on('data', (data) => {
    console.log(`[gateway <- optee] signature: ${data.toString()}`);
    // emit data to dashboard (react)
    io.emit('secure_telemetry', { status: 'Verified', hash: data.toString() });
  });

  socket.on('error', (err) => console.error('[gateway] TCP err:', err.message));
  socket.on('close', () => {
    console.log('[gateway] optee on qemu disconnected');
    secureWorldSocket = null;
  });
});

tcpServer.listen(5000, '0.0.0.0', () => {
  console.log('[gateway] TCP server listening for QEMU on port 5000');
});



//=======
// hardware communication
function sendCommand(command) {
  const allowedCommands = ['led_on', 'led_off'];
  if (!allowedCommands.includes(command)) {
    console.warn(`[security] blocked invalid command: ${command}`);
    return;
  }

  if (serialPort.isOpen) {
    console.log(`[gateway -> node] Executing: ${command}`);
    serialPort.write(`${command}\n`);
  }
}


io.on('connection', (socket) => {
  console.log(`[gateway] dashboard client connected: ${socket.id}`);

  socket.on('toggle_actuator', (data) => {
    if (data === 'ON') sendCommand('led_on');
    else if (data === 'OFF') sendCommand('led_off');
  });

  socket.on('disconnect', () => {
    console.log(`[gateway] client disconnected: ${socket.id}`);
  });
});

// parser.on('data', (data) => {
//   try {
//     const parsedData = JSON.parse(data.trim());
//     io.emit('telemetry', parsedData); 
//   } catch (err) {
//     console.error(`[gateway] err parsing serial data: ${err.message}`);
//   }
// });

parser.on('data', (data) => {
  try {
    const rawTelemetry = data.trim();
    console.log(`[Edge -> Gateway] Raw: ${rawTelemetry}`);

    const match = rawTelemetry.match(/{"payload": (.*), "hmac": "(.*)"}/);
    
    if (match && secureWorldSocket) {
      const exactPayload = match[1].trim();
      const exactHmac = match[2].trim();
      
      // pipe data to optee w format payload|hmac\n
      secureWorldSocket.write(`${exactPayload}|${exactHmac}\n`);
    } else if (!secureWorldSocket) {
      console.warn('[gateway] optee offline. dropping telemetry');
    }
  } catch (err) {
    console.error('[gateway] Data err:', err.message);
  }
});

server.listen(4000, () => {
  console.log('[gateway] HTTP & WebSocket server listening on port 4000');
});

//==================
// local termial input (for TESTING)
const terminalInput = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

terminalInput.on('line', (input) => {
  const cmd = input.trim();
  if (cmd) {
    sendCommand(cmd);
  }
});