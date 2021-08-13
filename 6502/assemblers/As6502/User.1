.de hd
'sp 3
.tl ''\fBas6502 User Notes\fR'Page %'
'sp 2
..
.de fo
'bp
..
.wh 0 hd
.wh -3 fo
.br
\fBSOURCE LINE FORMAT\fR:
.ti 0.5i
.sp 1
<label> <operation> <operand> <comment>
.sp 1
Each field is terminated by one or more spaces, a tab or a ';' (which
begins the comment field immediately).
.sp
\fBLABEL FIELD\fR:
.sp 1
If first character is ';', entire line is a comment.
If first character is space, label field is null.
Labels are alphanumeric strings beginning
with 'a' through 'z', 'A' through 'Z',
underscore or period followed by any of the above
characters or '0' through '9'.
Currently, labels are limited to 19 characters.
A, X, Y, a, x and y are reserved labels.
.sp
\fBOPERATION FIELD\fR:
.sp 1
Upper and lower case letters are equivalent.
Machine operation mnemonics are:
.in 0.5i
.nf
.sp 1
ADC     BMI     CLD     DEX     JSR     PHA     RTS     STY
AND     BNE     CLI     DEY     LDA     PHP     SBC     TAX
ASL     BPL     CLV     EOR     LDX     PLA     SEC     TAY
BCC     BRK     CMP     INC     LDY     PLP     SED     TSX
BCS     BVC     CPX     INX     LSR     ROL     SEI     TXA
BEQ     BVS     CPY     INY     NOP     ROR     STA     TXS
BIT     CLC     DEC     JMP     ORA     RTI     STX     TYA
.in 0
.fi
.sp 1
Pseudo operation mnemonics are:
.in 1.2i
.ti 0.5i
.sp 1
=      equate label name to operand field value
(space is not needed to terminate this operation).
.ti 0.5i
*=     set location counter to operand field value
(space is not needed to terminate this operation).
.ti 0.5i
.tr*.
*WORD  assign 16 bit value of operand field to next
two locations; low byte of value first, then high byte.
.ti 0.5i
*DBYT  assign 16 bit value of operand field to next
two locations; high byte of value first, then low byte.
.ti 0.5i
*BYTE  assign 8 bit value of operand field to next
location.
.ti 0.5i
*NLST  turn listing mode off (this source
line is not listed).
.ti 0.5i
*LIST  turn listing mode on (normal mode) (this
source line is not listed).
.ti 0.5i
*PAGE  start a new listing page.
Optionally specify a title for the page heading.
(this source line is not listed).
.tr**
.in 0
.sp 1
\fBOPERAND FIELD\fR:
.sp 1
Operand field expressions use infix notation and are evaluated strictly
from left to right.
No imbedded spaces are permitted.
.sp 1
Operand field terms include labels and numbers.
Asterisk (*) is the label for the location counter value.
Numbers are binary, octal, decimal, hexadecimal or ASCII.
Number type is indicated by the first character of the number string as follows:
.nf
.in 0.5i
.sp 1
%       binary prefix
@ or 0  octal prefix
1 - 9   decimal by default (prefix is part of number)
$       hexadecimal prefix
.tr,'
,       ASCII character prefix
.tr,,
"       ASCII character string prefix and suffix; in the
        string, \\t is a tab character, \\n is a new line.
.in 0
.fi
.sp 1
Operand field operations and the corresponding symbols are:
.in 0.5i
.nf
.sp 1
+       addition
-       subtraction
/       division
*       multiplication
%       modulo (remainder after integer division)
^       logical exclusive OR
&       logical AND
|       logical OR
<       low byte
>       high byte
.in 0
.fi
.sp 1
Addressing modes are represented as follows:
.in 0.5i
.nf
 lda =aa 	 immediate addressing
 lda #aa 	 immediate addressing, alternate
 lda D 	 	 direct addessing
 LDA aa 	 page zero addressing, aa < 256
 asl A 	 	 accumulator addressing
 brk 	 	 implied addressing
 lda (aa,X) 	 indirect,X addressing
 lda (aa),Y 	 indirect,Y addressing
 lda aa,X 	 zero page,X addressing
 lda D,X 	 absolute,X addressing
 lda D,Y 	 absolute,Y addressing
 bcc *-$10 	 relative addressing
 jmp (D) 	 indirect addressing
 ldx aa,Y 	 zero page,Y addressing
.fi
.sp 1
\fBERROR MESSAGES\fR:
.in 0.5i
.nf
.sp 1
Invalid operation code
Invalid argument count (when as6502 was invoked)
Open error for file
Creat error for object file 6502.out
Close error
Close error (6502.out)
Symbol table full
Label multiply defined
Sync error (pass 1 symbol value not equal pass 2 symbol value)
Invalid branch address
Operand field missing
Invalid addressing mode
Operand field size error
Undefined symbol in operand field
Invalid operand field
.fi
.in 0
.sp 1
\fBINVOKING as6502\fR:
.sp 1
.ti 0.5i
as6502 {-ilnmos} {-p -t -w } <source files descriptions>
.sp 1
Options:
.in 0.5i
.nf
.sp 1
-i   ignore any .nlst pseudo operations
-l   list errors only
-n   print addresses as <high byte><low byte>,
     rather than as <low byte>:<high byte>.
-o   generate ASCII object output in file 6502.out,
     format is
          ;<address lo><address hi><data>
-s   print symbol table at end of listing
-m   output the object file in standard MOS Technology format.
.sp 2
Parameters:
.in 0.5i
.nf
-p   specify the page length.
-t   specify the symbol table size.
-w   specify the print line size.

                J. H. Van Ornum, JHS  3/3/87
