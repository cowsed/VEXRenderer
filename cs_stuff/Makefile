#
# Created by gmakemake (Ubuntu Jul 25 2014) on Tue Feb 28 14:39:07 2023
#

#
# Definitions
#

.SUFFIXES:
.SUFFIXES:	.a .o .c .C .cpp .s .S
.c.o:
		$(COMPILE.c) $<
.C.o:
		$(COMPILE.cc) $<
.cpp.o:
		$(COMPILE.cc) $<
.S.s:
		$(CPP) -o $*.s $<
.s.o:
		$(COMPILE.cc) $<
.c.a:
		$(COMPILE.c) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.C.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.cpp.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%

CC =		gcc
CXX =		g++

RM = rm -f
AR = ar
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
CPP = $(CPP) $(CPPFLAGS)
########## Flags from header.mak


CFLAGS =	-ggdb -std=c99 -Wall -Wextra -pedantic -Werror

CLIBFLAGS =	-lm


########## End of flags from header.mak


CPP_FILES =	
C_FILES =	VLC.c node_heap.c test_heap.c
PS_FILES =	
S_FILES =	
H_FILES =	node_heap.h
SOURCEFILES =	$(H_FILES) $(CPP_FILES) $(C_FILES) $(S_FILES)
.PRECIOUS:	$(SOURCEFILES)
OBJFILES =	node_heap.o 

#
# Main targets
#

all:	VLC test_heap 

VLC:	VLC.o $(OBJFILES)
	$(CC) $(CFLAGS) -o VLC VLC.o $(OBJFILES) $(CLIBFLAGS)

test_heap:	test_heap.o $(OBJFILES)
	$(CC) $(CFLAGS) -o test_heap test_heap.o $(OBJFILES) $(CLIBFLAGS)

#
# Dependencies
#

VLC.o:	node_heap.h
node_heap.o:	node_heap.h
test_heap.o:	node_heap.h

#
# Housekeeping
#

Archive:	archive.tgz

archive.tgz:	$(SOURCEFILES) Makefile
	tar cf - $(SOURCEFILES) Makefile | gzip > archive.tgz

clean:
	-/bin/rm -f $(OBJFILES) VLC.o test_heap.o core

realclean:        clean
	-/bin/rm -f VLC test_heap 
