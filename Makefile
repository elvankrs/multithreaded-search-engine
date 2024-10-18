.PHONY:  all hellomake generatefile
all: abstractor.out


abstractor.out: abstractor.cpp
	g++ abstractor.cpp -o abstractor.out -lpthread

generatefile:
	touch empty_file.txt

clean:
	rm *.txt
	rm -f *.o