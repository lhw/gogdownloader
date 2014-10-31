Not actively maintained
================================
There is another implementation of the API by sude https://github.com/Sude-/lgogdownloader which should be working
unlike this one.

Building Instructions
================================

Currently the application can be compiled via cmake and make like this:

    cmake .
    make
    
Which will produce a single binary called `goglogin` in src/
On the most recent debian and ubuntu versions the build might fail because
it cannot find libprotobuf-c. The maintainer does not provide the package
with a pkg-config file.
I currently have an updated version in my personal package archive (PPA) at
https://launchpad.net/~lhw/+archive/packages or if you intend to you can also
compile your own from source.
