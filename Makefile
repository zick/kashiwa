all: core/libcore.a builtin/libbuiltin.a

core/libcore.a:
	$(MAKE) -C core

builtin/libbuiltin.a:
	$(MAKE) -C builtin

clean:
	$(MAKE) -C core clean
	$(MAKE) -C builtin clean

