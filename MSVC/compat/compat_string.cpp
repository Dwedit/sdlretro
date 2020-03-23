#if _MSC_VER
#include <ctype.h>

//Code from RetroArch (MIT license)

int retro_strcasecmp__(const char *a, const char *b)
{
   while (*a && *b)
   {
      int a_ = tolower(*a);
      int b_ = tolower(*b);

      if (a_ != b_)
         return a_ - b_;

      a++;
      b++;
   }

   return tolower(*a) - tolower(*b);
}

#endif
