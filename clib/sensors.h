#ifndef _SENSORS_H
#define _SENSORS_H

void sensors_init(void);
void sensors_task(void);
void sensors_func(char *in);

// TSL45315
//#define TSL_ADDR         (0x39<<1)
#define TSL_ADDR         (0x29<<1)
#define TSL_REG_CONTROL  0x80
#define TSL_REG_CONFIG   0x81
#define TSL_REG_DATALOW  0x84
#define TSL_REG_DATAHIGH 0x85
#define TSL_REG_ID       0x8A

#endif


/*

This is a K7000 brightness sensor:

00000001 	T1T2T3T41 	A1A2A301 	L11L12L13L141 	L21L22L23L241 	L31L32L33L341 	F11F12F13F141 	Q1Q2Q3Q41 	S1S2S3S41
Präambel 	___5___ 	_0..7_0 	____1er____ 	____10er___ 	___100er___ 	___Faktor__ 	_Check_ 	_Summe_

L1..L3: 3 * 4Bit Helligkeit [Lux]??? (BCD)
F1..F3: Faktor ( 0 = *1, 1 = *10, 2 = *100, 3 = *1000)

    if($typbyte == 5 && int(@a) > 5) {           # brightness
      my $fakt = 1;
      my $rawfakt = ($a[5])+0;
      if($rawfakt == 1) { $fakt =   10; }
      if($rawfakt == 2) { $fakt =  100; }
      if($rawfakt == 3) { $fakt = 1000; }
     
      my $br = (hex($a[5].$a[4].$a[3])*$fakt)  + $hash->{corr1};
      $val = "B: $br";
      $devtype = "Brightness";
      $family = "WS7000";
      $NotifyType="B";
      $NotifyBrightness=$br;
    }

*/
