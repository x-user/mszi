CC = i686-w64-mingw32-gcc
RC = i686-w64-mingw32-windres

CFLAGS = -O1 -march=i686 -mtune=generic -gdwarf-2 -gstrict-dwarf -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-inline -fno-unit-at-a-time -pipe

all: clean injector.exe hook.dll screenshot.exe

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.res: %.rc
	$(RC) -O coff $< -o $@

injector.exe: injector.o mszi.res
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32

screenshot.exe: screenshot.o mszi.res
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32

hook.dll: hook.o
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32 -shared

clean:
	rm -vf *.{o,res,dll,exe}