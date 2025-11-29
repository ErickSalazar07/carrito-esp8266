from flask import Flask, render_template
from flask_socketio import SocketIO
import socket
import threading
import time
from datetime import datetime

app = Flask(__name__)
app.config['SECRET_KEY'] = 'robot_secret_2024'
socketio = SocketIO(app, cors_allowed_origins="*")

ESP_IP = "10.69.108.75"
ESP_PORT = 5000
RECONNECT_DELAY = 1  # Segundos entre intentos de reconexión
MAX_RECONNECT_ATTEMPTS = 0  # 0 = infinito
SOCKET_TIMEOUT = 5  # Timeout para operaciones de socket

esp_connected = False
esp_socket = None
should_run = True  # Control para detener el hilo de reconexión
reconnect_attempts = 0


def conectar_esp():
    """Conecta al ESP8266 y procesa datos. Se reconecta automáticamente."""
    global esp_connected, esp_socket, should_run, reconnect_attempts

    while should_run:
        try:
            reconnect_attempts += 1
            print(f"🔄 Intento {reconnect_attempts}: Conectando al ESP8266 en {ESP_IP}:{ESP_PORT}...")
            
            esp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            esp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            esp_socket.settimeout(SOCKET_TIMEOUT)
            
            esp_socket.connect((ESP_IP, ESP_PORT))
            
            # Configurar para recepción continua
            esp_socket.settimeout(1.0)  # Timeout corto para recv
            
            esp_connected = True
            reconnect_attempts = 0
            print(f"✅ Conectado al ESP8266 en {ESP_IP}:{ESP_PORT}")
            socketio.emit('estado_esp', {'conectado': True})

            buffer = ""
            sin_datos_contador = 0
            MAX_SIN_DATOS = 30  # 30 segundos sin datos = reconectar

            while esp_connected and should_run:
                try:
                    chunk = esp_socket.recv(1024).decode('utf-8', errors='ignore')

                    if not chunk:
                        print("⚠️ Conexión cerrada por el ESP (recv vacío)")
                        break
                    
                    sin_datos_contador = 0  # Resetear contador
                    buffer += chunk

                    # Procesar todas las líneas completas
                    while "\n" in buffer:
                        linea, buffer = buffer.split("\n", 1)
                        linea = linea.strip()

                        if not linea:
                            continue

                        partes = linea.split(",")

                        if len(partes) != 5:
                            print("⚠️ Línea inválida (esperaba 5 valores):", linea[:100])
                            continue

                        try:
                            x_pos = float(partes[0])
                            y_pos = float(partes[1])
                            vel_izq = float(partes[2])
                            vel_der = float(partes[3])
                            vel_prom = float(partes[4])

                            datos = {
                                "timestamp": datetime.now().strftime("%H:%M:%S"),
                                "x_pos": x_pos,
                                "y_pos": y_pos,
                                "vel_izq": vel_izq,
                                "vel_der": vel_der,
                                "vel_prom": vel_prom
                            }

                            socketio.emit('nuevo_dato', datos)
                            print(f"📡 Datos: X={x_pos:.3f}m, Y={y_pos:.3f}m, VProm={vel_prom:.3f}m/s")

                        except ValueError as e:
                            print(f"⚠️ Valores no numéricos: {linea[:100]}")
                            continue

                except socket.timeout:
                    # Timeout normal, seguir esperando
                    sin_datos_contador += 1
                    if sin_datos_contador >= MAX_SIN_DATOS:
                        print(f"❌ Sin datos por {MAX_SIN_DATOS}s, reconectando...")
                        break
                    continue
                    
                except UnicodeDecodeError as e:
                    print(f"⚠️ Error de decodificación: {e}")
                    continue
                    
                except Exception as e:
                    print(f"❌ Error recibiendo datos: {e}")
                    break

        except socket.timeout:
            print(f"❌ Timeout al conectar al ESP ({SOCKET_TIMEOUT}s)")
        except ConnectionRefusedError:
            print(f"❌ Conexión rechazada por {ESP_IP}:{ESP_PORT}")
        except OSError as e:
            print(f"❌ Error de red: {e}")
        except Exception as e:
            print(f"❌ Error inesperado: {e}")

        finally:
            # Limpiar conexión
            if esp_socket:
                try:
                    esp_socket.shutdown(socket.SHUT_RDWR)
                    esp_socket.close()
                except:
                    pass
                esp_socket = None
            
            esp_connected = False
            socketio.emit('estado_esp', {'conectado': False})
            
            if esp_connected:
                print("🔌 Desconectado del ESP")

            # Esperar antes de reintentar (solo si debe seguir corriendo)
            if should_run:
                if MAX_RECONNECT_ATTEMPTS > 0 and reconnect_attempts >= MAX_RECONNECT_ATTEMPTS:
                    print(f"❌ Máximo de intentos alcanzado ({MAX_RECONNECT_ATTEMPTS})")
                    break
                    
                print(f"⏳ Reintentando en {RECONNECT_DELAY} segundos...")
                time.sleep(RECONNECT_DELAY)


@app.route('/')
def index():
    return render_template('index.html')


@socketio.on('connect')
def handle_connect():
    print("🟢 Cliente conectado")
    socketio.emit('estado_esp', {'conectado': esp_connected})


@socketio.on('disconnect')
def handle_disconnect():
    print("🔴 Cliente desconectado")


if __name__ == "__main__":
    # Iniciar hilo de reconexión automática
    thread = threading.Thread(target=conectar_esp, daemon=True)
    thread.start()
    
    print("🚀 Servidor iniciado en http://localhost:5001")
    print("🔄 Reconexión automática activada")
    
    try:
        socketio.run(app, host='0.0.0.0', port=5001, debug=True, use_reloader=False)
    except KeyboardInterrupt:
        print("\n⏹️ Deteniendo servidor...")
        should_run = False