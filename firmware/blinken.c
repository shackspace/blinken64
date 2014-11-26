/*
 *  blinken64 / blinken.c
 *
 *  8x8 pixel LED display for text scrolling
 *   - contains a complete ASCII (7 Bit) font with variable width chars
 *   - controllchars (0-31) for speed, wait, spacing, invert and pictures
 *   - 128 char message inkl 8 custom pictures stored in EEPROM
 *   - data input and output via one pin each
 *   - one pushbutton on the input pin
 *   - automatic data-in detection
 *   - reprogramming mode (eeprom rewrite via data in)
 *
 *
 *  ATTENTION: compile with avr-gcc (GCC) 4.3.4  to get the best results
 *
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

#include <inttypes.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

//----------------------------------------------------------------------

#include "font.h"
#include "display.h"
#include "comm.h"

//----------------------------------------------------------------------
// PROTOTYPES

void delay(uint16_t delay);
//void delay_ms(uint16_t delay);

//----------------------------------------------------------------------
// some static data

#define EEPROM_BEGIN    0x02
#define EEPROM_END      0x80

#define MSG_SEP         0x00
#define END_OF_MEMORY   0x01

#define SPEED1          0x02
#define SPEED2          0x03
#define SPEED3          0x04
#define SPEED4          0x05
#define SPEED5          0x06
#define SPEED6          0x07
#define SPEED7          0x08
#define SPEED8          0x09

#define INVERT          0x0A
#define HALT            0x0B

#define WAIT1           0x0E
#define WAIT2           0x0F
#define WAIT3           0x10
#define WAIT4           0x11
#define WAIT5           0x12
#define WAIT6           0x13
#define WAIT7           0x14
#define WAIT8           0x15

#define PICTURE1        0x16
#define PICTURE2        0x17
#define PICTURE3        0x18
#define PICTURE4        0x19
#define PICTURE5        0x1A
#define PICTURE6        0x1B
#define PICTURE7        0x1C
#define PICTURE8        0x1D
#define SPACER1         0x1E
#define SPACER2         0x1F
#define SPACE           0x20

#define spacing  1              // between chars

const uint16_t waits[8] = { 50, 100, 250, 500, 1000, 1500, 2000, 5000}; // in ms, delay per wait symbol
const uint8_t delays[8] = { 0, 5, 20, 32, 64, 96, 128, 255};            // in ms, delay per column


//----------------------------------------------------------------------
// global state stuff

volatile uint8_t skipmessage = 0;        // if true, scroll to next msg separator fast

enum MODES {MASTER, SLAVE, PROG};
volatile uint8_t mode = MASTER;

volatile uint8_t buff[8];         // screen memory, 8 rows / 8 cols
volatile uint8_t row = 0;         // active row number
volatile uint8_t inverted = 0;    // if characters should be inverted
volatile uint8_t display_on = ~0; // display on/off

//----------------------------------------------------------------------
// counters / timers (clock dividers for many things)

// timer 1ms
#define CTR_DELAY_MS_MAX    (20)      // upper val for 1ms clock
volatile uint16_t ctr_delay = 0;      // counts up w/ 20kHz

// counter for delay_ms
volatile uint16_t ctr_delay_ms = 0;   // counts up in 1 ms ticks

// counter for fastdelay
volatile uint16_t ctr_fast_delay = 0; // counts up w/ 20kHz


// fast delay, 20kHz
void delay (uint16_t _t) { ctr_fast_delay = 0; while (ctr_fast_delay < _t) {} }
//#define delay (uint16_t _t) {ctr_fast_delay = 0; while (ctr_fast_delay < (_t)) {} }

// millisecond delay
//void delay_ms(uint16_t _t) { ctr_delay_ms = 0; while (ctr_delay_ms < _t) {} }


////////////////////////////////////////////////////////////////////////



volatile uint8_t lastRead;   // state of input pin at last ISR call
volatile uint8_t rxBuff=0;
volatile uint8_t txBuff=0;
volatile uint8_t rxDone=0;   // '1' signals a newly arrived byte
volatile uint16_t comctr;    // counter for communication timing

// rx bit counter
volatile int8_t bitpos = -1; // -1=rx idle, 0-7=pos, 8=done
volatile uint8_t bitmask = 0; // so we only have to shift once per iteration


/*
 * ISR TIMER1 Compare Match
 * runs with 4MHz / 200 = 20kHz  (50us) -> only 200 cycles due to disabled 8x prescaler (fuse)
 * - creates 1 ms clock
 * - does display scanning in 1 ms interval
 * - has full control over the COM_READ pin:
 *   - key debouncing and presses
 *   - communication detection and automatic initiation
 *   - receive data
 */
