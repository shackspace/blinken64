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



# define PORT_COL1 PORTA
# define PORT_COL2 PORTB
# define PORT_COL3 PORTB
# define PORT_COL4 PORTD
# define PORT_COL5 PORTB
# define PORT_COL6 PORTD
# define PORT_COL7 PORTD
# define PORT_COL8 PORTD
# define COL1      (1 << 0)
# define COL2      (1 << 5)
# define COL3      (1 << 4)
# define COL4      (1 << 4)
# define COL5      (1 << 2)
# define COL6      (1 << 3)
# define COL7      (1 << 1)
# define COL8      (1 << 0)
# define PORT_ROW1 PORTD
# define PORT_ROW2 PORTA
# define PORT_ROW3 PORTB
# define PORT_ROW4 PORTD
# define PORT_ROW5 PORTB
# define PORT_ROW6 PORTB
# define PORT_ROW7 PORTB
# define PORT_ROW8 PORTB
# define ROW1      (1 << 5)
# define ROW2      (1 << 1)
# define ROW3      (1 << 0)
# define ROW4      (1 << 2)
# define ROW5      (1 << 7)
# define ROW6      (1 << 1)
# define ROW7      (1 << 6)
# define ROW8      (1 << 3)
# define A_OUTPUTS 0b00000011 // XX XX XX XX XX XX R2 C1  
# define B_OUTPUTS 0b11111111 // R5 R7 C2 C3 R8 C5 R6 R3
# define D_OUTPUTS 0b00111111 // XX XX R1 C4 C6 R4 C7 C8
# define A_INIT    0b00000010
# define B_INIT    0b11001011 // R->HIGH, C->LOW
# define D_INIT    0b00100100


// define outputs 
# define initDisplay { DDRA |= A_OUTPUTS; DDRB |= B_OUTPUTS; DDRD |= D_OUTPUTS; }

// resets ports so no LED is lighting up 
// kill 0's, then set 1's
# define shutdownDisplay { PORTA &= ~A_OUTPUTS & ~A_INIT; PORTA |= A_OUTPUTS & A_INIT; \
                           PORTB = B_INIT; \
                           PORTD &= ~D_OUTPUTS & ~D_INIT; PORTD |= D_OUTPUTS & D_INIT;  }
                           

