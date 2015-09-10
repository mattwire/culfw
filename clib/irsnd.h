 /*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irsnd.h
 *
 * Copyright (c) 2010-2014 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irsnd.h,v 1.20 2014/09/15 10:27:38 fm Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _IRSND_H_
#define _IRSND_H_

#include "board.h"

#include "irmpsystem.h"
#ifndef IRSND_USE_AS_LIB
#  include "irsndconfig.h"
#endif

#if IRSND_SUPPORT_SIEMENS_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, SIEMENS protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_SIEMENS_PROTOCOL
#  define IRSND_SUPPORT_SIEMENS_PROTOCOL        0
#endif

#if IRSND_SUPPORT_A1TVBOX_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, A1TVBOX protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_A1TVBOX_PROTOCOL
#  define IRSND_SUPPORT_A1TVBOX_PROTOCOL        0
#endif

#if IRSND_SUPPORT_RECS80_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80 protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_RECS80_PROTOCOL
#  define IRSND_SUPPORT_RECS80_PROTOCOL         0
#endif

#if IRSND_SUPPORT_RECS80EXT_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80EXT protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_RECS80EXT_PROTOCOL
#  define IRSND_SUPPORT_RECS80EXT_PROTOCOL      0
#endif

#if IRSND_SUPPORT_LEGO_PROTOCOL == 1 && F_INTERRUPTS < 20000
#  warning F_INTERRUPTS too low, LEGO protocol disabled (should be at least 20000)
#  undef IRSND_SUPPORT_LEGO_PROTOCOL
#  define IRSND_SUPPORT_LEGO_PROTOCOL           0
#endif

#include "irmpprotocols.h"

#define IRSND_NO_REPETITIONS                     0      // no repetitions
#define IRSND_MAX_REPETITIONS                   14      // max # of repetitions
#define IRSND_ENDLESS_REPETITION                15      // endless repetions
#define IRSND_REPETITION_MASK                   0x0F    // lower nibble of flags

extern void                                     irsnd_init (void);
extern uint8_t                                  irsnd_is_busy (void);
extern uint8_t                                  irsnd_send_data (IRMP_DATA *, uint8_t);
extern void                                     irsnd_stop (void);
extern uint8_t                                  irsnd_ISR (void);

#if IRSND_USE_CALLBACK == 1
extern void                                     irsnd_set_callback_ptr (void (*cb)(uint8_t));
#endif // IRSND_USE_CALLBACK == 1

#endif /* _IRSND_H_ */
