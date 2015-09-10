/*************************************************************************
 *  Dateiname    : xeeprom.h
 *
 *  Beschreibung : Memory Mapped EEprom für xMegas
 *
 *  Autor        : Joachim Rath
 *  Revision     : initial 04/2010
 *
 *************************************************************************
 *                 (C) Copyright 2010 Joachim Rath
 *                Joachim[dot]Rath[at]JoRath[dot]eu
 *************************************************************************
 * Dieses Programm ist freie Software. Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation
 * veröffentlicht, weitergeben und/oder modifizieren, entweder gemäß
 * Version 2 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
 *
 * Die Veröffentlichung dieses Programms erfolgt in der Hoffnung,
 * daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE,
 * sogar ohne die implizite Garantie der MARKTREIFE oder der
 * VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der
 * GNU General Public License.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen
 * mit diesem Programm erhalten haben.
 * Falls nicht, schreiben Sie an die Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *************************************************************************/
#ifndef _XEEPROM_INCLUDED
#define _XEEPROM_INCLUDED
/*************************************************************************
 *                            Definitionen
 *************************************************************************/
#ifndef _AVR_EEPROM_H_
#define EEMEM __attribute__((section(".eeprom")))

#define eeprom_read_byte(Src)            EEpromReadByte(Src)
#define eeprom_write_byte(Dst,Val)       EEpromWriteByte(Dst,Val)
#define eeprom_read_word(Src)            EEpromReadWord(Src)
#define eeprom_write_word(Dst,Val)       EEpromWriteWord(Dst,Val)
#define eeprom_read_dword(Src)           EEpromReadDWord(Src)
#define eeprom_write_dword(Dst,Val)      EEpromWriteDWord(Dst,Val)
#define eeprom_read_float(Src)           EEpromReadFloat(Src)
#define eeprom_write_float(Dst,Val)      EEpromWriteFloat(Dst,Val)
#define eeprom_read_block(Dst,Src,Len)   EEpromReadBlock(Dst,Src,Len)
#define eeprom_write_block(Src,Dst,Len)  EEpromWriteBlock(Src,Dst,Len)

#define eeprom_update_byte(Dst,Val)      EEpromWriteByte(Dst,Val)
#define eeprom_update_word(Dst,Val)      EEpromWriteWord(Dst,Val)
#define eeprom_update_dword(Dst,Val)     EEpromWriteDWord(Dst,Val)
#define eeprom_update_float(Dst,Val)     EEpromWriteFloat(Dst,Val);
#define eeprom_update_block(Src,Dst,Len) EEpromWriteBlock(Src,Dst,Len)

#define eeprom_busy_wait() ((while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);)
#define eeprom_is_ready()  ((NVM.STATUS & NVM_NVMBUSY_bm) != NVM_NVMBUSY_bm))
#else
 #warning "gemeinsame Verwendung von <eeprom.h> und "xeeprom.h" ist nicht getestet"
#endif
/*************************************************************************
 *
 *  Name         : EEpromInit
 *
 *  Beschreibung : EEprom Memory Mapped einblenden
 *
 *  Parameter    : keine
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromInit(void)
{
 while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
 NVM.CTRLB |= NVM_EEMAPEN_bm;
}
/*************************************************************************
 *
 *  Name         : EEpromReadByte
 *
 *  Beschreibung : Ein Byte lesen
 *
 *  Parameter    : pointer Quelle
 *
 *  Rückgabe     : das Byte
 *
 *************************************************************************/
#pragma GCC diagnostic ignored "-Warray-bounds"
static inline uint8_t
EEpromReadByte(void *Src)
{
 return(*(uint8_t *)((uint16_t)Src + MAPPED_EEPROM_START));
}
/*************************************************************************
 *
 *  Name         : EEpromReadWord
 *
 *  Beschreibung : Ein Word lesen
 *
 *  Parameter    : pointer Quelle
 *
 *  Rückgabe     : das Word
 *
 *************************************************************************/
static inline uint16_t
EEpromReadWord(void *Src)
{
 return(*(uint16_t *)((uint16_t)Src + MAPPED_EEPROM_START));
}
/*************************************************************************
 *
 *  Name         : EEpromReadDWord
 *
 *  Beschreibung : Ein DWord lesen
 *
 *  Parameter    : pointer Quelle
 *
 *  Rückgabe     : das DWord
 *
 *************************************************************************/
static inline uint32_t
EEpromReadDWord(void *Src)
{
 return(*(uint32_t *)((uint16_t)Src + MAPPED_EEPROM_START));
}
/*************************************************************************
 *
 *  Name         : EEpromReadFloat
 *
 *  Beschreibung : Eine Float lesen
 *
 *  Parameter    : pointer Quelle
 *
 *  Rückgabe     : float
 *
 *************************************************************************/
static inline float
EEpromReadFloat(void *Src)
{
 return(*(float *)((uint16_t)Src + MAPPED_EEPROM_START));
}
#pragma GCC diagnostic warning "-Warray-bounds"
/*************************************************************************
 *
 *  Name         : EEpromReadBlock
 *
 *  Beschreibung : Block lesen
 *
 *  Parameter    : pointer Ziel,Quelle + Länge
 *
 *
 *  Rückgabe     : pointer Ziel
 *
 *************************************************************************/
