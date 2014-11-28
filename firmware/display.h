/*  
 * blinken64 / display.h
 * 
 * Defines how the 8x8 pixel LED display is connected to the attiny2313.
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

#ifndef __DISPLAY_H__
# define __DISPLAY_H__
#endif



# define PORT_ROW1 PORTD
# define ROW1      (1 << 2)
# define PORT_ROW2 PORTD
# define ROW2      (1 << 1)
# define PORT_ROW3 PORTD
# define ROW3      (1 << 4)
# define PORT_ROW4 PORTD
# define ROW4      (1 << 0)
# define PORT_ROW5 PORTB
# define ROW5      (1 << 3)
# define PORT_ROW6 PORTD
# define ROW6      (1 << 5)
# define PORT_ROW7 PORTB
# define ROW7      (1 << 5)
# define PORT_ROW8 PORTB
# define ROW8      (1 << 0)


# define PORT_COL1 PORTB
# define COL1      (1 << 4)
# define PORT_COL2 PORTA
# define COL2      (1 << 1)
# define PORT_COL3 PORTA
# define COL3      (1 << 0)
# define PORT_COL4 PORTB
# define COL4      (1 << 1)
# define PORT_COL5 PORTD
# define COL5      (1 << 3)
# define PORT_COL6 PORTB
# define COL6      (1 << 2)
# define PORT_COL7 PORTB
# define COL7      (1 << 6)
# define PORT_COL8 PORTB
# define COL8      (1 << 7)


# define A_OUTPUTS 0b00000011 // xxx xxx xxx xxx xxx xxx !R2 !R3
# define B_OUTPUTS 0b11111111 // !R8 !R7  C7 !R1  C5 !R6 !R4  C8  
# define D_OUTPUTS 0b00111111 // xxx xxx  C6  C3 !R5  C1  C2  C4
# define A_INIT    0b00000011
# define B_INIT    0b11010110 // R->HIGH, C->LOW für ausschalten
# define D_INIT    0b00001000


// define outputs 
# define initDisplay { DDRA |= A_OUTPUTS; DDRB |= B_OUTPUTS; DDRD |= D_OUTPUTS; }

// resets ports so no LED is lighting up 
// kill 0's, then set 1's
# define shutdownDisplay { PORTA &= ~A_OUTPUTS & ~A_INIT; PORTA |= A_OUTPUTS & A_INIT; \
                           PORTB = B_INIT; \
                           PORTD &= ~D_OUTPUTS & ~D_INIT; PORTD |= D_OUTPUTS & D_INIT;  }
                           

