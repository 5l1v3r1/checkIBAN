all: checkIBAN

bnsl.o: bnsl.h bnsl.c
	gcc -Wall -pipe -O2 -ggdb3 -c bnsl.c

checkIBAN: bnsl.o checkIBAN.c
	gcc -Wall -pipe -O2 -ggdb3 $@.c -o $@ bnsl.o

.PHONY: clean
clean:
	rm -rf *.o checkIBAN
