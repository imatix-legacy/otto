#! /bin/sh
#----------------------------------------------------------------------------
#   c           Compile ANSI C program ('UNIX' multiplatform, includes OS/2)
#
#   Written:    95/03/10  iMatix <tools@imatix.com>
#   Revised:    99/06/28  iMatix <tools@imatix.com>
#
#   Authors:    General: Pieter Hintjens <ph@imatix.com>
#               For OS/2: Ewen McNeill <ewen@imatix.com>
#               For SCO: Vance Shipley <vances@vambo.telco.on.ca>
#               For FreeBSD: Bruce Walter <walter@fortean.com>
#               For Solaris: Grant McDorman <grant@isgtec.com>
#
#   Syntax:     c filename...     Compile ANSI C program(s)
#               c -c filename...  (Backwards compatible: compile C programs)
#               c -l main...      Compile and link main program(s)
#               c -L main...      Link main program(s), no compile
#               c -S              Report detected system name
#               c -C              Report C compiler command syntax
#               c -r lib file     Replace file.o into library
#                 -v              (First arg prefix to above): be verbose
#                 -q              (First arg prefix to above): be quiet
#
#   Requires:   Bourne shell
#   Usage:      Compiles a subroutine or compiles/links a main program.
#               Will read configuration information from /usr/local/etc/c.conf
#               or /etc/c.conf (if present), from $HOME/.c.conf (if
#               present) and also from c.conf or .c.conf (if present).  
#               The configuration information takes the form of a Bourne 
#               shell script that is sourced into the current shell, but 
#               should generally contain only items setting variables, in
#               the form NAME=VALUE.
#
#               The following variables can be set to control the program:
#                   UTYPE         Type of Unix system
#                   CCNAME        Name of compiler (default: cc)
#                   CCOPTS        Options required for ANSI C compilation
#                   CCDEFINES     Macro definitions, etc, for compiler
#                   CCPRODLEVEL   One of:
#                                 debug:      debug macro, debug symbols
#                   (default)     standard:   debug macro, optimise
#                                 production: optimise, strip
#                                 The exact flags vary from system to system,
#                                 and some systems may not make distinction
#                   CCLIBNAME     Library to archive into if lib is "any"
#                   CCLIBS        Libraries to link against, also includes libs
#                                   in current directory and sfl if present
#                   RANLIB        If 1, ranlib will be run after library
#                                 archiving (otherwise 'ar rs' used)
#                   LINKPATH      If 1, link path must come before the
#                                 libraries to link; otherwise will be after
#           
#               Does SQL precompilation if required (currently does Oracle).
#               Return code: 0 = okay, 1 = compile error.
#
#   Copyright:  Copyright (c) 1995-99 iMatix Corporation
#   License:    This is free software; you can redistribute it and/or modify
#               it under the terms of the SFL License Agreement as provided
#               in the file LICENSE.TXT.  This software is distributed in
#               the hope that it will be useful, but without any warranty.
#----------------------------------------------------------------------------

# Save the potentially set configuration values
#
SAVED_UTYPE="$UTYPE"
SAVED_CCNAME="$CCNAME"
SAVED_CCOPTS="$CCOPTS"
SAVED_CCDEFINES="$CCDEFINES"
SAVED_CCPRODLEVEL="$CCPRODLEVEL"
SAVED_CCLIBS="$CCLIBS"
SAVED_RANLIB="$RANLIB"
SAVED_LINKPATH="$LINKPATH"
SAVED_LIBNAME="$LIBNAME"

# Read in global configuration file if present.  
#
if [ -r "/usr/local/etc/c.conf" ]; then
    . /usr/local/etc/c.conf
elif [ -r "/etc/c.conf" ]; then
    . /etc/c.conf
fi

# Read in per-user configuration file, if present
#
if [ -r "$HOME/.c.conf" ]; then
    . "$HOME/.c.conf"
fi

# Read in per-directory configuration file, if present.  This may contain
# values overriding the settings from the global configuration file.
# NOTE: Visible (ie, c.conf) filename tried first for obviousness
#
if [ -r "c.conf" ]; then
    . "c.conf"
