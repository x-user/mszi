CC = i686-w64-mingw32-gcc
RC = i686-w64-mingw32-windres

CFLAGS = -O2 -march=i686 -mtune=generic -pipe

all: clean mszi.exe hook.dll

%.o: %.c
	$(CC) -c $< -o $@

%.res: %.rc
	$(RC) -O coff $< -o $@

mszi.exe: main.o mszi.res
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32 -s

hook.dll: dllmain.o
	$(CC) $(CFLAGS) -Wl,--subsystem,windows $^ -o $@ -lkernel32 -lgdi32 -shared -s

clean:
	rm -vf *.{o,res,dll,exe}