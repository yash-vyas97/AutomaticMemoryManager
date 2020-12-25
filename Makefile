# For my makefile to work you must have 3 folders/directories
# The names are
# include , source , and object
#The Makefile should be the fourth subfile including the three above

INCDIR = ./include
SRCDIR = ./source

CC = clang++
CFLAGS = -Wall -g -I$(INCDIR)

PROG = object/objectExecutables
HDRS = $(INCDIR)/ObjectManager.h $(INCDIR)/Object.h
SRCS = $(SRCDIR)/main.c $(SRCDIR)/ObjectManager.c

OBJS = object/ObjectManager.o object/main.o 

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)
  
object/ObjectManager.o: $(SRCDIR)/ObjectManager.c $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCDIR)/ObjectManager.c -o object/ObjectManager.o

object/main.o: $(SRCDIR)/main.c  $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o object/main.o
  
clean:
	rm -f $(PROG) $(OBJS)
