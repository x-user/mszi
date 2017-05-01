CC = i686-w64-mingw32-gcc
RC = i686-w64-mingw32-windres

CFLAGS = -m32 -gdwarf-2 -gstrict-dwarf -march=i686 -mtune=generic -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-inline -fno-unit-at-a-time -pipe

all: injector.exe hook.dll hook_new.dll screenshot.exe

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.res: %.rc
	$(RC) -O coff $< -o $@

injector.exe: injector.o error.o mszi.res
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32

screenshot.exe: screenshot.o error.o mszi.res
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32

hook.dll: hook.o
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32 -shared

hook_new.dll: hook_new.o
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32 -shared

clean:
	rm -vf *.{o,res,exe,dll,bmp}
