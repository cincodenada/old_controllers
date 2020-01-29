//    This file is part of Micro 64's Firmware.

//    Micro 64 is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Micro 64 is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with Micro 64.  If not, see <http://www.gnu.org/licenses/>.

// Simple program with the primary purpose of working out a precomputed CRC table.
// To check that the table is correct the program can test using a repeated message.
// A file called crc.dat needs to be placed at whatever directory fopen is set to use

#include <stdio.h>
#include <stdint.h>

#define ADDRESS

#ifdef ADDRESS
#define BITS 5
#define POLY 0x15
#else
#define BITS 8
#define POLY 0x85
#endif


int main()
{
	uint8_t repeated,
			table[256];
	FILE *input;
	int j;
	int stream;

	input = fopen("crc.dat","w");

	for (int i = 0; i<256; i++)
	{
		stream = i << (8-BITS)
		for (j = 0; j<8; j++){
			if (stream & 0x80){
				stream = stream << 1;
				stream = stream ^ (POLY << (8-BITS));
			}
			else
				stream = stream << 1;
		}
		for (j = 0; j < 8;j++){
			if (control & (0x80 >> j)){
				addinxor = addinxor ^ (POLY<<((BITS-1)-j));
			}
		}
		table[i] = addinxor;
		fprintf(input, "    0x%2.2X        ;0x%2.2X\n", addinxor, i);		
	}

	fclose(input);

	while(1){
		printf("Enter a value for a repeated crc to be calculated, 0 to quit:");
		scanf("%X", &repeated);

		if (repeated == 0)
			break;
		stream = repeated;
		for (int i = 0; i<31; i++){
			stream = repeated ^ table[stream];
		}
		stream = 0 ^ table[stream];
		printf("%X\n",stream);
	}
	return 0;
}
