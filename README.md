
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
