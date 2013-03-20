#include <string.h>

// strncasecmp does not exist on Windows. strnicmp does not exist on Linux.
int strncasecmp(char *pTarget, char *pSource, int count)
{
  return _strnicmp(pTarget, pSource, count);
}
