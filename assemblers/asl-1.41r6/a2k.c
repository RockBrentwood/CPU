#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*** valid sym. character */

int readline(file, dest)
FILE *file;
char *dest;
{
   char *run = dest;
   int zeichen = 'a';

   while ((!feof(file)) && (zeichen != EOF) && (zeichen != '\n')) {
      zeichen = fgetc(file);
      if ((zeichen != EOF) && (zeichen != '\n')) *(run++) = zeichen;
   }
   *run = '\0';
   return 0;
}

int isblankline(line)
char *line;
{
      for (; *line != '\0'; line++) if (!isspace(*line)) return 0;
   return 1;
}

int linestartswidth(line, needle)
char *line;
char *needle;
{
   while (isspace(*line)) line++;
   return (strncmp(line, needle, strlen(needle)) == 0);
}

#define BUFFERSIZE 10

int main(argc, argv)
int argc;
char **argv;
{
   FILE *inpfile, *outfile;
   char lines[BUFFERSIZE][500], *p;
   char orig[1000], dest[1000], params[1000], single[1000], save;
   int BufferFill, start, z, flag;

   if (argc != 3) {
      fprintf(stderr, "usage: %s <input file> <output file>\n", argv[0]);
      exit(1);
   }

   if (strcmp(argv[1], "-") == 0) inpfile = stdin;
   else inpfile = fopen(argv[1], "r");
   if (inpfile == NULL) {
      perror(argv[1]);
      return 2;
   }

   if (strcmp(argv[2], "-") == 0) outfile = stdout;
   else outfile = fopen(argv[2], "w");
   if (outfile == NULL) {
      perror(argv[2]);
      return 2;
   }

   BufferFill = 0;
   while (!feof(inpfile)) {
      if (BUFFERSIZE == BufferFill) {
         fprintf(outfile, "%s\n", lines[0]);
         for (z = 0; z < BufferFill; z++) strcopy(lines[z], lines[z + 1]);
         BufferFill--;
      }
      readline(inpfile, lines[BufferFill++]);

   /* condition for a function header:
      1. body begins with '{' in first column
      2. header searched backward until blank, comment, or preprocessor
      line detected */

      if (strcmp(lines[BufferFill - 1], "{") == 0) {
         for (start = BufferFill - 1; start >= 0; start--)
            if (isblankline(lines[start])) break;
            else if (*lines[start] == '#') break;
            else if (strncmp(lines[start], "/*", 2) == 0) break;
            else if (strcmp(lines[start] + strlen(lines[start]) - 2, "*/") == 0) break;
         start++;

      /* found: assemble source lines into a single line */

         for (z = start, *orig = '\0'; z <= BufferFill - 2; z++) {
            p = lines[z];
            while (isspace(*p)) p++;
            strcat(orig, p);
            if (z != BufferFill - 2) strcat(orig, " ");
         }

      /* cut function name+prefixes: parameter list starts at first '(' */

         p = strchr(orig, '(');
         *p = '\0';
         sprintf(dest, "\t%s", orig);
         strcat(dest, "(");
         strcopy(orig, p + 1);

      /* cut trailing ')' */

         for (p = orig + strlen(orig) - 1; *p != ')'; p--);
         *p = '\0';

      /* loop through parameters: discard 'void' entries */

         *params = 0;
         flag = 0;
         while (*orig != '\0') {
            p = strchr(orig, ',');
            if (p == NULL) {
               strcopy(single, orig);
               *orig = '\0';
            } else {
               *p = '\0';
               strcopy(single, orig);
               strcopy(orig, p + 1);
            }
            for (p = single; isspace(*p); p++);
            strcopy(single, p);
            for (p = single + strlen(single) - 1; isspace(*p); p--);
            p[1] = '\0';
            if (strncmp(single, "const ", 6) == 0) strmove(single, 6);
            if (strcmp(single, "void") != 0) {
               strcat(params, single);
               strcat(params, ";\n");
               for (p = single + strlen(single) - 1; (isalnum(*p)) || (*p == '_'); p--);
               if (flag) strcat(dest, ",");
               strcat(dest, p + 1);
               flag = 1;
            }
         }

      /* close function head */

         strcat(dest, ")");

      /* flush contents berore header from buffer */

         for (z = 0; z < start; fprintf(outfile, "%s\n", lines[z++]));

      /* print K&R header; don't forget opening of function body! */

         fprintf(outfile, "%s\n%s{\n", dest, params);

      /* flush buffer */

         BufferFill = 0;
      }

   /* Detect 'extern' definitions in header files */

      else if (linestartswidth(lines[BufferFill - 1], "extern ")) {
      /* find opening parenthesis.  if none, it's a variable definition */

         p = strchr(lines[BufferFill - 1], '(');
         if (p != NULL) {
         /* if next character is a '*', we have an external definition of
            a function pointer and have to find the next '(' */

            if (p[1] == '*') p = strchr(p + 1, '(');

         /* copy out first part, extend by ');' and write the 'prototype'.
            flush line buffer before. */

            save = p[1];
            p[1] = '\0';
            strcopy(dest, lines[BufferFill - 1]);
            strcat(dest, ");");
            p[1] = save;

            for (z = 0; z < BufferFill - 1; fprintf(outfile, "%s\n", lines[z++]));
            fprintf(outfile, "%s\n", dest);

         /* discard lines until end of prototype */

            strcopy(dest, lines[BufferFill - 1]);
            BufferFill = 0;
            while (strcmp(dest + strlen(dest) - 2, ");") != 0) {
               readline(inpfile, dest);
            }
         }
      }
   }

   for (z = 0; z < BufferFill; fprintf(outfile, "%s\n", lines[z++]));

   fclose(inpfile);
   fclose(outfile);
   return 0;
}
