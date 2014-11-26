/*
 *  blinken64 / comm.h
 *
 *  Defines for the single pin asynchronous data transfer.
 *
 *  Copyright (C) 2011 Manuel Jerger <nom@nomnom.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __COMM_H__
# define __COMM_H__
#endif


// timing references for data transmission
#define COM_T_TIMEOUT   (65000)     // global timeout, terminates communication
#define COM_T_INIT      (6)         // max length of init pulse
#define COM_T_INIT_TX   (4)         // init pulse length used by transmit() 
#define COM_T_BIT       (16)        // threshold for bit decoding while receiving   
#define COM_T_LOW       (10)        // length of pulse for transm. a low bit
#define COM_T_HIGH      (22)        // length of pulse for transm. a high bit
#define COM_T_DEBOUNCE  (2)         // debounces keys


#if defined (_SLAVE_ONLY)      // use PD6 as input, do not use RES / output

 // PD6 as INPUT
 #define COM_IN_PORT    PORTD
 #define COM_IN_PIN     PIND
 #define COM_IN_DDR     DDRD
 #define COM_IN_BIT     (1 << PD6)
                    
 #define COM_OUT_H      {}      // NOT IMPLEMENTED
 #define COM_OUT_L      {}      // NOT IMPLEMENTED
 #define COM_WRITE      {}      // NOT IMPLEMENTED
 #define COM_READ       (COM_IN_PIN & COM_IN_BIT)
 
 #define initComm { COM_IN_DDR &= ~COM_IN_BIT; COM_IN_PORT |= COM_IN_BIT; }

 
#elif defined (_MASTER_ONLY_)   // use PD6 as output, do not use RES / input

 // PD6 as OUTPUT
 #define COM_OUT_PORT   PORTD
 #define COM_OUT_DDR    DDRD
 #define COM_OUT_BIT    (1 << PD6)

 #define COM_OUT_H      {COM_OUT_PORT |=  COM_OUT_BIT;}     // write high
 #define COM_OUT_L      {COM_OUT_PORT &= ~COM_OUT_BIT;}     // write low
 #define COM_WRITE      {COM_OUT_PORT ^= COM_OUT_BIT;}      // toggle pin
 #define COM_READ       1       // NOT IMPLEMENTED
 
 #define initComm { COM_OUT_DDR |= COM_OUT_BIT; COM_OUT_L; \
                    TIMSK |= (1 << TOIE1);}


#else                           // use RES as output, PD6 as input
 
 // RESET as OUTPUT
 #define COM_OUT_PORT    PORTA
 #define COM_OUT_DDR     DDRA
 #define COM_OUT_BIT     (1 << PA2)
 // PD6 as INPUT
 #define COM_IN_PORT    PORTD
 #define COM_IN_PIN     PIND
 #define COM_IN_DDR     DDRD
 #define COM_IN_BIT     (1 << PD6)
 
 #define COM_OUT_H      {COM_OUT_PORT |=  COM_OUT_BIT;}     // write high
 #define COM_OUT_L      {COM_OUT_PORT &= ~COM_OUT_BIT;}     // write low
 #define COM_WRITE      {COM_OUT_PORT ^= COM_OUT_BIT;}      // toggle pin
 #define COM_READ       (COM_IN_PIN & COM_IN_BIT)
 
 #define initComm { COM_IN_DDR &= ~COM_IN_BIT; COM_IN_PORT |= COM_IN_BIT; \
                    COM_OUT_DDR |= COM_OUT_BIT; COM_OUT_L; } 
 
#endif


