include ../../../mhead

DEVNAME= device.so
all: $(DEVNAME) 
	
SRC= dev.c main.c DataProcess.c


TGT=$(SRC:.c=.o) ../../../txj/libio.o

$(SRC):types.h dev.h main.h DataProcess.h

	@touch $@
clean:
	-rm -f  *.gdb *.elf *.o *.so
	
%.o: %.c 

	$(CC) -c $?

$(DEVNAME): $(TGT)
	$(CC) -g -Wall -shared -o $@ $(TGT) -dl
	cp ./$(DEVNAME) ../../../bin/devices