ISR(TIMER1_COMPA_vect) {


    ctr_fast_delay++;           // for delay() function


    // 1ms clocked counter
    if ( ++ctr_delay >= CTR_DELAY_MS_MAX) {
        ctr_delay = 0;
        ctr_delay_ms++;
    }


  #ifndef _MASTER_ONLY_

    // communication and button handling: the amazing input pin

    uint8_t read = COM_READ;

    // pinstate has changed, take action
    if (lastRead != read) {

        // pulse too short -> debounce extends timeout
        if (comctr <= COM_T_DEBOUNCE) {
            comctr = 0;
            bitpos = -1;
            rxBuff = 0;
        }


        // com channel was in idle state (indicated by bitpos=-1)
        if (bitpos == -1) {
            // HI-LO
            if (!read) {
                // try to start receiving, lets see whats coming in..
                bitpos = 0;
                bitmask = 1;

            }

        // rx next bit (or button input)
        } else if (bitpos < 8) {
            // rx 0
            if (comctr < COM_T_BIT) {
                rxBuff &= ~bitmask;
                bitpos++;
                bitmask <<= 1;

            // rx 1
            } else if (comctr < 2*COM_T_BIT){
                rxBuff |= bitmask;
                bitpos++;
                bitmask <<= 1;

            // too long, maybe the button?
            } else {
                if (bitpos < 7 && mode == MASTER) {
                    skipmessage = 1;
                    bitpos = -1;
                }
            }

            // byte completed
            if (bitpos == 8) {
                rxDone = 1;
            }
             // and the master becomes a slave...
             if (mode == MASTER && bitpos == 1) { mode = SLAVE; }

        } else {
            bitpos = -1;
        }


        // remember for next ISR call
        lastRead = read;
        comctr = 0;

    // pinstate has not changed, increment and check timeouts
    } else {

        // global timeout)
        if (++comctr >= COM_T_TIMEOUT) {
            comctr = 0;
            bitpos = -1;
            if (mode == SLAVE) { mode = MASTER; }
        }

    }
  #endif



    /*
     * row scanning (8 rows a 8 bit)
     * w/ 1000 Hz
     */
    if (!ctr_delay) {

        shutdownDisplay;
        if (display_on) {
            uint8_t val;//, b=1, i;
            row++;
            if (row > 7) { row = 0; }
            val = buff[row];
            if ( val & 0x01 ) { PORT_COL1 |= COL1; } 
            if ( val & 0x02 ) { PORT_COL2 |= COL2; } 
            if ( val & 0x04 ) { PORT_COL3 |= COL3; } 
            if ( val & 0x08 ) { PORT_COL4 |= COL4; } 
            if ( val & 0x10 ) { PORT_COL5 |= COL5; } 
            if ( val & 0x20 ) { PORT_COL6 |= COL6; } 
            if ( val & 0x40 ) { PORT_COL7 |= COL7; } 
            if ( val & 0x80 ) { PORT_COL8 |= COL8; } 
            switch (row) {                           
                case 0 : PORT_ROW1 &= ~ROW1; break;  
                case 1 : PORT_ROW2 &= ~ROW2; break;  
                case 2 : PORT_ROW3 &= ~ROW3; break;  
                case 3 : PORT_ROW4 &= ~ROW4; break;  
                case 4 : PORT_ROW5 &= ~ROW5; break;  
                case 5 : PORT_ROW6 &= ~ROW6; break;  
                case 6 : PORT_ROW7 &= ~ROW7; break;  
                case 7 : PORT_ROW8 &= ~ROW8; break;  
            }

        }
    }


}




