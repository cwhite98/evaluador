all: init delMem

LDFLAGS= -pthread #-lrt

init: init.o
	$(CXX) -o $@ $^ $(LDFLAGS)

delMem: delMem.o
	$(CXX) -o $@ $^ $(LDFLAGS)

init.o: init.cpp elementos.h init.h

delMem.o: delMem.cpp elementos.h


clean:
	rm -f *.o
	rm -f *.*~ *~
	rm -f init delMem