elif [ -r ".c.conf" ]; then
    . ".c.conf"
fi

# Restore the saved values from the preexisting environment, if any,
# so that they override the per-system and per-directory settings.
#
# XXX: is this portable?  (Okay in bash, Solaris sh?)
#
UTYPE="${SAVED_UTYPE:-${UTYPE}}"
CCNAME="${SAVED_CCNAME:-${CCNAME}}"
CCOPTS="${SAVED_CCOPTS:-${CCOPTS}}"
CCDEFINES="${SAVED_CCDEFINES:-${CCDEFINES}}"
CCPRODLEVEL="${SAVED_CCPRODLEVEL:-${CCPRODLEVEL}}"
CCLIBS="${SAVED_CCLIBS:-${CCLIBS}}"
RANLIB="${SAVED_RANLIB:-${RANLIB}}"
LINKPATH="${SAVED_LINKPATH:-${LINKPATH}}"
LIBNAME="${SAVED_LIBNAME:-${LIBNAME}}"

#   If not already known, detect UNIX system type.  This algorithm returns 
#   one of these system names, as far as I know at present:
#
#       AIX      APOLLO   A/UX     BSD/OS    FreeBSD   HP-UX    IRIX
#       Linux    NCR      NetBSD   NEXT      OSF1      SCO      Pyramid
#       SunOS    ULTRIX   OS/2     UnixWare  Generic   SINIX-N
#
#   Sets the variable UTYPE to one of the UNIX system names above, and
#   CCOPTS to the appropriate compiler options for ANSI C compilation.

if [ -z "$UTYPE" ]; then
    UTYPE=Generic                       #   Default system name
    if [ -s /usr/bin/uname       ]; then UTYPE=`/usr/bin/uname`; fi
    if [ -s /bin/uname           ]; then UTYPE=`/bin/uname`;     fi

    if [ -s /usr/apollo/bin      ]; then UTYPE=APOLLO;   fi
    if [ -s /usr/bin/ncrm        ]; then UTYPE=NCR;      fi
    if [ -s /usr/bin/swconfig    ]; then UTYPE=SCO;      fi
    if [ -s /usr/lib/NextStep/software_version ]; \
                                    then UTYPE=NEXT;     fi
    if [ "$UTYPE" = "SMP_DC.OSx" ]; then UTYPE=Pyramid;  fi
    if [ -d /var/sadm/pkg/UnixWare ]; \
                                    then UTYPE=UnixWare; fi
    if [ -n "$COMSPEC" -o -n "$OS2_SHELL" ]; \
                                    then UTYPE=OS/2;     fi
fi

#   If CCPRODLEVEL is not already set, default to "standard".
#
CCPRODLEVEL="${CCPRODLEVEL:-standard}"

#   If CCDEFINES is not already set by the caller, and CCPRODLEVEL is
#   one of "standard" or "debug", then we define DEBUG. 
#   You can set CCPRODLEVEL to production, or define CCDEFINES to " " 
#   to switch-off debug support (e.g. assertions) or add your own definitions.
#
CCDEFINES="${CCDEFINES:--DDEBUG}"

#   Set specific system compiler options and other flags
#   CCNAME      Name of compiler
#   CCOPTS      Compiler options, except -c
#   LINKPATH    If 1, -L path must come before library list
#   RANLIB      Use ranlib command to reindex library; else use 'ar rs'
#
#   CCOPTS has no sensible default; so has to be either set by the 
#   general configuration variables, or detected based on system type
#   and compiler.  If CCOPTS is set, we assume the other values are
#   set or the defaults are correct.
#
#   NOTE: CCNAME default is set below these checks, so that we can test
#   on CCNAME when setting CCOPTS, and/or set CCNAME and CCOPTS together.
#
#   NOTE: It is encouraged to check the value of CCPRODLEVEL when setting
#   the CCOPTS flags.

RANLIB="${RANLIB:-0}"                    #   By default, "ar rs" is used
LINKPATH="${LINKPATH:-0}"                #   By default, accept '-lsfl... -L.'

