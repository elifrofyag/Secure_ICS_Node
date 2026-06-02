import React, { useState, useEffect } from 'react';
import io from 'socket.io-client';
import './App.css';

const socket = io('http://localhost:4000');

function App() {
  const [moisture, setMoisture] = useState(0);
  const [isValveOpen, setIsValveOpen] = useState(false);

  useEffect(() => {
    socket.on('secure_telemetry', (data) => {
      console.log('[dashboard <- gateway] Received telemetry:', data);
      if (data && data.soil_moisture !== undefined) {
        setMoisture(data.soil_moisture);
      }
    });

    return () => {
      socket.off('secure_telemetry');
    };
  }, []);

  const handleActuator = () => {
    const newState = !isValveOpen;
    setIsValveOpen(newState);
    socket.emit('toggle_actuator', newState ? 'ON' : 'OFF');
  };

  return (
    <div className="dashboard-container">
      <header className="header">
        <h1>ICS Gateway</h1>
      </header>

      <main className="surface-grid">
        <section className="data-surface">
          <h2>Soil Moisture</h2>
          <div className="telemetry-readout">
            {moisture} <span className="unit">ADC</span>
          </div>
        </section>

        <section className="control-surface">
          <h2>Actuator Control</h2>
          <button 
            className={`action-button ${isValveOpen ? 'active' : ''}`}
            onClick={handleActuator}
          >
            {isValveOpen ? 'CLOSE VALVE' : 'OPEN VALVE'}
          </button>
        </section>
      </main>
    </div>
  );
}

export default App;