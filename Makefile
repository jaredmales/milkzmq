######################################################
## makefile for milkzmq                             ##
##                                                  ## 
## Edit BIN_PATH to specify installation directory. ##
######################################################

#This will change for both client and server
export BIN_PATH=/usr/local/bin


all:
	$(MAKE) -f makefile.server all
	$(MAKE) -f makefile.client all

install:
	$(MAKE) -f makefile.server install
	$(MAKE) -f makefile.client install

clean:
	$(MAKE) -f makefile.server clean
	$(MAKE) -f makefile.client clean
