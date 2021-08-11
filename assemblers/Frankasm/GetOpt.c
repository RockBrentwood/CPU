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
// (Especially after the hack job I did on it.  Mark Zenier.)

// This is a public domain version of getopt(3).
// Bugs, fixes to:
//		Keith Bostic
//			ARPA: keith@seismo
//			UUCP: seismo!keith
// Added NO_STDIO, opterr handling, Rich $alz (mirror!rs).

// Framework Cross Assembler
//	use strchr
//	remove NO_STDIO code
//	Mark Zenier 	Specialized Systems Consultants, Inc.

// Global variables.
static char EMSG[] = "";
// The (undocumented) error-suppressor; which tells us whether or not to print errors.
static bool opterr = true;
// The index into argv.
int optind = 1;
// The option character being verified.
static int optopt;
// The argument associated with the option.
char *optarg;

int GetOpt(int AC, char **AV, char *OptStr) {
   static char *place = EMSG; /* option letter processing */
   char *oli; /* option letter list index */

   if (!*place) { /* update scanning pointer */
      if (optind >= AC || *(place = AV[optind]) != '-' || !*++place)
         return (EOF);
      if (*place == '-') { /* found "--" */
         optind++;
         return (EOF);
      }
   }
/* option letter okay? */
   if ((optopt = *place++) == ':' || (oli = strchr(OptStr, optopt)) == NULL) {
      if (!*place)
         optind++;
      if (opterr) fprintf(stderr, "%s: illegal option -- %c\n", *AV, optopt);
      goto Bad;
   }
   if (*++oli != ':') { /* don't need argument */
      optarg = NULL;
      if (!*place)
         optind++;
   } else { /* need an argument */
   if (*place)
      optarg = place; /* no white space */
   else if (AC <= ++optind) {
      place = EMSG;
      if (opterr) fprintf(stderr, "%s: option requires an argument -- %c\n", *AV, optopt);
      goto Bad;
   } else
      optarg = AV[optind]; /* white space */
   place = EMSG;
   optind++;
   }
   return (optopt); /* dump back option letter */
Bad:
   return ('?');
}
#endif
