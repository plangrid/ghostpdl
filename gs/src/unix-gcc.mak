#    Copyright (C) 1997, 1998, 1999 Aladdin Enterprises.  All rights reserved.
# 
# This file is part of Aladdin Ghostscript.
# 
# Aladdin Ghostscript is distributed with NO WARRANTY OF ANY KIND.  No author
# or distributor accepts any responsibility for the consequences of using it,
# or for whether it serves any particular purpose or works at all, unless he
# or she says so in writing.  Refer to the Aladdin Ghostscript Free Public
# License (the "License") for full details.
# 
# Every copy of Aladdin Ghostscript must include a copy of the License,
# normally in a plain ASCII text file named PUBLIC.  The License grants you
# the right to copy, modify and redistribute Aladdin Ghostscript, but only
# under certain conditions described in the License.  Among other things, the
# License requires that the copyright notice and this notice be preserved on
# all copies.


# makefile for Unix/gcc/X11 configuration.

# ------------------------------- Options ------------------------------- #

####### The following are the only parts of the file you should need to edit.

# Define the directory for the final executable, and the
# source, generated intermediate file, and object directories
# for the graphics library (GL) and the PostScript/PDF interpreter (PS).

BINDIR=./bin
GLSRCDIR=./src
GLGENDIR=./obj
GLOBJDIR=./obj
PSSRCDIR=./src
PSLIBDIR=./lib
PSGENDIR=./obj
PSOBJDIR=./obj

# Do not edit the next group of lines.

#include $(COMMONDIR)/gccdefs.mak
#include $(COMMONDIR)/unixdefs.mak
#include $(COMMONDIR)/generic.mak
include $(GLSRCDIR)/version.mak
DD=$(GLGENDIR)/
GLD=$(GLGENDIR)/
PSD=$(PSGENDIR)/

# ------ Generic options ------ #

# Define the installation commands and target directories for
# executables and files.  The commands are only relevant to `make install';
# the directories also define the default search path for the
# initialization files (gs_*.ps) and the fonts.

INSTALL = $(GLSRCDIR)/instcopy -c
INSTALL_PROGRAM = $(INSTALL) -m 755
INSTALL_DATA = $(INSTALL) -m 644

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
scriptdir = $(bindir)
mandir = $(prefix)/man
man1ext = 1
man1dir = $(mandir)/man$(man1ext)
datadir = $(prefix)/share
gsdir = $(datadir)/ghostscript
gsdatadir = $(gsdir)/$(GS_DOT_VERSION)

docdir=$(gsdatadir)/doc
exdir=$(gsdatadir)/examples
GS_DOCDIR=$(docdir)

# Define the default directory/ies for the runtime
# initialization and font files.  Separate multiple directories with a :.

GS_LIB_DEFAULT=$(gsdatadir)/lib:$(gsdir)/fonts

# Define whether or not searching for initialization files should always
# look in the current directory first.  This leads to well-known security
# and confusion problems, but users insist on it.
# NOTE: this also affects searching for files named on the command line:
# see the "File searching" section of Use.htm for full details.
# Because of this, setting SEARCH_HERE_FIRST to 0 is not recommended.

SEARCH_HERE_FIRST=1

# Define the name of the interpreter initialization file.
# (There is no reason to change this.)

GS_INIT=gs_init.ps

# Choose generic configuration options.

# -DDEBUG
#	includes debugging features (-Z switch) in the code.
#	  Code runs substantially slower even if no debugging switches
#	  are set.
# -DNOPRIVATE
#	makes private (static) procedures and variables public,
#	  so they are visible to the debugger and profiler.
#	  No execution time or space penalty.

GENOPT=

# Define the name of the executable file.

GS=gs

# Define the directories for debugging and profiling binaries, relative to
# the standard binaries.

DEBUGRELDIR=../debugobj
PGRELDIR=../pgobj

# Define the directory where the IJG JPEG library sources are stored,
# and the major version of the library that is stored there.
# You may need to change this if the IJG library version changes.
# See jpeg.mak for more information.

JSRCDIR=jpeg
JVERSION=6

# Choose whether to use a shared version of the IJG JPEG library (-ljpeg).
# DON'T DO THIS. If you do, the resulting executable will not be able to
# read some PostScript files containing JPEG data, because Adobe chose to
# define PostScript's JPEG capabilities in a way that is slightly
# incompatible with the JPEG standard.  See Make.htm for more details.

# DON'T SET THIS TO 1!  See the comment just above.
SHARE_JPEG=0
JPEG_NAME=jpeg

