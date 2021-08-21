#include <stdio.h>
#include <stdlib.h>

int main(void) {
   for (int Ch; (Ch = getchar()) != EOF; ) {
      if (Ch == ';') putchar(';');
      putchar(Ch);
   }
   return EXIT_SUCCESS;
}
