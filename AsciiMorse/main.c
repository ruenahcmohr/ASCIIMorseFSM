/***********************************************************************************

math rom

INPUTS
 - argument A, 7 bits ascii value
 - argument B, 5 bits machine feedback


OUTPUTS
 - morse bit (0)
 - done  bit (1)
 
FEEDBACK
 - feedback (5 bits)



Address bits      8 bit rom size

       -- no parallel roms available --
     8                  2 k
     9                  4 k
     10                 8 k
     
       -- eeproms available from here --
     11                 16 k  (28C16)
     12                 32 k  (28C32)
     
       -- eprom practical sizes from here --
     13                 64 k  (2764)
     14                 128 k (27128)
     15                 256 k 
     16                 512 k
     17                 1 M  (27010)
     18                 2 M
     19                 4 M
     20                 8 M

       -- flash from here up --



**************************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include "ROMLib.h"
#include <math.h>
#include <ctype.h>
#include "Morse.h"
#include <string.h>

// the number of address lines you need
#define InputBits 12

// the output data size, 8 or 16
#define OutputBits 8


// default output value
#define DFOutput  0x00



// geez, tuck this one away.  Please dont use this in real fft code,
//   YOU KNOW how many bits your working with, and you can use a 
//   specific case generator for it.
uint8_t uniReverse(uint8_t i, uint8_t bits) {

  uint8_t r, m, b;
  r = 0;             // result
  m = 1 << (bits-1); // mask will travel right
  b = 1;             // bit will travel left
  
  while(m) {
    if (i&b) r |=m;
    b <<= 1;
    m >>= 1;  
  }
  
  return r;  

}


char getMorse(char c, uint8_t slot) {
  int i;
  int p;
  
  c = toupper(c);
  
  for (i = 0; chars[i] ; i++)  if (chars[i] == c) break;
  
  if (i == 37) return '\0';
  
  p = strlen(morse[i]);
  
  if (slot > p) return '\0';
  
  return morse[i][slot];

  
}



int main(void) {

  uint16_t AsciiI, StateI;           // values are 16 bits      
  uint32_t MorseO, StateO, DoneO;           // bit fields are 32 bits

  char m;
  uint16_t maxState = 0;

  uint32_t out;
  
  setup();  // open output file.
  
  
  // loop thru each address
  for( A=0; A<(1<<InputBits); A++) { // A is a bitfield (a mash-up of the vars we want)
  
     // assign default values for outputs     
     out       = DFOutput;
     
     // build input values
     spliceValueFromField( &AsciiI,        A,  7,   0, 1, 2, 3, 4, 5, 6);      // Ascii input, 7 bits (its just 7 bit)
     spliceValueFromField( &StateI,        A,  6,   7, 8, 9, 10, 11 );  // FSM state, 6 bits , could be 5
     
    

     // do task
     
     m = getMorse(AsciiI, StateI-1);
     if(0) {
     } else if (StateI == 0) { // silence state.
       MorseO = 0;
       StateO = 1;
       DoneO  = 1;
     } else if (m == 0) { // invalid character or invalid slot for current character.
       MorseO = 0;
       StateO = 0;
       DoneO  = 0;
     } else {
       MorseO = (m == '0')?0:1;
       StateO = StateI + 1;
       DoneO  = 1;
       maxState = (StateO > maxState)?StateO:maxState;
     }            
       
   
     // reconstitute the output
     spliceFieldFromValue( &out, MorseO,     1,  0);
     spliceFieldFromValue( &out, DoneO,      1,  1);
     spliceFieldFromValue( &out, StateO,     5,  2, 3, 4, 5, 6);
     
     
     
     // submit to file
     write(fd, &out, OutputBits>>3);  // in bytes
     //printf("%d, ", out);
  }
  
  cleanup(); // close file
//  printf("}\n");
  printf("-> The highest counter state is %d\n", maxState);
  return 0;
}




