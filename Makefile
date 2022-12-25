CC=g++
CXXFLAGS=-std=c++11 -c -Wall -Wextra -Wshadow -Wredundant-decls -Wunreachable-code -Winline
INCLUDES=-I/usr/include/blkid
LDFLAGS=-lblkid -ludev

ifeq ($(DEBUG),1)
CXXFLAGS+=-g
endif

OBJ:=usbdrive.o main.o
EXE=demo

COMPILE.1=$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $<
ifeq ($(VERBOSE),)
COMPILE=@printf "  > compiling %s\n" $(<F) && $(COMPILE.1)
else
COMPILE=$(COMPILE.1)
endif

%.o: %.cpp
	$(COMPILE)

.PHONY: all clean rebuild

all: $(EXE)

$(EXE): $(OBJ) $(OUTPUT_DIR)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJ)

rebuild: clean all
