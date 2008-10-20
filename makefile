SRCDIR = source
OBJDIR = bin
BINDIR = bin

CFLAGS = /openmp /W4 -D_CRT_SECURE_NO_WARNINGS /nologo /Zi
CC = @cl

PARTS = \
	$(OBJDIR)\disk.obj \
	$(OBJDIR)\disk_internals.obj \
	$(OBJDIR)\parser.obj \
	$(OBJDIR)\memory_index_hash_node.obj\
	$(OBJDIR)\memory.obj \
	$(OBJDIR)\memory_index.obj \
	$(OBJDIR)\postings_piece.obj

{$(SRCDIR)\}.c{$(OBJDIR)\}.obj:
	$(CC) $(CFLAGS) /c /Tp $< /Fo$@

all : $(BINDIR)\main.exe $(BINDIR)\index.exe

$(BINDIR)\main.exe : $(PARTS) $(OBJDIR)\main.obj
	$(CC) $(CFLAGS) $(OBJDIR)\main.obj $(PARTS) /Fe$@

$(BINDIR)\index.exe : $(PARTS) $(OBJDIR)\index.obj
	$(CC) $(CFLAGS) $(OBJDIR)\index.obj $(PARTS) /Fe$@

$(OBJDIR)\index.obj : $(SRCDIR)\index.c

$(OBJDIR)\main.obj : $(SRCDIR)\main.c

$(OBJDIR)\disk_internals.obj : $(SRCDIR)\disk_internals.c $(SRCDIR)\disk_internals.h

$(OBJDIR)\disk.obj : $(SRCDIR)\disk.c  $(SRCDIR)\disk.h $(SRCDIR)\disk_internals.h

$(OBJDIR)\parse.obj : $(SRCDIR)\parse.c $(SRCDIR)\parse.h $(SRCDIR)\string_pair.h

clean :
	del $(OBJDIR)\*.obj $(BINDIR)\*.exe
