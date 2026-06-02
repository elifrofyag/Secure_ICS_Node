const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const readline = require('readline');

const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');
const helmet = require('helmet');


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

parser.on('data', (data) => {
  try {
    const parsedData = JSON.parse(data.trim());
    io.emit('telemetry', parsedData); 
  } catch (err) {
    console.error(`[gateway] err parsing serial data: ${err.message}`);
  }
});

server.listen(4000, () => {
  console.log('[gateway] secure server on http://localhost:4000');
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