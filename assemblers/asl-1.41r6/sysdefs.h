// Preliminary adjustments to the Systems Matrix.
// Unify 68K platforms.
#if (defined __mc68020 || defined MOT_IEEE) && !defined __m68k
#   define __m68k
#endif
// Just a hack to allow distinguishing SunOS from Solaris on Sparcs.
#if defined sparc
#   define __sparc
#endif
#if defined __sparc
#   if defined __SVR4
#      define __solaris__
#   else
#      define __sunos__
#   endif
#endif
// Similar on Sun 3's.
#if defined __m68k && !defined __NetBSD__ && !defined __MUNIX__
#   define __sunos__
#endif
// MSDOS only runs on x86s.
#if defined __MSDOS__
#   define __i386
#endif

// The Systems Matrix:
// ───────────────────
// __m68k: __sunos__, __NetBSD__, __MUNIX__, __linux__
// __sparc: __sunos__, __solaris__
// __mips: __ultrix, __NetBSD__, __sgi
// __hppa: __hpux
// _POWER: _AIX
// __alpha: __osf__, __linux__
// __i386: __linux__, __FreeBSD__, __EMX__, __MSDOS__
// __x86_64__: __linux__
// _CRAYMPP

#if defined __m68k
// SUN/3 with SunOS 4.x: see my SunOS quarrels in the Sparc section.
#   if defined __sunos__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
#      if defined __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#         define HAS64
#      else
#         define NOLONGLONG
#      endif
#      define NO_NLS
#   endif
// SUN/3 with NetBSD 1.x: quite a normal 32-Bit-UNIX system.
#   if defined __NetBSD__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
// PCS/Cadmus: quite a bare system, lots of work required.
#   if defined __MUNIX__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
#      define NEEDS_CASECMP
#      define NEEDS_STRSTR
typedef char Integ8;
typedef unsigned char Card8;
typedef short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef int Integ32;
typedef unsigned int Card32;
#      define NOLONGLONG
#      define memmove(s1,s2,len) bcopy(s2,s1,len)
extern double strtod();
#      define NO_NLS
#   endif
// Linux/68K: quite a normal 32-Bit-UNIX system.
#   if defined __linux__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define NO_NLS
#   endif
#endif

// SPARC platforms
#if defined __sparc
// SUN Sparc with SunOS 4.1.x: don't try cc, use gcc, it's hopeless without an ANSI-compliant compiler.
// SunOS does have NLS support, but it does not have D_FMT and T_FMT
// I should change this.
// Though the manual pages claim that memmove and atexit exist, I could not find them in any library :-(
// Fortunately, bcopy claims to be safe for overlapping arrays, we just have to reverse source and destination pointers.
// The sources themselves contain a switch to use on_exit instead of atexit
// (it uses a different callback scheme, so we cannot just make a #define here.
// To get rid of most of the messages about missing prototypes, add -D__USE_FIXED_PROTOTYPES__ to your compiler flags!
// Apart from these few points, one could claim SunOS to be quite a normal 32-bit-UNIX.
#   if defined __sunos__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
#      if defined __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#         define HAS64
#      else
#         define NOLONGLONG
#      endif
#      define fpos_t long
extern void bcopy();
#      define memmove(s1,s2,len) bcopy(s2,s1,len)
#      define NO_NLS
#   endif
// SUN Sparc with Solaris 2.x: quite a normal 32-Bit-UNIX system.
#   if defined __solaris__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
#endif

// Mips platforms
#if defined __mips
// R3000 with Ultrix 4.3: nl_langinfo prototype is there, but no function in library ?!
// use long long only if you have gcc, c89 doesn't like them !
// cc isn't worth trying, believe me!
#   if defined __ultrix
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
#      define NEEDS_STRDUP
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
#      if defined __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#         define HAS64
#      else
#         define NOLONGLONG
#      endif
#      define NO_NLS
#   endif
// R2000/3000 with NetBSD 1.2: quite a normal 32-Bit-UNIX system.
#   if defined __NetBSD__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
// R3000/4x00 with Irix 5.x: quite a normal 32-Bit-UNIX system; seems also to work with 6.2.
#   if defined __sgi
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
#endif

// HP-PA platforms
#if defined __hppa
// HP-PA 1.x with HP-UX:
#   if defined __hpux
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
#endif

// POWER platforms
#if defined _POWER
// POWER with AIX 4.1:
#   if defined _AIX
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
#endif

// DEC Alpha platforms
#if defined __alpha
// DEC Alpha with Digital UNIX and DEC C / GCC:
// Alpha is a 64 bit machine, so we do not need to use extra longs OSF has full NLS support.
#   if defined __osf__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
// DEC Alpha with Linux and GCC: see OSF.
// NLS still missing; well, my Linux/Alpha is stone-age and still ECOFF-based.
#   if defined __linux__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#      define HAS64
#      define NO_NLS
#   endif
#endif

// Intel i386 platforms.
#if defined __i386
// Intel i386 with Linux and GCC: principally, a normal 32-bit *NIX.
#   if defined __linux__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
// Intel i386 with FreeBSD and GCC: principally, a normal 32-bit *NIX.
#   if defined __FreeBSD__
#      define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define NO_NLS
#   endif
// Intel i386 with OS/2 and emx-GCC: well, not really a UNIX.
#   if defined __EMX__
#      define DEFSMADE
#      define OPENRDMODE "rb"
#      define OPENWRMODE "wb"
#      define OPENUPMODE "rb+"
#      define PATHSEP '\\'
#      define SPATHSEP "\\"
#      define DRSEP ':'
#      define SDRSEP ":"
#      define NULLDEV "NUL"
#      define NEEDS_CASECMP
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define OS2_NLS
#   endif
// Intel x86 with MS-DOS and Borland-C: well, not really a UNIX; assure we get a usable memory model.
#   if defined __MSDOS__ && defined __TURBOC__
#      if !defined __HUGE__
#         error Wrong memory model - use huge!
#      endif
#      define DEFSMADE
#      define OPENRDMODE "rb"
#      define OPENWRMODE "wb"
#      define OPENUPMODE "rb+"
#      define PATHSEP '\\'
#      define SPATHSEP "\\"
#      define DRSEP ':'
#      define SDRSEP ":"
#      define NULLDEV "NUL"
#      define NEEDS_CASECMP
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed long Integ32;
typedef unsigned long Card32;
#      define NOLONGLONG
#      define NO_NLS
#   endif
#endif

// Intel x86_64 platforms.
#if defined __x86_64__
// Intel x86_64 with Linux and GCC.
#   if defined __linux__
#      define DEFSMADE
#      define OPENRDMODE "r"
#      define OPENWRMODE "w"
#      define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#      define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#      define HAS64
#      define LOCALE_NLS
#   endif
#endif

// Misc.
// Just for curiosity, it won't work without 16 bit int's.
#if defined _CRAYMPP
#   define OPENRDMODE "r"
#   define OPENWRMODE "w"
#   define OPENUPMODE "r+"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ32;
typedef unsigned short Card32;
typedef signed int Integ64;
typedef unsigned int Card64;
#   define HAS64
#   define LOCALE_NLS
#endif

// Post-Processing: check for definition, add defaults.
#if defined DEFSMADE
#   if !defined PATHSEP
#      define PATHSEP '/'
#      define SPATHSEP "/"
#   endif
#   if !defined NULLDEV
#      define NULLDEV "/dev/null"
#   endif
#else
#   error "your platform so far is not included in AS's header files!"
#   error "please edit sysdefs.h!"
#endif
