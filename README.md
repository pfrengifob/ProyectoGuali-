# ProyectoGuali-
Protecto Sistemas Embebidos PAO 1 2024-2025

Este proyecto consiste en un sistema de control de gestos faciales utilizando una aplicación móvil. El sistema permite el control de las cejas mediante servomotores, la visualización de ojos en un display LCD, y la expresión de emociones a través de tiras LED. Además, incluye funcionalidades de conexión Wi-Fi y control remoto a través de un microcontrolador ESP32S.

## Características

- **Control de Cejas:** Utiliza dos servomotores de 180 grados para controlar el movimiento de las cejas.
- **Visualización de Ojos:** Un display LCD muestra imágenes de ojos, ajustables a través de la aplicación móvil.
- **Expresiones Faciales:** Tiras LED RGB controladas por un módulo de 4 relés 
- **Conectividad Wi-Fi:** Soporte para Access Point y conexión a redes Wi-Fi existentes, con almacenamiento de credenciales en EEPROM.
- **Encendido/Apagado por Sensor Infrarrojo:** El sistema se enciende o apaga al detectar la proximidad de la mano.
- **Alimentación Portátil:** Funcionamiento con alimentación de 5V, lo que permite su uso en diferentes entornos.

## Requisitos

### Hardware:
- Microcontrolador ESP32S
- 2 Servomotores SG90 (180 grados)
- Display LCD I2C
- Tiras LED RGB 5V
- Módulo de 4 relés
- Sensor infrarrojo
- PCB personalizado con pines para ESP32S y pistas de GND/Vcc

### Software:
- [App Inventor](http://appinventor.mit.edu/) para la creación de la aplicación móvil
- Visual Studio Code con la extensión Platformio para la programación del ESP32S
