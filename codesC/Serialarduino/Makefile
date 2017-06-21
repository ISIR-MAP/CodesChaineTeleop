BIN_DIR = bin/Debug/
IDIR =include
LDIR =lib
CPPFLAGS += -fno-exceptions -static
LDLIBS =
build: main.o serial.o
	g++ main.o serial.o -o $(BIN_DIR)ressort
main.o: main.cpp
	g++ $(CPPFLAGS) -c main.cpp $(RDIR) -I$(IDIR)
serial.o: Serial.cpp
	g++ -c Serial.cpp $(RDIR) -I$(IDIR)