if [ -z "$CCOPTS" ]; then
    if [ "$CCNAME" = "gcc" ]; then
        case "$CCPRODLEVEL" in 
           debug)       CCOPTS="-g -lsocket -lnsl -Wall -pedantic"   
                        ;;
           standard)    CCOPTS="-O2 -lsocket -lnsl -Wall -pedantic" 
                        ;;
           production)  CCOPTS="-s -O2 -lsocket -lnsl -Wall -pedantic"
                        ;;
        esac

    elif [ "$UTYPE" = "AIX" ]; then
        CCOPTS="-O"

    elif [ "$UTYPE" = "BSD/OS" ]; then
        CCOPTS="-O -Dbsd"
        RANLIB=1

    elif [ "$UTYPE" = "FreeBSD" ]; then
        CCOPTS="-O2 -Wall -pedantic"
        CCNAME="${CCNAME:-gcc}"             #   Use gcc if not set
        RANLIB=1

    elif [ "$UTYPE" = "HP-UX" ]; then
        CCOPTS="-O -Ae -D_HPUX_SOURCE"      #   May need -Aa
        LINKPATH=1

    elif [ "$UTYPE" = "Linux" ]; then
        CCNAME="${CCNAME:-gcc}"             #   Use gcc if not set
        case "$CCPRODLEVEL" in 
           debug)       CCOPTS="-g -Wall -pedantic"   
                        ;;
           standard)    CCOPTS="-O2 -Wall -pedantic" 
                        ;;
           production)  CCOPTS="-s -O2 -Wall -pedantic"
                        ;;
        esac

    elif [ "$UTYPE" = "NetBSD" ]; then
        RANLIB=1

    elif [ "$UTYPE" = "OS/2" ]; then
        #  EDM, 96/04/02: -Zsysv-signals turns on SysV-style signal handling
        #  which is assumed in some iMatix code.
        #
        CCOPTS="-O2 -lsocket -lnsl -Wall -pedantic -Zsysv-signals"
        CCNAME="${CCNAME:-gcc}"             #   Use gcc if not set
        case "$CCPRODLEVEL" in 
           debug)    CCOPTS="-g -lsocket -lnsl -Wall -pedantic -Zsysv-signals"
                     ;;
           standard) CCOPTS="-O2 -lsocket -lnsl -Wall -pedantic -Zsysv-signals"
                     ;;
           production)
                  CCOPTS="-s -O2 -lsocket -lnsl -Wall -pedantic -Zsysv-signals"
                     ;;
        esac

    elif [ "$UTYPE" = "SCO" ]; then
        CCOPTS="-Dsco"                      #   -O switch can cause problems
        LINKPATH=1

    elif [ "$UTYPE" = "SunOS" ]; then
        #   Must distinguish Solaris (SunOS 5.x and later)
        RELEASE=`uname -r`
        MAJOR=`expr "$RELEASE" : '\([0-9]*\)\.'`
        MINOR=`expr "$RELEASE" : "$MAJOR\\.\\([0-9]*\\)"`
        if [ "$MAJOR" -gt 5 -o \( "$MAJOR" -eq 5 -a "$MINOR" -gt 1 \) ] ; then
            CCOPTS="-g -Xa -lsocket -lnsl -DSYSV"
        else
            CCOPTS="-O -Xa -lsocket -lnsl"
        fi
        LINKPATH=1
        RANLIB=1

    elif [ "$UTYPE" = "UnixWare" ]; then
        LINKPATH=1

    elif [ "$UTYPE" = "SINIX-N" ]; then
        CCOPTS="-WO"

    else
        CCOPTS=""
    fi
fi

# Set compiler to default (cc) if not set already
# And patch together the CC options and defines into one variable
#
CCNAME="${CCNAME:-cc}"
CCOPTS="$CCOPTS $CCDEFINES"

#   Parse command line arguments, figure out what we are doing
#   (Parsing is currently fairly simplistic, and depends on ordering
#   of flags.  Could be improved later if required.)
#
#   Set default values for compile & link options
LINKUP=no
COMPILE=yes
VERBOSE=yes                       # Backwards compatible default

#   -v means verbose reports
if [ /$1/ = /-v/ ]; then
    VERBOSE=yes
    shift
