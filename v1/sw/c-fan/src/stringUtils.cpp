#include <string.h>

char *SubstChar(char *buf, char searchChar, char newChar)
{
  int l = strlen(buf);
  for(int i = 0; i < l; ++i)
    if (buf[i] == searchChar) buf[i] = newChar;
  return buf;
}
