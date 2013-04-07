SRC:=source

PROG:=parcon

vpath %.c $(SRC)
vpath %.h $(SRC)

$(PROG): parcon.c parcon.h
	$(CC) -o $(PROG) $(SRC)/*.c -Os -Wall

clean:
	rm $(PROG)

install: $(PROG)
	cp $(PROG) /usr/bin/

remove:
	rm /usr/bin/$(PROG)

./PHONY: clean install remove