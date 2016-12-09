

#Hinweise für Funkmodul im Tuxradio4

per USB durch /dev/ttyACM0

##Modulsteuerung:

*pi* - Pigator Info

*pr* - Pigator Reset

*pb* - Pigator Bootload (so unterstützt)

*ps* - speichern der Baudrate (wie über ttyACM1 gesetzt) bei seriellen/RS485 PIM 

##Zugriff auf optionales Pigator modul:

über /dev/ttyACM1

##Reset RF module

```
make reset
```

##Firmware update

```
make dfu
```
