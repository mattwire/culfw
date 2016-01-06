

#Hinweise für CUNX

Ein CUNX spricht man sinnvollerweise am Anfang per USB durch /dev/ttyACM0 an.

##Netzwerk

*En* - Auslesen der IP Konfiguration

*Wim* - Setzten der MAC Adresse (wird automatisch gesetzt oder: Wima45055112233)
*Wid* - DHCP enabled flag (Wid01 - DHCP an // Wid00 DHCP aus)
*Wia* - Setzten der IPV4 Adresse (Wia192.168.2.168)
*Wig* - Setzten des IPV4 Gateways (Wig192.168.2.1)
*Win* - Setzten der IPV4 Netzmaske (Win255.255.255.0)

##Zugriff auf optionales Pigator modul:

- USB über /dev/ttyACM1

oder

- Netzwerk TCP Port 2324 der CUNX-IP-Adresse

Zudem kann man über CUNX (ttyACM0) das Modul steuern, flashen etc:

*pi* - Pigator Info

*pr* - Pigator Reset

*pb* - Pigator Bootload (so unterstützt)

