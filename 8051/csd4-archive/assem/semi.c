#include <stdio.h>
main() {
   int Ch;
   while ((Ch = getchar()) != EOF) {
      if (Ch == ';') putchar(';');
      putchar(Ch);
   }
}
