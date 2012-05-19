
/*
 some functions for converting strings to and from hex.
 
 'plain' strings are binary safe, and need not be null-terminated.
 'hex'   strings are uppercase hex (68656C6C6F0D0A) and null-terminated.
 
 both encoding and decoding can be performed in-place.
 we do no checking for buffer overflows here.
*/

// encode a string as HEX.
void hexEncode(char* plain, char* hex, int len){
  for(int i=len-1;i>=0;i--) // loop backwards, so we can do it in-place.
    charToHex(plain[i],&hex[2*i]);
  hex[len*2]=0; // null terminate
}

// sets two bytes of hx to the hex value of c
void charToHex(char c, char* hx){
  char n1=(c>>4)&0xF;     // high nibble
  char n2=c&0xF;          // low nibble
  hx[0]=n1>9?n1+55:n1+48;
  hx[1]=n2>9?n2+55:n2+48;
}

// decodes a string of hex into plain
int hexDecode(char* hex, char* plain){
  int len=strlen(hex)/2; // if it's not even, ignore the extra nibble.
  for(int i=0;i<len;i++)
    plain[i]=hexToChar(hex[2*i],hex[2*i+1]);
  plain[len]=0; // null terminate
  return len;
}

// converts two ascii hex chars to one char
char hexToChar(char h1, char h2){
  return (h1>64?h1-55:h1-48)<<4 + (h2>64?h2-55:h2-48); // no error checking :)
}

