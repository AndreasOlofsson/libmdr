# libmdr

This library is an implementation of the [MDR protocol](https://github.com/AndreasOlofsson/mdr-protocol) which is used by Sony headphones to change various settings.

_Please note that this project is a work in progress and should not be expected to be very correct, complete or reliable._

This library is primarily designed to be used with the Bluez stack but it should be usable on any platform since it just uses libc and simple sockets.

See [mdrd](https://github.com/AndreasOlofsson/mdrd) for a daemon that exposes MDR-devices on the system D-Bus.

Contributions are very welcome, send a pull request if you've implemented some more of the protocol or have found a bug.

## Building

Run `make` in the project root.

### Dependencies

* make
* a C compiler (gcc is recommended)

