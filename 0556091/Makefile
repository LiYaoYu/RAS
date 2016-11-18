CC := gcc

# for gdb debug
CFLAGS = -Wall -g -O0

# for efficiency
#CFLAGS = -Wall -O3 

TARGET = main.c RAS.c RAS.h TCP.c TCP.h pipes.c pipes.h


server : $(TARGET) 
	$(CC) -o $@ $^ 
	cd ../testing_data && $(MAKE)
	cd ../commands && $(MAKE)

test : test.c RAS.c RAS.h pipes.c pipes.h
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -rf server 
	rm -rf test
	cd ../testing_data && make clean
	rm -f ../ras/*.txt
