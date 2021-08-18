// AS-Portierung
// Bereitstellung von fuer AS benoetigten Handle-Funktionen
typedef enum { NoRedir, RedirToDevice, RedirToFile } TRedirected; /* Umleitung von Handles */

#define NumStdIn 0
#define NumStdOut 1
#define NumStdErr 2

extern TRedirected Redirected;

void RewriteStandard(FILE ** T, char *Path);
void stdhandl_init(void);
