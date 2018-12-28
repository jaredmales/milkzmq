
all:
	$(MAKE) -f makefile.server all
	$(MAKE) -f makefile.client all

clean:
	$(MAKE) -f makefile.server clean
	$(MAKE) -f makefile.client clean