static inline void *
EEpromReadBlock(void *Dst,void *Src,size_t Len)
{
 return(memcpy(Dst,(uint8_t *)(uint16_t)Src + MAPPED_EEPROM_START,Len));
}
/*************************************************************************
 *
 *  Name         : EEpromWritePage
 *
 *  Beschreibung : eine Page via NVM Controller zurückschreiben
 *
 *  Parameter    : Adresse im EEprom
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWritePage(uint16_t Addr)
{
 NVM.ADDR0 = Addr & 0xFF;
 NVM.ADDR1 = Addr >> 8 & 0x1F;          // max 8K
 NVM.ADDR2 = 0x00;

 NVM.CMD   = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;

 CPU_CCP   = CCP_IOREG_gc;
 NVM.CTRLA = NVM_CMDEX_bm;
                                        // warten bis fertig
 while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
}
/*************************************************************************
 *
 *  Name         : EEpromWriteByte
 *
 *  Beschreibung : ein Byte schreiben
 *
 *  Parameter    : pointer Ziel,Wert
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWriteByte(void *Dst, uint8_t Val)
{
 *(uint8_t *)((uint16_t)Dst + MAPPED_EEPROM_START) = Val;
 EEpromWritePage((uint16_t)Dst);
}
/*************************************************************************
 *
 *  Name         : EEpromWriteWord
 *
 *  Beschreibung : ein Word schreiben
 *
 *  Parameter    : pointer Ziel,Wert
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWriteWord(void *Dst, uint16_t Val)
{
 uint16_t Idx = (uint16_t)Dst;

 // Word Zugriff an Page grenze ?
 if(Idx % EEPROM_PAGE_SIZE == EEPROM_PAGE_SIZE-1)
  {                             // dann aufteilen
   *(uint8_t *)(Idx + MAPPED_EEPROM_START ) = Val&0xff;
   EEpromWritePage(Idx++);
   *(uint8_t *)(Idx + MAPPED_EEPROM_START ) = Val >> 8;
   EEpromWritePage(Idx);
  }
 else
  {
   *(uint16_t *)(Idx + MAPPED_EEPROM_START) = Val;
   EEpromWritePage(Idx);
  }
}
/*************************************************************************
 *
 *  Name         : EEpromWriteDWord
 *
 *  Beschreibung : ein DWord schreiben
 *
 *  Parameter    : pointer Ziel,Wert
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWriteDWord(void *Dst, uint32_t Val)
{
 uint16_t Idx = (uint16_t)Dst;

 // DWord Zugriff an Page grenze ?
 if(Idx % EEPROM_PAGE_SIZE >= EEPROM_PAGE_SIZE-3)
  {                             // dann aufteilen
   EEpromWriteWord((void*)Idx++,Val & 0xffff);
   EEpromWriteWord((void*)++Idx,Val  >> 16);
  }
 else
  {
   *(uint32_t *)(Idx + MAPPED_EEPROM_START) = Val;
   EEpromWritePage(Idx);
  }
}
/*************************************************************************
 *
 *  Name         : EEpromWriteFloat
 *
 *  Beschreibung : eine Float schreiben
 *
 *  Parameter    : pointer Ziel,Wert
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWriteFloat(void *Dst, float Val)
{
 union WieZumGeierCastetManEineFloat{
   uint32_t d;
   float    f;
 };

 union WieZumGeierCastetManEineFloat AufEinDWord;

 AufEinDWord.f = Val;

 EEpromWriteDWord(Dst,AufEinDWord.d);
}
/*************************************************************************
 *
 *  Name         : EEpromWriteBlock
 *
 *  Beschreibung : einen Block schreiben
 *
 *  Parameter    : pointer Ziel,Quelle + Länge
 *
 *  Rückgabe     : keine
 *
 *************************************************************************/
static inline void
EEpromWriteBlock(void *Src,void *Dst,size_t Len)
{
 uint16_t sIdx,dIdx;
 uint8_t  Rest;

 sIdx = (uint16_t)Src;
 dIdx = (uint16_t)Dst;
 Rest = 0;

 while(Len)
  {
   *(uint8_t *)((uint16_t)dIdx++ + MAPPED_EEPROM_START) = *(uint8_t*)sIdx++;
   Rest = 1;

   if(!(dIdx % EEPROM_PAGE_SIZE))       // Page Grenze ?
    {
     EEpromWritePage(dIdx-1);
     Rest = 0;
    }

   Len--;
  };

 if(Rest)
  EEpromWritePage(dIdx-1);
}
/*************************************************************************
 *
 *  Name         :
 *
 *  Beschreibung :
 *
 *  Parameter    :
 *
 *
 *  Rückgabe     :
 *
 *************************************************************************/
#endif
/*************************************************************************
 *                      E N D   O F   S O U R C E
 *************************************************************************/

