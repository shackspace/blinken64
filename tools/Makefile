#
# blinken64 tools / Makefile
#
#  builds the blinken64 tools (font and text converter)
#
#  Copyright (C) 2011 Manuel Jerger <nom@nomnom.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


all: fontconv textconv
	
fontconv:
	gcc fontconv.c -o fontconv

textconv:
	gcc textconv.c -o textconv


clean:
	rm -f textconv fontconv
