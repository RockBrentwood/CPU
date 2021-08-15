# -------------------------------------------------------------------------
# choose your compiler (must be ANSI-compliant!) and linker command, plus
# any additionally needed flags

CC = gcc
LD = gcc
CFLAGS = -O3 -fomit-frame-pointer -D__USE_GNU -Wall
LDFLAGS =

# -------------------------------------------------------------------------
# directories where binaries, includes, and manpages should go during
# installation

BINDIR = /usr/local/bin
INCDIR = /usr/local/include/asl
MANDIR = /usr/local/man

# -------------------------------------------------------------------------
# language AS will speak to you

LANGRSC = lang_DE

# -------------------------------------------------------------------------
# character encoding to use (choose one of them)

CHARSET = CHARSET_ISO8859_1
# CHARSET = CHARSET_ASCII7
# CHARSET = CHARSET_IBM437
