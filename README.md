# milk0mq

A ZeroMQ-based server and client for ImageStreamIO image streams.

The server monitors an ImageStreamIO image stream on its host computer, and publishes new images as they appear.  A frames-per-second limit is applied to limit host computer and network resource use.

The client receives the images and updates an ImageStreamIO buffer on a remote computer.

## Building

Requirements:
 - c++14 compiler (tested on gcc)
 - ImageStreamIO
 - ZeroMQ

### Dependencies

#### ImageStreamIO

You need the `milk` library, usually as part of `CACAO`.


#### ZeroMQ

You need ZeroMQ: http://zeromq.org/intro:get-the-software, download package for your arch

For Ubuntu 18.04:
```
$ sudo su
$ echo "deb https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/ ./" >> /etc/apt/sources.list
$ wget https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/Release.key -O- | sudo apt-key add
$ apt-get install libzmq3-dev
```

To test your install get the examples:

```
git clone --depth=1 https://github.com/imatix/zguide.git
```

### Configuration

You may wish to edit the `BIN_PATH` in the makefiles to control where executables are installed.

### Building
A simple `make` in this directory should build both server and client.

## Running

Suppose you have a process writing images to `/tmp/image00.im.shm` on `myserver.myschool.edu`.  To run a server which will publish these images at 5 f.p.s. you would execute:
```
milk0Server -f 5.0 image00
```

Then on the remote machine you would execute
```
milk0Client myserver.myschool.edu image00
```
Now on this remote machine an `ImageStreamIO` image will be available at `/tmp/imtest00.im.shm`, with updates at 5.0 Hz.

Several parameters can be set for each program.  The following shows the output of the online `-h` help.

### milk0Server

```
$ milk0Server -h
milk0Server:

usage: milk0Server [options] shm-name

   shm-name is the root of the ImageStreamIO shared memory image file.
            If the full path is "/tmp/image00.im.shm" then shm-name=image00
options:
    -h    print this message and exit.
    -p    specify the port number of the server [default = 5556].
    -u    specify the loop sleep time in usecs [default = 100].
    -f    specify the F.P.S. target [default = 30.0].
    -s    specify the semaphore number [default=0].
```

### milk0Client
```
$ milk0Client -h
milk0Client:

usage: ./milk0Client [options] remote-host shm-name

   remote-host is the address of the remote host where milk0Server is running.

   shm-name is the root of the ImageStreamIO shared memory image file.
            If the full path is "/tmp/image00.im.shm" then shm-name=image00
options:
    -h    print this message and exit.
    -p    specify the port number of the server [default = 5556].
    -l    specify the local shared memory file name [default is same as shm-name].
```
