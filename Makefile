
all:
	b2

clean:
	rd /s /q bin

distclean: clean
	rd /s /q dist

.PHONY: all clean distclean