# Define the directory where the PNG library sources are stored,
# and the version of the library that is stored there.
# You may need to change this if the libpng version changes.
# See libpng.mak for more information.

PSRCDIR=libpng
PVERSION=10003

# Choose whether to use a shared version of the PNG library, and if so,
# what its name is.
# See gs.mak and Make.htm for more information.

SHARE_LIBPNG=0
LIBPNG_NAME=png

# Define the directory where the zlib sources are stored.
# See zlib.mak for more information.

ZSRCDIR=zlib

# Choose whether to use a shared version of the zlib library, and if so,
# what its name is (usually libz, but sometimes libgz).
# See gs.mak and Make.htm for more information.

SHARE_ZLIB=0
#ZLIB_NAME=gz
ZLIB_NAME=z

# Define how to build the library archives.  (These are not used in any
# standard configuration.)

AR=ar
ARFLAGS=qc
RANLIB=ranlib

# ------ Platform-specific options ------ #

# Define the name of the C compiler.

CC=gcc

# Define the name of the linker for the final link step.
# Normally this is the same as the C compiler.

CCLD=$(CC)

# Define the default gcc flags.
# Note that depending whether or not we are running a version of gcc with
# the 2.7.0-2.7.2 optimizer bug, either "-Dconst=" or
# "-Wcast-qual -Wwrite-strings" is automatically included.

GCFLAGS=-Wall -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wtraditional -fno-builtin -fno-common

# Define the added flags for standard, debugging, and profiling builds.

CFLAGS_STANDARD=-O2
CFLAGS_DEBUG=-g -O
CFLAGS_PROFILE=-pg -O2

# Define the other compilation flags.  Add at most one of the following:
#	-DBSD4_2 for 4.2bsd systems.
#	-DSYSV for System V or DG/UX.
# 	-DSYSV -D__SVR3 for SCO ODT, ISC Unix 2.2 or before,
#	   or any System III Unix, or System V release 3-or-older Unix.
#	-DSVR4 -DSVR4_0 (not -DSYSV) for System V release 4.0.
#	-DSVR4 (not -DSYSV) for System V release 4.2 (or later) and Solaris 2.
# XCFLAGS can be set from the command line.
# We don't include -ansi, because this gets in the way of the platform-
#   specific stuff that <math.h> typically needs; nevertheless, we expect
#   gcc to accept ANSI-style function prototypes and function definitions.
XCFLAGS=

CFLAGS=$(CFLAGS_STANDARD) $(GCFLAGS) $(XCFLAGS)

# Define platform flags for ld.
# SunOS 4.n may need -Bstatic.
# Solaris 2.6 (and possibly some other versions) with any of the SHARE_
# parameters set to 1 may need
#	-R /usr/local/xxx/lib:/usr/local/lib
# giving the full path names of the shared library directories.
# XLDFLAGS can be set from the command line.
XLDFLAGS=

LDFLAGS=$(XLDFLAGS) -fno-common

# Define any extra libraries to link into the executable.
# ISC Unix 2.2 wants -linet.
# SCO Unix needs -lsocket if you aren't including the X11 driver.
# SVR4 may need -lnsl.
# Solaris may need -lnsl -lsocket -lposix4.
# (Libraries required by individual drivers are handled automatically.)

EXTRALIBS=

# Define the include switch(es) for the X11 header files.
# This can be null if handled in some other way (e.g., the files are
# in /usr/include, or the directory is supplied by an environment variable);
# in particular, SCO Xenix, Unix, and ODT just want
#XINCLUDE=
# Note that x_.h expects to find the header files in $(XINCLUDE)/X11,
# not in $(XINCLUDE).

XINCLUDE=-I/usr/local/X/include

# Define the directory/ies and library names for the X11 library files.
# XLIBDIRS is for ld and should include -L; XLIBDIR is for LD_RUN_PATH
# (dynamic libraries on SVR4) and should not include -L.
# Newer SVR4 systems can use -R in XLIBDIRS rather than setting XLIBDIR.
# Both can be null if these files are in the default linker search path;
# in particular, SCO Xenix, Unix, and ODT just want
#XLIBDIRS=
# Solaris and other SVR4 systems with dynamic linking probably want
#XLIBDIRS=-L/usr/openwin/lib -R/usr/openwin/lib
# X11R6 (on any platform) may need
#XLIBS=Xt SM ICE Xext X11

#XLIBDIRS=-L/usr/local/X/lib
XLIBDIRS=-L/usr/X11/lib
XLIBDIR=
XLIBS=Xt Xext X11

