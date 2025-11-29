# Proyecto de Física Mecánica — Robot con ESP8266, Motores DC, Encoders y Telemetría en Tiempo Real

Este proyecto implementa un vehículo experimental utilizado para prácticas de **física mecánica**, permitiendo medir posición, velocidades y trayectoria en tiempo real mediante un ESP8266, encoders, sensores IR y un sistema de visualización web.

Incluye control manual desde PC, telemetría continua y graficación en tiempo real mediante WebSockets.

---

## ⚠️ Aviso Importante sobre la Red WiFi

En los archivos del ESP8266 se utiliza una **red WiFi de prueba**, creada exclusivamente para el proyecto académico.

- **No es una red personal.**
- **La contraseña es completamente de testing.**
- **No se expone ninguna clave sensible en el repositorio.**

Esto se especifica para evitar confusiones o supuestos riesgos de seguridad.

---

## 🛠️ Hardware Utilizado

- **ESP8266 NodeMCU**
- **Driver L298N**
- **Motores DC** (2 unidades)
- **Encoders digitales** para medición de velocidad
- **Sensores infrarrojos IR**
- **Base de acrílico** para montaje estructural
- Cableado Dupont y tornillería
- Alimentación 7–12V para motores

El montaje final fija motores, driver, encoders y sensores sobre la base de acrílico, permitiendo establecer un vehículo estable y adecuado para experimentos de cinemática.

---

## 💸 Costos Aproximados

El proyecto tuvo un costo estimado de:

> **≈ 150.000 COP**

Incluyendo electrónica, base acrílica, motores, sensores y materiales de montaje.

---

## 🧩 Arquitectura del Sistema

ESP8266 → Socket TCP → Servidor Python (Flask + SocketIO) → Página Web en tiempo real

### Flujo de funcionamiento:

1. El **ESP8266** se conecta a la red WiFi de prueba.
2. Establece un **socket TCP** directo con el PC.
3. Envía continuamente:
   - Posición X
   - Posición Y
   - Velocidad izquierda
   - Velocidad derecha
   - Velocidad promedio
4. El servidor (`app.py`) procesa los datos y los reenvía vía **SocketIO**.
5. La interfaz web (`index.html`) grafica y muestra todo en tiempo real.

---

## 📁 Estructura del Proyecto

.<br>
├── app.py <br>
├── control\_por\_comando\_esp8266.ino<br>
├── control\_robot.py<br>
├── enviar\_datos\_velocidad\_esp8266.ino<br>
├── LICENSE<br>
├── README.md<br>
├── static<br>
│   └── style.css<br>
└── templates<br>
    └── index.html<br>

### Descripción de Archivos

#### **app.py**

Servidor principal en Flask + SocketIO.

- Maneja la conexión TCP con el ESP8266.
- Reconecta automáticamente si la comunicación se cae.
- Reenvía datos al cliente web en tiempo real.
- Sirve la página principal (`index.html`).

#### **control_por_comando_esp8266.ino**

Firmware para el ESP8266 que recibe comandos del PC (W, A, D, etc.) para controlar el movimiento del robot mediante el L298N.

#### **enviar_datos_velocidad_esp8266.ino**

Sketch del ESP8266 encargado de:

- Leer encoders y sensores IR.
- Calcular posición y velocidades.
- Enviar los datos al servidor por socket TCP.

#### **control_robot.py**

Script en Python que permite controlar el robot desde el teclado:

- `W` → Avanzar
- `A` → Girar izquierda
- `D` → Girar derecha
- `Q` → Salir

#### **/templates/index.html**
Interfaz web que muestra:

- Estado de conexión
- Gráficas en tiempo real
- Lectura de sensores y trayectoria

#### **/static/style.css**
Estilos para la interfaz gráfica.

---

## 🌐 Página Web en Tiempo Real

La interfaz web permite:

- Ver la trayectoria del robot.
- Graficar velocidades izquierda, derecha y promedio.
- Mostrar posición en ejes X–Y.
- Mostrar estado de conexión del ESP8266.
- Actualizar datos en tiempo real mediante SocketIO.

---

## 🧑‍🤝‍🧑 Contribuidores

- **Erick Salazar** — Contribuidor del proyecto 
- **Eduardo Díaz Rojas** — Contribuidor del proyecto  

---

## 📜 Licencia

El proyecto incluye archivo `LICENSE` en el repositorio.

---

## 📌 Notas Finales

Este proyecto fue desarrollado con fines académicos para el curso de **física mecánica**, demostrando conceptos de:

- Cinemática
- Medición de velocidad real
- Control de movimiento
- Transmisión y análisis de datos en tiempo real

El sistema completo puede ser extendido para proyectos de robótica, control dinámico, SLAM básico y análisis experimental.

