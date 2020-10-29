#
#  Makefile for program(s) testbtree
#   generated automatically
#
#   (C) Joerg Schoen 1994-98
#

#  My preferred optimization flags for some systems:
# --- Silicon Graphics ---
#COPTIMIZE= -O2 -mips2
# --- AIX ---
#COPTIMIZE= -O3 -qstrict -qansialias
# --- AIX PWR2 ---
#COPTIMIZE= -O3 -qstrict -qansialias -qarch=pwrx -qtune=pwrx
# --- Linux ---
#COPTIMIZE= -pipe -O3 -fstrength-reduce -funroll-loops

# Default settings for configuration dependent options.
CFLAGS= -I. $(COPTIMIZE) $(CCOPTIONS)
LFLAGS= $(LDOPTIONS)
BINDIR=..

#  Definitions for libraries.
L=libjssubs.a
LSRC=	ErrCodes.c bigsort.c btreebuild.c btreecheck.c btreefile.c \
	btreefind.c btreeupdate.c bufffile.c jserror.c jsgetopt.c \
	jsperror.c jsstrerr.c 
LOBJ=	$L(ErrCodes.o) $L(bigsort.o) $L(btreebuild.o) $L(btreecheck.o) \
	$L(btreefile.o) $L(btreefind.o) $L(btreeupdate.o) \
	$L(bufffile.o) $L(jserror.o) $L(jsgetopt.o) $L(jsperror.o) \
	$L(jsstrerr.o) 

#  Definitions for program "testbtree"
OBJ1=	testbtree.o 
EXE1=	$(BINDIR)/testbtree

#  General user targets
all: $(EXE1)

clean:
	-rm *.o *.a

realclean: clean
	-rm $(EXE1)

#  How to make the executable(s)
$(EXE1): $(L) $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) $(L) $(LFLAGS) -o $@


# These rules should make the library and the library header files
#  and the error definitions if necessary. For the headers you'll
#  need my utilities makeproto, makedefs and makeerror.
$(L): $(LOBJ)

header:  jssubs.h

jssubs.h: $(LSRC) jssubs.in
	makeproto -i jssubs.in $(LSRC) | makedefs -i - $(LSRC) -o $@

errdefs: $(LSRC)
	makeerror -o ErrCodes.c -mM Error.txt $(LSRC)

# Some platforms need this rule. Remove comments if yacc
#  files get not compiled automatically.
#.y.a:
#	$(YACC) $(YFLAGS) $<
#	$(CC) -c $(CFLAGS) y.tab.c
#	rm y.tab.c
#	mv y.tab.o $*.o
#	$(AR) $(ARFLAGS) $@ $*.o
#	-rm -f $*.o
