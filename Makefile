CC=gcc
CFLAGS=-L./libs -I./include -Wall
BUILDDIR=build
BINDIR=$(BUILDDIR)/bin
OBJDIR=$(BUILDDIR)/objs
CSOURCEFILE=main.c
OBJECTS=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(notdir $(wildcard src/*.c)))))
LIBS=-lbassasio -lbass -lbass_fx -laubio-5 -lopengl32 -lGdi32

OUTFILE=$(BINDIR)/unnamedas.exe

.PHONY: clean

all: dirs unnamedas

unnamedas: $(OBJECTS) $(CSOURCEFILE)
	$(CC) $(CSOURCEFILE) $(OBJECTS) -o $(OUTFILE) $(CFLAGS) $(LIBS)
	
$(OBJDIR)/%.o: src/%.c include/%.h
	$(CC) -c $(filter %.c, $^) -o $@ $(CFLAGS) 
	
dirs: | $(BUILDDIR) $(BINDIR) $(OBJDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)
	
$(BINDIR):
	mkdir $(BINDIR)
	
$(OBJDIR):
	mkdir $(OBJDIR)
	
clean:
	-rm $(OBJECTS)
	-rm $(OUTFILE)