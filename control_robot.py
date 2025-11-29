import socket
import keyboard
import time

IP_ESP = "10.69.108.75"
PUERTO = 5000

def main():
    print("Conectando al robot...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((IP_ESP, PUERTO))
    print("Conectado!")

    print("\nControles:")
    print(" W = Avanzar 1s")
    print(" A = Girar izquierda 90°")
    print(" D = Girar derecha 90°")
    print(" Q = Salir\n")

    while True:
        if keyboard.is_pressed("w"):
            s.send(b"W")
            print("W")
            time.sleep(0.2)     # evita repetir muchas veces

        if keyboard.is_pressed("a"):
            s.send(b"A")
            print("A")
            time.sleep(0.2)

        if keyboard.is_pressed("d"):
            s.send(b"D")
            print("D")
            time.sleep(0.2)

        if keyboard.is_pressed("q"):
            print("Saliendo...")
            break

    s.close()

if __name__ == "__main__":
    main()