# Define whether this platform has floating point hardware:
#	FPU_TYPE=2 means floating point is faster than fixed point.
# (This is the case on some RISCs with multiple instruction dispatch.)
#	FPU_TYPE=1 means floating point is at worst only slightly slower
# than fixed point.
#	FPU_TYPE=0 means that floating point may be considerably slower.
#	FPU_TYPE=-1 means that floating point is always much slower than
# fixed point.

FPU_TYPE=1

# Define the .dev module that implements thread and synchronization
# primitives for this platform.  Don't change this unless you really know
# what you're doing.

SYNC=posync

# ------ Devices and features ------ #

# Choose the language feature(s) to include.  See gs.mak for details.
# Note that there is one feature that requires a GNU library:
# $(PSD)gnrdline.dev, which adds support for GNU readline, including
# on-the-fly name completion and evaluation.  For details, including
# licensing problems that will arise if you add this feature to Aladdin
# Ghostscript releases, see gp_gnrdl.c.

FEATURE_DEVS=$(PSD)psl3.dev $(PSD)pdf.dev $(PSD)dpsnext.dev $(PSD)ttfont.dev $(PSD)pipe.dev
#FEATURE_DEVS=$(PSD)psl3.dev $(PSD)pdf.dev $(PSD)dpsnext.dev $(PSD)ttfont.dev $(PSD)pipe.dev $(PSD)rasterop.dev
# The following is strictly for testing.
FEATURE_DEVS_ALL=$(PSD)psl3.dev $(PSD)pdf.dev $(PSD)dpsnext.dev $(PSD)ttfont.dev $(PSD)pipe.dev $(PSD)rasterop.dev $(PSD)double.dev $(PSD)trapping.dev $(PSD)compht.dev $(PSD)gnrdline.dev
#FEATURE_DEVS=$(FEATURE_DEVS_ALL)

# Choose whether to compile the .ps initialization files into the executable.
# See gs.mak for details.

COMPILE_INITS=0

# Choose whether to store band lists on files or in memory.
# The choices are 'file' or 'memory'.

BAND_LIST_STORAGE=file

# Choose which compression method to use when storing band lists in memory.
# The choices are 'lzw' or 'zlib'.  lzw is not recommended, because the
# LZW-compatible code in Ghostscript doesn't actually compress its input.

BAND_LIST_COMPRESSOR=zlib

# Choose the implementation of file I/O: 'stdio', 'fd', or 'both'.
# See gs.mak and sfxfd.c for more details.

FILE_IMPLEMENTATION=stdio

# Define the name table capacity size of 2^(16+n).
# Setting this to a non-zero value will slow down the interpreter.

EXTEND_NAMES=0

# Choose the device(s) to include.  See devs.mak for details,
# devs.mak and contrib.mak for the list of available devices.

DEVICE_DEVS=$(DD)x11.dev $(DD)x11alpha.dev $(DD)x11cmyk.dev $(DD)x11gray2.dev $(DD)x11gray4.dev $(DD)x11mono.dev

#DEVICE_DEVS1=
#DEVICE_DEVS2=
#DEVICE_DEVS3=
#DEVICE_DEVS4=
#DEVICE_DEVS5=
#DEVICE_DEVS6=
#DEVICE_DEVS7=
#DEVICE_DEVS8=
#DEVICE_DEVS9=
#DEVICE_DEVS10=
#DEVICE_DEVS11=
#DEVICE_DEVS12=
#DEVICE_DEVS13=
#DEVICE_DEVS14=
#DEVICE_DEVS15=