fi

#   -q means quiet
if [ /$1/ = /-q/ ]; then
    VERBOSE=no
    shift
fi

#   -S means report detected system type
if [ /$1/ = /-S/ ]; then
    echo "$UTYPE"
    exit
fi

#   -C means report compiler syntax type
if [ /$1/ = /-C/ ]; then
    echo "$CCNAME -c $CCOPTS"
    exit
fi

#   -c means compile the object -- we were going to do that anyway,
#   but this ensures backwards compatibility
#
if  [ /$1/ = /-c/ ]; then
    shift
fi

#   -r means replace object file into library
#   The RANLIB symbol should be set to 1 if 'ar rs' does not work.
#   If the library is specified as 'any', uses value of CCLIBNAME, or
#   first lib*.a file in directory.  Creates library if necessary.
#   If 'any' library is specified but no library exists and CCLIBNAME
#   is not defined, uses libany.a.
#
if [ /$1/ = /-r/ ]; then
    if [ /$2/ = /any/ ]; then
        if [ -n "$CCLIBNAME" ]; then
            LIBRARY=$CCLIBNAME
        else
            LIBRARY=libany.a
            for LIBFILE in lib*.a; do
                if [ -f $LIBFILE ]; then
                    LIBRARY=$LIBFILE
                    break
                fi
            done
        fi
    else
        LIBRARY=$2
    fi
    LIBNAME=`echo $LIBRARY | cut -d"." -f1`
    shift; shift
    
    for i in $*; do
        shift
        OBJECT=`echo $i | cut -d"." -f1`.o
        if [ "$VERBOSE" = "yes" ]; then
            echo "Replacing object $OBJECT in library $LIBRARY..."
        fi
        if [ "$RANLIB" = "1" ]; then
            ar r $LIBNAME.a $OBJECT
            ranlib $LIBNAME.a
        else
            ar rs $LIBNAME.a $OBJECT
        fi
    done
    exit 
fi

#   Compile/link main if -l is first argument
if [ /$1/ = /-l/ ]; then
    LINKUP=yes
    shift
fi

#   Link main if -L is first argument (assumed to already be compiled)
if [ /$1/ = /-L/ ]; then
    LINKUP=yes
    COMPILE=no
    shift
fi

#   If we will be linking, and don't already have a list of libraries to
#   link against, then build list of libraries to link with.
#
#   Modified for OS/2, EDM, 96/12/30 (OS/2 uses name.a, rather than libname.a)

