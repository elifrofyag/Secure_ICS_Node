const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const readline = require('readline');

const portPath = 'COM3'; 

const port = new SerialPort({
  path: portPath,
  baudRate: 115200,
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));


function sendCommand(command) {
  if (port.isOpen) {
    console.log(`[gateway -> node] Executing: ${command}`);
    port.write(`${command}\n`, (err) => {
      if (err) {
        console.error('[gateway] Failed to write to serial port:', err.message);
      }
    });
  } else {
    console.error('[gateway] Cannot send command: Serial port is closed.');
  }
}


port.on('open', () => {
  console.log(`[gateway] connected to edge node on ${portPath}`);
});

parser.on('data', (data) => {
  try {
    const parsedData = JSON.parse(data.trim());
    console.log('[node -> gateway] ', parsedData);


  } catch (err) {
    console.log('[node -> gateway] err:', data.trim());
  }
});

port.on('error', (err) => {
  console.error('[gateway] Serial Error: ', err.message);
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