DEVICE_DEVS1=$(DD)bmpmono.dev $(DD)bmpgray.dev $(DD)bmpsep1.dev $(DD)bmpsep8.dev $(DD)bmp16.dev $(DD)bmp256.dev $(DD)bmp16m.dev $(DD)bmp32b.dev
DEVICE_DEVS2=$(DD)bmpamono.dev $(DD)bmpasep1.dev $(DD)bmpasep8.dev $(DD)bmpa16.dev $(DD)bmpa256.dev $(DD)bmpa16m.dev $(DD)bmpa32b.dev
DEVICE_DEVS3=$(DD)deskjet.dev $(DD)djet500.dev $(DD)laserjet.dev $(DD)ljetplus.dev $(DD)ljet2p.dev $(DD)ljet3.dev $(DD)ljet3d.dev $(DD)ljet4.dev $(DD)ljet4d.dev $(DD)lj5mono.dev $(DD)lj5gray.dev
DEVICE_DEVS4=$(DD)cdeskjet.dev $(DD)cdjcolor.dev $(DD)cdjmono.dev $(DD)cdj550.dev $(DD)pj.dev $(DD)pjxl.dev $(DD)pjxl300.dev
DEVICE_DEVS5=$(DD)uniprint.dev
DEVICE_DEVS6=$(DD)bj10e.dev $(DD)bj200.dev $(DD)bjc600.dev $(DD)bjc800.dev
DEVICE_DEVS7=$(DD)faxg3.dev $(DD)faxg32d.dev $(DD)faxg4.dev
DEVICE_DEVS8=$(DD)pcxmono.dev $(DD)pcxgray.dev $(DD)pcx16.dev $(DD)pcx256.dev $(DD)pcx24b.dev $(DD)pcxcmyk.dev
DEVICE_DEVS9=$(DD)pbm.dev $(DD)pbmraw.dev $(DD)pgm.dev $(DD)pgmraw.dev $(DD)pgnm.dev $(DD)pgnmraw.dev $(DD)pnm.dev $(DD)pnmraw.dev $(DD)ppm.dev $(DD)ppmraw.dev $(DD)pkm.dev $(DD)pkmraw.dev $(DD)pksm.dev $(DD)pksmraw.dev
DEVICE_DEVS10=$(DD)tiffcrle.dev $(DD)tiffg3.dev $(DD)tiffg32d.dev $(DD)tiffg4.dev $(DD)tifflzw.dev $(DD)tiffpack.dev
DEVICE_DEVS11=$(DD)tiff12nc.dev $(DD)tiff24nc.dev
DEVICE_DEVS12=$(DD)psmono.dev $(DD)psgray.dev $(DD)psrgb.dev $(DD)bit.dev $(DD)bitrgb.dev $(DD)bitcmyk.dev
DEVICE_DEVS13=$(DD)pngmono.dev $(DD)pnggray.dev $(DD)png16.dev $(DD)png256.dev $(DD)png16m.dev
DEVICE_DEVS14=$(DD)jpeg.dev $(DD)jpeggray.dev
DEVICE_DEVS15=$(DD)pdfwrite.dev $(DD)pswrite.dev $(DD)epswrite.dev $(DD)pxlmono.dev $(DD)pxlcolor.dev

DEVICE_DEVS16=
DEVICE_DEVS17=
DEVICE_DEVS18=
DEVICE_DEVS19=
DEVICE_DEVS20=

# ---------------------------- End of options --------------------------- #

# Define the name of the partial makefile that specifies options --
# used in dependencies.

MAKEFILE=$(GLSRCDIR)/unix-gcc.mak
TOP_MAKEFILES=$(MAKEFILE) $(GLSRCDIR)/unixhead.mak

# Define the ANSI-to-K&R dependency.  There isn't one, but we do have to
# detect whether we're running a version of gcc with the const optimization
# bug.

AK=$(GLGENDIR)/cc.tr

# Define the compilation rules and flags.

CCFLAGS=$(GENOPT) $(CFLAGS)
CC_=$(CC) `cat $(AK)` $(CCFLAGS)
CCAUX=$(CC) `cat $(AK)`
CC_LEAF=$(CC_) -fomit-frame-pointer
# gcc can't use -fomit-frame-pointer with -pg.
CC_LEAF_PG=$(CC_)
# These are the specific warnings we have to turn off to compile those
# specific few files that need this.  We may turn off others in the future.
CC_NO_WARN=$(CC_) -Wno-cast-qual -Wno-traditional

# ---------------- End of platform-specific section ---------------- #

include $(GLSRCDIR)/unixhead.mak
include $(GLSRCDIR)/gs.mak
include $(GLSRCDIR)/lib.mak
include $(PSSRCDIR)/int.mak
include $(PSSRCDIR)/cfonts.mak
include $(GLSRCDIR)/jpeg.mak
# zlib.mak must precede libpng.mak
include $(GLSRCDIR)/zlib.mak
include $(GLSRCDIR)/libpng.mak
include $(GLSRCDIR)/devs.mak
include $(GLSRCDIR)/contrib.mak
include $(GLSRCDIR)/unix-aux.mak
include $(GLSRCDIR)/unixlink.mak
include $(GLSRCDIR)/unix-end.mak
include $(GLSRCDIR)/unixinst.mak

# This has to come last so it won't be taken as the default target.
$(AK):
	if ( $(CC) --version | egrep "^2\.7\.([01]|2(\.[^1-9]|$$))" >/dev/null ); then echo -Dconst= >$(AK); else echo -Wcast-qual -Wwrite-strings >$(AK); fi
