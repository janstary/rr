PREFIX	= $(HOME)
BINDIR	= $(PREFIX)/bin
MANDIR	= $(PREFIX)/man/man1
CFLAGS	= -Wall -pedantic

all: rr
rr: rr.c rr.h
	$(CC) $(CFLAGS) -o rr rr.c

test: rr
	./rr zero.rm  < zero.in | diff - zero.out
	./rr copy.rm  < copy.in | diff - copy.out
	./rr succ.rm  < succ.in | diff - succ.out
	./rr add.rm   < add.in  | diff - add.out

lint: rr.1
	mandoc -Tlint -Wstyle rr.1

install: test rr.1
	install -d -m 755 $(BINDIR) && install -m 755 rr   $(BINDIR)
	install -d -m 755 $(MANDIR) && install -m 644 rr.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/rr
	rm -f $(MANDIR)/rr.1

clean:
	rm -f rr *.o *.core *~
