10  '  *******  EELOAD.BAS  25/9/86 *******'
20 ' Written by R.Soja, Motorola East Kilbride'
30 ' Motorola Copyright 1986'
40 ' This program uploads S record file to HC11 through special'
50 ' bootstrap program, designed to support 2864 EEPROMs'
60 ' Uploaded data is echoed on terminal.'
70 ' ==================================
80 CR$=CHR$(13)
90 MIN$=CHR$(32)
100 MAX$=CHR$(127)
110 ERM$="Can't find "
120 CHAN=0
130 CLS
140 PRINT "  <<<<<<<<<<<<<<<<<   2864 EEPROM loader  >>>>>>>>>>>>>>>>"
150 PRINT "  <<<<<<<<<<<<<<<  Motorola Copyright 1986   >>>>>>>>>>>>>"
160 PRINT
170  PRINT "==>  Before continuing, ensure 68HC11 is in bootstrap mode,"
180 PRINT "     RESET is off, and COM1 or COM2 is connected to the SCI"
190 PRINT
200 ' First make sure EEPROG.BOO is available'
210 ON ERROR GOTO 540
220 OPEN "EEPROG.BOO" FOR INPUT AS #2
230 CLOSE #2
240 ON ERROR GOTO 0
250 WHILE CHAN<1 OR CHAN>2
260 INPUT "Enter COM channel number (1/2):",CHAN
270 WEND
280 CM$="COM"+MID$(STR$(CHAN),2)
290 ' Now set baud rate to 1200 and load EEPROG through boot loader'
300 ' by executing DOS MODE and COPY commands'
310 SHELL "MODE "+CM$+":1200,N,8,1"
320 SHELL "COPY EEPROG.BOO "+CM$
330 ON ERROR GOTO 570
340 INPUT "Enter filename to upload: ",F$
350 CLOSE
360 OPEN F$ FOR INPUT AS #2
370 ON ERROR GOTO 0
380 'COM1 or 2 connected to SCI on HC11'
390 OPEN CM$+":9600,N,8,1" AS #1
400 ' Clear potential Rx error'
410 ON ERROR GOTO 520
420 PRINT #1,CR$;
430 WHILE LOC(1)=0:WEND:B$=INPUT$(1,#1)
440 WHILE NOT EOF(2)
450 A$=INPUT$(1,#2)
460 IF A$>MIN$ AND A$<MAX$ THEN                                                           PRINT #1,A$;:WHILE LOC(1)=0:WEND:B$=INPUT$(1,#1):PRINT B$;                ELSE  IF A$=CR$ THEN PRINT
470 WEND
480 CLOSE #2
490 SYSTEM
500 END
510 ' -----------------'
520 RESUME 350
530 ' -----------------'
540 PRINT ERM$;"EEPROG.BOO":PRINT "Program aborted"
550 GOTO 490
560 ' -----------------'
570 PRINT ERM$;F$;SPACE$(40)
580 RESUME 330
