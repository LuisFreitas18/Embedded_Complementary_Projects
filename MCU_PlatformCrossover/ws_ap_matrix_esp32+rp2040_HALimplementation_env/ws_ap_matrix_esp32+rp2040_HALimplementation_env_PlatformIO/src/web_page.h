#ifndef WEB_PAGE_H
#define WEB_PAGE_H

static const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Matriz ESP-IDF</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #121212; color: white; margin: 0; padding: 20px; }
    h2 { margin-bottom: 20px; }
    .grid { display: grid; grid-template-columns: repeat(3, 80px); gap: 10px; justify-content: center; margin: 20px auto; }
    .cell { width: 80px; height: 50px; text-align: center; font-size: 18px; border-radius: 8px; border: 1px solid #444; background: #1e1e1e; color: #00ffcc; font-weight: bold; }
    .status { margin-top: 15px; font-size: 14px; color: #aaa; }
    .status.online { color: #00ff66; }
  </style>
</head>
<body>
  <h2>Controlo da Matriz 2D (ESP-IDF)</h2>
  <div id="status" class="status">Desconectado</div>
  <div class="grid" id="gridContainer"></div>

  <script>
    const ROWS = 3, COLS = 3;
    let gateway = `ws://${window.location.hostname}/ws`;
    let websocket;

    function applyMatrix(values) {
      for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
          const input = document.getElementById(`cell_${r}_${c}`);
          if (input && values[r] && typeof values[r][c] !== 'undefined') {
            input.value = values[r][c];
          }
        }
      }
    }

    async function loadInitialMatrix() {
      try {
        const response = await fetch('/matrix', { cache: 'no-store' });
        if (!response.ok) return;

        const data = await response.json();
        if (Array.isArray(data.values)) {
          applyMatrix(data.values);
        }
      } catch (error) {
        console.warn('Failed to load initial matrix state', error);
      }
    }

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = () => {
        document.getElementById('status').innerText = 'Conectado ao ESP32';
        document.getElementById('status').className = 'status online';
      };
      websocket.onclose = () => {
        document.getElementById('status').innerText = 'Reconectando...';
        document.getElementById('status').className = 'status';
        setTimeout(initWebSocket, 2000);
      };
      websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        const input = document.getElementById(`cell_${data.row}_${data.col}`);
        if (input) input.value = data.value;
      };
    }

    function createGrid() {
      const container = document.getElementById('gridContainer');
      for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
          let input = document.createElement('input');
          input.type = 'number';
          input.className = 'cell';
          input.id = `cell_${r}_${c}`;
          input.value = 0;
          input.onchange = () => {
            if (websocket && websocket.readyState === WebSocket.OPEN) {
              websocket.send(JSON.stringify({ row: r, col: c, value: parseInt(input.value) || 0 }));
            }
          };
          container.appendChild(input);
        }
      }
    }

    window.onload = async () => {
      createGrid();
      await loadInitialMatrix();
      initWebSocket();
    };
  </script>
</body>
</html>
)rawliteral";

#endif