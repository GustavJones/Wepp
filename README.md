# Wepp

## Table of Content

1. [About](#about) 
2. [Building and Using](#building-and-using)
3. [Credits](#credits) 

## About
### Project
This is a simple HTTPS server written in C++. It uses OS specific sockets with my library GNetworking to create
a connection to a browser over TCP. HTTP parsing is handled through my library GParsing and TLS encryption is handled
by OpenSSL.

### Purpose
The purpose of this project is for me to learn more about networking and website creation from scratch by implementing
my own.

## Building and Using
### Prerequisites
- An OpenSSL installation
- C++ compiler (GCC tested on Linux and MSVC on Windows)
- CMake

For the OpenSSL libraries I recommend installing with a package manager on Linux and Windows. I recommend using 
Winget to install OpenSSL by installing `ShiningLight.OpenSSL`.

> [!NOTE]
> On Linux some package managers only ship Dynamic libraries for OpenSSL and manual compilation
> might be needed. This project uses static OpenSSL libraries instead of dynamic libraries.

If you are having trouble with OpenSSL not being found, set the `OPENSSL_ROOT_DIR` environment variable to your OpenSSL
root directory. If you are using `ShiningLight.OpenSSL`, set to `C:\Program Files\OpenSSL-Win64` or similiar directory.

### Configuring and Building

Run `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
followed by `cmake --build build --config Release -j` to build to the `build` directory.

### Usage
Copy executable to a working directory containing an SSL PEM key and certificate.
The key and certificate should be named `ssl.key.pem` and `ssl.crt.pem` respectively.
When run a `data` directory should be created where the server will be serving any containing files.

## Credits

This project uses `libopenssl` for TLS encryption. This repository does not include any source or binary distribution
of OpenSSL software but uses it's functions.

This project also uses `wxWidgets` for Wepp-Builder subproject. See `external/wxWidgets` for more information.
