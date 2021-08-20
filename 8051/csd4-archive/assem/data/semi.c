#include <stdio.h>
#include <stdlib.h>

int main(void) {
   int Ch;
   while ((Ch = getchar()) != EOF) {
      if (Ch == ';') putchar(';');
      putchar(Ch);
   }
   return EXIT_SUCCESS;
}