////////////////////////////////////////////////////////////////////////
// MAIN
void main(void) __attribute__ ((noreturn));  // main does not return -> 14 byte less!
void main(void) {

    // HARDWARE INIT
    TCCR1B |= (1 << WGM12);     // timer 1 mode 9: CTC
    TCCR1B |= (1 << CS10);      // timer 1 no prescaler -> 4 MHz
    OCR1A  = 199;               // timer 1 20kHz
    TIMSK |= (1 << OCIE1A);     // enable timer 1 compare match interrupt
    ACSR |= (1 << ACD);         // disable analog comparator

    initComm;
    initDisplay;

    sei();                      // enable interrupts

    // this is a display, not a programmer -> signal this to the next display in chain
    COM_OUT_H;

    // wait: maybe another display is connected to the input and has started up at the time
    delay(15000);


    #ifndef _MASTER_ONLY_
    // read low -> programmer is attached, go into PROG mode
    if (!COM_READ) {
        mode = PROG;
        buff[3] = 0b00011000;
        buff[4] = 0b00011000;
        lastRead = 0;
    }
    #endif




////////////////////////////////////////////////////////////////////////

    uint8_t i,j;

    uint8_t text_begin = EEPROM_BEGIN;     // position of current displayed msg in eeprom (first bit)
    uint8_t pos = EEPROM_BEGIN;            // current position in eeprom
    uint8_t speed = 4;                     // framewait default
    

    // main loop : do the loop
    for (;;) {


        uint8_t width   = 0;                        // width of current character
        uint8_t currchar;                           // location of current character in font[]
        uint8_t chr[8];                             // char buffer, 8 columns a 8 bit, char aligned at the lowest index
        uint8_t rows2do = 0;                        // num rows to scroll the display

        // SLAVE waits for a new column to come in
        if (mode == SLAVE) {
            while (!rxDone && mode == SLAVE) { }
            chr[0] = rxBuff;
            rxDone = 0;
            rows2do = 1;
            width = 1;


        // PROG reprogramming eeprom in local loop
        // wait for data until eeprom is full, toggle display to signal activity
        } else if (mode == PROG) {

            uint8_t p = 0;

            // first two bytes must be 0xAA (init sequence), otherwise we 
            // might accidentally tap into the outputstream of another
            // blinken64. then it programs itself with nonsense data -.-
            while (mode == PROG) {
                
                while (!rxDone) { }     // wait for byte

                if ( p < EEPROM_BEGIN) {
                    if (rxBuff == 0xAA) { p++; }
                } else if (p < EEPROM_END) {       // no overflow
                    eeprom_write_byte(p,rxBuff);
                    display_on = ~display_on;      // activity toggle
                    p++;
                
                }
                
                rxDone = 0;
            }


        // MASTER
        } else {

            // get next
            currchar = eeprom_read_byte((uint8_t*)pos);
            pos++;


            ////////////////////////////////////////////////////////////////
            // evaluate control chars

            // Message separator + End Of Memory
            if (currchar <= END_OF_MEMORY) {

                // we found the next message...
                if (skipmessage) {
                    skipmessage = 0;
                    if (currchar == END_OF_MEMORY) { pos = EEPROM_BEGIN; } // End Of Memory -> wrap around
                    text_begin = pos;   // select next message

                // loop current message
                } else {
                    pos = text_begin;
                }

            // SPEEDs 8x
            } else if (currchar <= SPEED8) {
                speed = currchar - SPEED1;

            // INVERT 1x
            } else if (currchar == INVERT) {
                inverted = ~inverted;

            // HALT 1x
            } else if (currchar == HALT) {
                if (!skipmessage) { pos--; }

            // WAIT 8x
            } else if (currchar <= WAIT8) {
                if (!skipmessage) {  
                    ctr_delay_ms = 0;
                    while ( (ctr_delay_ms < waits[currchar - WAIT1] )) {}
                }

            // EEPROM PICS 8x
            } else if (currchar <= PICTURE8) {
                width=8; rows2do=8;
                for (i=0; i<8;i++) {
                    chr[i]=eeprom_read_byte(128- (currchar-PICTURE1)*8-8+i);
                }

            // SPACERS 3x (inkl SPACEBAR )
            } else if (currchar <= SPACE) {
                rows2do = currchar - 29;

            ////////////////////////////////////////////////////////////
            // load displayable char from font[]
            // not the straight-forward way - we need so save space!
            } else if (currchar <= lastchar) {

                // seek to mem position by summing up the widths (which are stored in half-byte format)
                uint16_t charpos = 0;
                for (i=0; i < currchar-firstchar+1; i++) {
                    width = pgm_read_byte(&widths[(i)>>1]);

                    if (!(i & 0x01)) { width >>= 4; }  // even : highest 4 bits
                    
                    width &= 0b1111;
                    
                    if (i < currchar-firstchar) {
                        charpos += width;
                    }
                }

                // copy char
                for (i=0; i<width; i++) { chr[i] = pgm_read_byte(&font[charpos]+i); }
                rows2do = spacing + width;

            } // end big if-elseif block



        } // end if (master)



        ///////////////////////////////////////////////////////////////
        // DO SCROLLING (for MASTER and SLAVE), if not skipmessage
        for (i=0; (i<rows2do) && !skipmessage; i++) {

          #ifndef _SLAVE_ONLY_

            // TRANSMISSION - transmit last column (if speed > 0)
            if (speed) {

                // HACK : we have to access (write to) ctr_fast_delay here, otherwise the very first call to delay() does sometimes return instantly, seemingly depending on the character loaded some cycles before.
                //       a delay(1) also works.. very weird
                ctr_fast_delay = 0;

                //COM_WRITE;
                COM_OUT_L;
                bitmask=1;
                while (bitmask) {
                    if ( (buff[7]) & bitmask) {
                         //ctr_fast_delay = 0; while (ctr_fast_delay < COM_T_HIGH) {}   // this ALSO does not work
                         delay(COM_T_HIGH);
                    } else {
                         // ctr_fast_delay = 0; while (ctr_fast_delay < COM_T_LOW) {}   // this ALSO does not work
                         delay(COM_T_LOW);
                    }
                    COM_WRITE;
                    bitmask <<= 1;
                }
                delay(COM_T_BIT/2);
                COM_OUT_H;
            }

          #endif


            // shift array -> move display contents to the left
            for (j=7; j>0; j--){ buff[j] = buff[j-1]; }

            // set right column to the new  value / inverted value, or to 0/1 for empty cols
            if (i < width) { buff[0]=chr[i]^inverted; } else { buff[0]=inverted; }

            // FRAMEWAIT only in master mode, skip immediately if master becomes slave
            ctr_delay_ms = 0;
            while ( (ctr_delay_ms < delays[speed] ) && mode==MASTER) {}


        }


    }   // end for

} // end main()
