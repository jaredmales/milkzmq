######################################################
## Makefile for milkzmq.                            ##
######################################################

ifeq ($(wildcard /opt/magaox),)
export BIN_PATH	:= /usr/local/bin
export INC_PATH	:= /usr/local/include
export LIB_PATH	:= /usr/local/lib
else
export BIN_PATH	:= /opt/magaox/bin
export INC_PATH	:= /opt/magaox/include
export LIB_PATH	:= /opt/magaox/lib
endif

all:
	$(MAKE) -f makefile.server all
	$(MAKE) -f makefile.client all

install:
	$(MAKE) -f makefile.server install
	$(MAKE) -f makefile.client install

clean:
	$(MAKE) -f makefile.server clean
	$(MAKE) -f makefile.client clean
