#TARGET=i386-mingw32- #for mac
CC=$(TARGET)gcc
LD=$(TARGET)gcc
WINDRES=$(TARGET)windres

CFLAGS=-DWINVER=0x500 -DWIN32_WINNT=0x500 -Os -DWITH_ICON=1 
LDFLAGS=-mwindows

PROG=neoplace.exe
OBJS=main.o resources.o trayicon.o


ifdef DEBUG
				CFLAGS+= -g
				LDFLAGS:=$(filter-out -mwindows, $(LDFLAGS))
endif

all: ${PROG}

clean:
				rm $(OBJS) $(PROG) 

%.o: %.rc
				$(WINDRES) -i $^ -o $@

%.o: %.c
				$(CC) $(CFLAGS) -c -o $@ $^

$(PROG): $(OBJS)
				$(LD) $(LDFLAGS) -o $@ $^
