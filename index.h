#include "pgmspace.h"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>Robot Laberinto Configuración</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <style>
        html { font-family: Arial; display: inline-block; text-align: center; }
        body { margin: 0; background-color: #f4f4f4; }
        .topnav { overflow: hidden; background-color: #12125E; color: white; font-size: 0.9rem; padding: 5px; }
        .content { padding: 20px; }
        .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5); max-width: 400px; margin: 20px auto; padding: 20px 30px; border-radius: 10px; }
        .card-header { padding: 10px; font-size: 1.5rem; border-bottom: 1px solid #ddd; margin-bottom: 20px; }
        .text-input { font-size: 1.2rem; margin-bottom: 15px; display: flex; justify-content: space-between; align-items: center; }
        .text-input input { padding: 5px; font-size: 1.2rem; width: 80px; border-radius: 8px; border: 1px solid #12125E; text-align: center; }
        
        .submit-button input { width: 220px; padding: 12px; border: 1px solid #12125E; border-radius: 10px; background-color: #12125E; color: white; font-size: 1.4rem; cursor: pointer; transition: 0.3s; }
        .submit-button input:hover { background-color: #31318F; }
        
        /* Clase para el botón cuando está en estado "Apagar" */
        .btn-off { background-color: #8B0000 !important; border-color: #8B0000 !important; }
        .btn-off:hover { background-color: #A52A2A !important; }
    </style>
</head>
<body>
    <div class="topnav">
        <h1>Robot Laberinto Configuración</h1>
    </div>
    <div class="content">
        <div class="card">
            <div class="card-header">Parámetros PID</div>
            <form action="/update">
                <div class="text-input">
                    <label for="kp">Constante Kp</label>
                    <input id="kp" name="kp" type="number" step="0.001" value="%KP%">
                </div>
                <div class="text-input">
                    <label for="ki">Constante Ki</label>
                    <input id="ki" name="ki" type="number" step="0.001" value="%KI%">
                </div>
                <div class="text-input">
                    <label for="kd">Constante Kd</label>
                    <input id="kd" name="kd" type="number" step="0.001" value="%KD%">
                </div>
                <div class="text-input">
                    <label for="max_speed">Vel. Máxima</label>
                    <input id="max_speed" name="max_speed" type="number" min="0" max="100" value="%MAX_SPEED%">
                </div>
                <div class="submit-button">
                    <input type="submit" value="Actualizar">
                </div>
            </form>
        </div>

        <div class="card">
            <form action="/toggle">
                <div class="submit-button">
                    <input type="submit" name="state" id="btn-power" value="%STATE%" onclick="toggleText()">
                </div>
            </form>
        </div>
    </div>

    <script>
        // Lógica para mantener el color correcto al cargar la página
        window.onload = function() {
            var btn = document.getElementById("btn-power");
            if (btn.value === "Encender") {
                btn.classList.add("btn-off");
                btn.value = "Apagar";
            } else {
                btn.classList.remove("btn-off");
                btn.value = "Encender";
            }
        };
    </script>
</body>
</html>
)rawliteral";