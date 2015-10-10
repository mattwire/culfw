

#Hinweise für CUNX

Ein CUNX spricht man sinnvollerweise am Anfang per USB durch /dev/ttyACM0 an.

##Netzwerk

Derzeit ist nur DHCP möglich. Statische IP wird noch nicht unterstützt.

*En* - Auslesen der IP Adresse

##Zugriff auf optionales Pigator modul:

- USB über /dev/ttyACM1

oder

- Netzwerk TCP Port 2324 der CUNX-IP-Adresse

Zudem kann man über CUNX (ttyACM0) das Modul steuern, flashen etc:

*pi* - Pigator Info

*pr* - Pigator Reset

*pb* - Pigator Bootload (so unterstützt)

