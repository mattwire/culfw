/*
 * Copyright by D.Tostmann
 * - hash table eeprom implementation - 
 * License: GPL v2
 */

#include "board.h"
#include "arch.h"
#include "registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef XMEGA

#include <xeeprom.h>
#define REG_START 0x100

static uint8_t registry_crc(REG_CRC_t set) {
  uint16_t  pos = REG_START;
  uint16_t wpos = 0;
  RID_t     id;
  uint16_t len;
  uint8_t  byte;
  uint8_t  move = 0;
  uint16_t crc = 0;

  while (pos<(E2END-3)) {
    id  = EEpromReadByte((void*)pos++);
    len = EEpromReadWord((void*)pos); pos += 2;

    if (id == REG_CRC) {// EOT?
      // len equals crc
      switch (set) {
      case CRC_REORG:
	if (move) {
	  EEpromWriteByte((void*)(wpos++), id);
	  EEpromWriteWord((void*)(wpos), crc);
	}
	return 1;
      case CRC_WRITE:
	EEpromWriteWord((void*)(pos-2), crc);
	return 1;
      case CRC_VERIFY:
	return (crc == len) ? 1 : 0;
      default:
	return 0;
      }
    }

    if ((set == CRC_REORG) && (id == REG_DELETED)) {
      if (!move)
	wpos = pos-3; // our start: move data here
      pos += len;     // point to next record
      move = 1;
      continue;
    }

    crc += (id + len);

    if (move) {
      EEpromWriteByte((void*)(wpos++), id);
      EEpromWriteWord((void*)(wpos), len); wpos += 2;
    }

    // sanity check ...
    if ((len+pos+3)>E2END)
      return 0;

    while (len--) {
      byte = EEpromReadByte((void*)pos++);
      crc += byte;
      if (move)
	EEpromWriteByte((void*)(wpos++), byte);
    }
    
  }

  return 0;
}

void registry_init(void) {
  uint16_t pos = REG_START;

  while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
  NVM.CTRLB |= NVM_EEMAPEN_bm;

  // check data integrity
  if (registry_crc(CRC_VERIFY) == 0) {
    EEpromWriteByte((void*)(pos++), REG_CRC);
    EEpromWriteWord((void*)(pos), 0);
  }
  else registry_crc(CRC_REORG);
}


REG_STATUS_t registry_get(RID_t rid, void *data) {
  uint16_t pos = REG_START;
  RID_t     id = REG_CRC;
  uint16_t len;

  while (pos<E2END) {
    id = EEpromReadByte((void*)pos++);
    
    if (id == REG_CRC) // EOT?
      return REG_STATUS_NOT_FOUND;

    len = EEpromReadWord((void*)pos); pos += 2;

    // sanity check ...
    if (len+pos>E2END)
      return REG_STATUS_ERROR;

    if (id == rid) {
      if (data)
	eeprom_read_block( data, (void*)pos, len); 
      return REG_STATUS_OK;
    }

    pos += len;
  }
  
  return REG_STATUS_ERROR;
}

void registry_set(RID_t rid, void *data, uint16_t len) {
  uint16_t pos = REG_START;
  RID_t     id;
  uint16_t   l = 0;

  while (pos<E2END) {
    id  = EEpromReadByte((void*)pos++);
    l   = EEpromReadWord((void*)pos); pos += 2;
    
    if (id == REG_CRC) { // EOT?
      // add at end
      EEpromWriteByte((void*)(pos-3), rid);
      EEpromWriteWord((void*)(pos-2), len);
      eeprom_write_block( data, (void*)pos, len); 
      pos += len;
      
      // add CRC
      EEpromWriteByte((void*)(pos++), REG_CRC);
      registry_crc(CRC_WRITE);

      return;
    }

    if (id == rid) {
      // we have rid already - check size if inline replace is possible
      if (len == l) {
	eeprom_write_block( data, (void*)pos, len); 
	registry_crc(CRC_WRITE);
	return;
      }
      // mark deleted and continue to end...
      EEpromWriteByte((void*)(pos-3), REG_DELETED);
    }

    pos += l;
  }
  
}

#else
void registry_init(void) {};
REG_STATUS_t registry_get(RID_t rid, void *data) { return REG_STATUS_NOT_FOUND };
void registry_set(RID_t rid, void *data, uint16_t len) {};
#endif