if [ "$LINKUP" = "yes" -o /$1/ = // ]; then
    LIBSFL=0
    LIBLIST=""
    for LIBRARY in lib*.a; do
        if [ "$LIBRARY" = "lib*.a" ]; then
            break
        fi
        #   Pull out the xxx from libxxx.a (or similiar)
        LIBNAME=`echo $LIBRARY | sed -e 's/^...\([^\.]*\)\..*$/\1/'`

        #  libsfl.a must come last, so we will just flag it
        if [ "$LIBNAME" = "sfl" ]; then
            LIBSFL=1
        else
            #  In OS/2 we must link with the whole name, including 
            #  lib prefix, if any
            if [ "$UTYPE" = "OS/2" ]; then
                 LIBNAME=`echo $LIBRARY | cut -d"." -f1`
            fi
            LIBLIST="$LIBLIST -l$LIBNAME"
        fi
    done

    #   XXX -- Possible we should put this list in both orders, ie
    #   forward and backwards?  (sort; sort -r)

    #   Build list of libraries to include in the link command, including 
    #   those referred to by libsfl.a:
    #
    #      - /usr/lib/libsfl.a, if not found locally
    #      - /emx/lib/libsfl.a, or /local/emx/lib/libsfl.a, under OS/2, 
    #        (if present)
    #
    #   Modified for OS/2, by EDM, 96/12/30 (searching for libsfl.a)

    if [ -n "$CCLIBS" ]; then
        LIBLIST="$LIBLIST $CCLIBS"
    elif [ "$UTYPE" = "OS/2" ]; then
        if [ $LIBSFL -eq 1 \
        -o -f /usr/lib/libsfl.a \
        -o -f /emx/lib/libsfl.a \
        -o -f /local/emx/lib/libsfl.a ]; then
            LIBLIST="$LIBLIST -llibsfl"
        fi
    else
        if [ $LIBSFL -eq 1 \
        -o -f /usr/lib/libsfl.a ]; then
            LIBLIST="$LIBLIST -lsfl"
        fi
    fi
fi

#   Show help if no arguments
if [ /$1/ = // ]; then
    echo "Detected system=$UTYPE, compiles with:"
    echo "     $CCNAME -c $CCOPTS"
    echo "Syntax: c filename...    Compile ANSI C program(s)"
    echo "        c -c filename... Compile ANSI C programs(s)"
    echo "        c -l main...     Compile and link main program(s)"
    echo "        c -L main...     Link main(s) with" ${LIBLIST-"no libraries"}
    echo "        c -S             Report detected system name"
    echo "        c -C             Report C compiler command syntax"
    echo "        c -r lib file    Replace file into specified library"
    echo "          -v             (First arg prefix to above): be verbose"
    echo "          -q             (First arg prefix to above): be quiet"
    exit
fi

#   Compile and maybe link each filename on the command line
for i in $*; do
    shift
    FILENAME=`echo $i | cut -d"." -f1`

    #   Precompilation is required if program contains 'EXEC SQL'
    egrep -- "EXEC SQL" $FILENAME.c >/dev/null
    if [ $? -eq 0 ]; then
        PRECOMPILE=yes
    else
        PRECOMPILE=no
    fi

    #   Compile, if required
    PRECOMPILED=0
    if [ "$COMPILE" = "yes" -o ! -f $FILENAME.o ]; then
        if [ "$PRECOMPILE" = "yes" ]; then
            #   Precompile using Oracle SQL pre-compiler
            if [ "$ORACLE_HOME" != "" ]; then
                mv $FILENAME.c $FILENAME.pc
                nice proc16 $PCCFLAGS code=ansi_c ireclen=255 \
                            iname=$FILENAME.pc oname=$FILENAME.c
                $PRECOMPILED=1
            fi
        fi
        if [ -f $FILENAME.o ]; then
            rm $FILENAME.o
        fi
        if [ "$VERBOSE" = "yes" ]; then
#           echo "Compiling $FILENAME... ($CCNAME -c $CCOPTS $FILENAME.c)"
            echo "Compiling $FILENAME..."
        fi
        nice $CCNAME -c $CCOPTS $FILENAME.c 2> $FILENAME.lst

        #   Show listing and abort if there was a compile error
        if [ $? -eq 0 ]; then
            cat $FILENAME.lst
            rm  $FILENAME.lst
        else
            cat $FILENAME.lst
            exit 1
        fi

        #   If we precompiled the program, restore original source code
        if [ $PRECOMPILED -eq 1 ]; then
            mv $FILENAME.pc $FILENAME.c
        fi
    fi

    #   If okay, link if required
    if [ "$LINKUP" = "yes" ]; then
        if [ "$VERBOSE" = "yes" ]; then
#           echo "Linking $FILENAME... ($CCNAME $CCOPTS $FILENAME.o $LIBLIST)"
            echo "Linking $FILENAME..."
        fi
        if   [ "$UTYPE" = "OS/2" ]; then
            nice $CCNAME $CCOPTS $FILENAME.o -o $FILENAME.exe $LIBLIST -L. \
                2> $FILENAME.lst
        elif [ "$LINKPATH" = "1" ]; then
            nice $CCNAME $CCOPTS $FILENAME.o -o $FILENAME -L. $LIBLIST $LIBLIST \
               2> $FILENAME.lst
        else
            nice $CCNAME $CCOPTS $FILENAME.o -o $FILENAME $LIBLIST $LIBLIST -L. \
               2> $FILENAME.lst
        fi

        #   Show listing and abort if there was a link error
        if [ $? -eq 0 ]; then
            cat $FILENAME.lst
            rm  $FILENAME.lst
        else
            cat $FILENAME.lst
            exit 1
        fi
    fi
done

