# milkzmq

A ZeroMQ-based server and client for ImageStreamIO image streams.

The server monitors an ImageStreamIO image stream on its host computer, and publishes new images as they appear.  A frames-per-second limit is applied to limit host computer and network resource use.

The client receives the images and updates an ImageStreamIO buffer on a remote computer.

## Building

Requirements:
 - c++23 compiler (tested on gcc 13.x)
 - ImageStreamIO
 - ZeroMQ

### Dependencies

#### 1. ImageStreamIO

You need the `milk` library, usually as part of `CACAO`.


#### 2. ZeroMQ

You need the *DRAFT API* of ZeroMQ: http://zeromq.org/intro:get-the-software, which means you can not use the version provided by default in Ubuntu or CentOS.  Follow the instructions below for your O/S. 


##### For Ubuntu 22.04:

Option 1: Compile yourelf.

```
$ wget https://github.com/zeromq/libzmq/releases/download/v4.3.4/zeromq-4.3.4.tar.gz
$ tar -xvzf zeromq-4.3.4.tar.gz
$ cd zeromq-4.3.4
$ ./configure --enable-drafts
$ make
$ sudo make install
```

Option 2: use the 20.04 packages.

As of 2022-06-18 the draft API version of the packages have not been released by 0MQ.  You can get the 20.04 packages to work, but you will need to do this manually.  This includes downloading libpgm from the 20.04 repo yourself.  You may also have to configure apt to use unsigned repos. 2022-09-03: this is still true.  The 22.04 release-draft repos do not have this library yet.   You will have to downgrade libzmq5, and use the 20.04 release-draft repos.
- First follow the instructions below to install the xUbuntu_20.04 repo and key.  Don't bother installing `libzmq3-dev` yet.
- Next execute the following:
```
sudo apt install libzmq5=4.3.4-0
sudo apt install libzmq3-dev=4.3.4-0
```
These packages are now downgraded.  

To prevent them from being upgrade, do this:
```
sudo apt-mark hold libzmq5
sudo apt-mark hold libzmq3-dev
```

##### For Ubuntu 20.04:
```
$ sudo su
$ echo "deb https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-draft/xUbuntu_20.04/ ./" >> /etc/apt/sources.list
$ wget https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-draft/xUbuntu_20.04/Release.key -O- | sudo apt-key add
$ apt-get install libzmq3-dev
```

##### For Ubuntu 18.04:
```
$ sudo su
$ echo "deb https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-draft/xUbuntu_18.04/ ./" >> /etc/apt/sources.list
$ wget https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-draft/xUbuntu_18.04/Release.key -O- | sudo apt-key add
$ apt-get install libzmq3-dev
```

##### On CentOS 7:
```
$ sudo su
$ yum-config-manager --add-repo https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-draft/CentOS_7/network:messaging:zeromq:release-draft.repo
$ yum install -y zeromq-devel libzmq5
```

#### 3. cppzmq
We also need the c++ bindings for ZeroMQ from the cppzmq project.  It is a simple header-only library.  To get it, you can do this:
```
$ git clone https://github.com/zeromq/cppzmq.git
$ cd cppzmq/
$ sudo cp *.hpp /usr/local/include/
```

#### 4. xrif

Install libxrif from here: https://github.com/jaredmales/xrif

### Configuration

You may wish to edit the `BIN_PATH` in the makefiles to control where executables are installed.

### Building
A simple `make` in this directory should build both server and client.

## Running

Suppose you have a process writing images to `/tmp/image00.im.shm` on `myserver.myschool.edu`.  To run a server which will publish these images at 5 f.p.s. you would execute:
```
milkzmqServer -f 5.0 image00
```

Then on the remote machine you would execute
```
milkzmqClient myserver.myschool.edu image00
```
Now on this remote machine an `ImageStreamIO` image will be available at `/tmp/imtest00.im.shm`, with updates at 5.0 Hz.

Several parameters can be set for each program.  The following shows the output of the online `-h` help.

### milkzmqServer
How to run the server.  To get help:
```
$ ./milkzmqServer -h
```
will output
```
./milkzmqServer 

usage: ./milkzmqServer [options] shm-name [shm-names]

   shm-name is the root of the ImageStreamIO shared memory image file(s).
            If the full path is "/tmp/image00.im.shm" then shm-name=image00
            At least one must be specified.
options:
    -h    print this message and exit.
    -p    specify the port number of the server [default = 5556].
    -u    specify the loop sleep time in usecs [default = 1000].
    -f    specify the F.P.S. target [default = 10.0].
    -x    turn on compression for INT16 and UINT16 types [default is off].
    -a    If no shm-names are listed, export all from MILK_SHM_DIR.
```

### milkzmqClient
How to run the client.  To get help:
```
$ ./milkzmqClient -h
```
will output
```
./milkzmqClient: 

usage: ./milkzmqClient [options] remote-host shm-name [shm-names]

   remote-host is the address of the remote host where milkzmqServer is running.

   shm-name is the root of the ImageStreamIO shared memory image file.
            If the full path is "/tmp/image00.im.shm" then shm-name=image00
            At least one shm-name must be specified.
            To specify a different local name, use a /.  Example: "image00/local_image00"
            will stream the remote image00 locally as local_image00.
options:
    -h    print this message and exit.
    -p    specify the port number of the server [default = 5556].

```
