Title:			Small C compiler
Author:			Dr A.J. Travis
Version:		0.7
Title From:		Documentation
Author From:		Documentation
Version From:		Documentation
Supplier:		Dr A. J. Travis
Date:			01/05/89
Keywords:               C ; Cross compilers ; Program compilers ;
                        Program processors ; Programming languages
Shareware:		No
Machine:		IBM PC
Operating system:	DOS
Memory requirements:	
Peripherals needed:	
Other s/ware needed:	
Directory:		micros/ibmpc/dos/f/f049
Date mounted/updated:	03/10/89
File names:		f049tcc.boo	Program
			f049tcc.doc	Documentation
Unarchived files:	
Unarchived size:	
Omissions:		
See also:		bbc/a02 for native BBC 6502 version

-------------------------------------------------------------------------------

Acquiring the package:

First download the BOO-format file or files, and use the DEBOO utility to
convert each to its binary archive format. Each BOO format file will produce
one file with a name ending in either .ARC, .ZOO or .ZIP, depending on the
archive system used.

Next use the appropriate archive program to extract the original files as
follows:
           .ARC files      Use PKUNPAK
           .ZOO files      Use ZOO
           .ZIP files      Use PKUNZIP

The DEBOO utility and the archive programs are in directory 
micros/ibmpc/dos/tools see the file micros/ibmpc/dos/tools/00readme for
details.

Description:

Small-C is a subset of the C programming language for which a number of
public-domain compilers have been written.  The original compiler was
written by Ron Cain and appeared in the May 1980 issue of Dr.Dobb's
Journal.  More recently, James E.Hendrix has improved and extended the
original Small-C compiler and published "The Small-C Handbook", ISBN
0-8359-7012-4 (1984).  Both compilers produce 8080 assembly language,
which is the most popular implementation of Small-C to-date.  My 6502
Small-C compiler for the BBC Micro is based on "RatC", a version of the
original Ron Cain compiler described by R.E.Berry and B.A.Meekings in "A
Book on C", ISBN 0-333-36821-5 (1984).  The 6502 compiler is written in
Small-C and was bootstrapped using Zorland C on an Amstrad PC1512 under
MSDOS 3.2, then transferred onto a BBC Micro using Kermit.  The compiler
can be used to cross-compile 6502 code from an MSDOS host, or as a
'resident' Small-C compiler on a BBC Micro. 

