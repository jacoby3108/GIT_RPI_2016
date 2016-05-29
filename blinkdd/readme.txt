Les pasamos una carpeta llamada blinkdd que contiene todos los codigos fuentes del driver asi como algunos programas de prueba.

La idea es copiar esta carpeta a la Raspberry pi para compilar el driver

para lo cual vamos al directorio que contiene la carpeta en la pc y hacemos desde la pc

scp -r ./blinkdd/   pi@10.42.0.93:/home/pi/rfdany/rf-bitbanger/blinkdd/

luego: 

1 - logearse a rpi 
2-  Ir a la carpeta /home/pi/rfdany/rf-bitbanger/blinkdd/
3-  Nos pasamos a SU  (sudo su)
4-  Compilar driver: make KERNELDIR=/home/pi/rfdany/raspberrypi-linux-6f2064c
5-  Instalar driver


5a primera vez ./start.sh
5b el  resto  insmod rfbb.ko

Si compilamos nuevamente el driver debemos remover el driver antes de instalarlo nuevamente 

rmmod rfbb.ko

Notas  si de casualidad se crea la carpeta como root (en lugar de usuario pi) se deberá cambiar los permisos

 chown -R pi:pi blinkdd/

de lo contrario NO podemos copiar desde la pc como usuario pi



