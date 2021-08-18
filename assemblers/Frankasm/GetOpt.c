#if !defined NOGETOPT
#   include <getopt.h>
int GetOpt(int AC, char **AV, char *OptStr) { return getopt(AC, AV, OptStr); }
#else
#   include <stdio.h>
#   include <stdbool.h>
// Frankenstein Cross-Assemblers, version 2.0.
// Original authors: Keith Bostic, Rich $alz.
// Adapted by Mark Zenier.
// This is some ancient code I found on a version 7 system when I was running the original port.
// Asking for help from the original authors is not advised.
// Especially after the hack job I did on it.
// Mark Zenier.

// This is a public domain version of getopt(3).
// (Formerly ― bugs, fixes went to Keith Bostic, but he's ... preoccupied now.)
// Added NO_STDIO, opterr handling, Rich $alz (mirror!rs).

// Framework Cross-Assembler
// ∙	use strchr,
// ∙	remove NO_STDIO code.
// Mark Zenier, Specialized Systems Consultants, Inc.

// Global variables.
static char EMSG[] = "";
// The (undocumented) error-suppressor; which tells us whether or not to print errors.
static bool opterr = true;
// The index into argv.
int optind = 1;
// The option character being verified.
static int optopt;
// The argument (if any) associated with the option character.
char *optarg;

int GetOpt(int AC, char **AV, char *OptStr) {
   static char *place = EMSG; // Option letter processing.
   if (*place == '\0') { // Update the scanning pointer.
      if (optind >= AC || *(place = AV[optind]) != '-' || *++place == '\0') return EOF;
      if (*place == '-') { // Found "--".
         optind++;
         return EOF;
      }
   }
// Is the option letter okay?
   char *oli; // The option letter list index.
   if ((optopt = *place++) == ':' || (oli = strchr(OptStr, optopt)) == NULL) {
      if (*place == '\0') optind++;
      if (opterr) fprintf(stderr, "%s: illegal option -- %c\n", *AV, optopt);
      return '?';
   }
   if (*++oli != ':') { // We don't need an argument.
      optarg = NULL;
      if (*place == '\0') optind++;
   } else { // We do need an argument.
      if (place != '\0') optarg = place; // No white space.
      else if (AC > ++optind) optarg = AV[optind]; // Yes white space.
      else {
         place = EMSG;
         if (opterr) fprintf(stderr, "%s: option requires an argument -- %c\n", *AV, optopt);
         return '?';
      }
      place = EMSG, optind++;
   }
   return optopt; // Dump back the option letter.
}
#endif
