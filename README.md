# libmdr

This library is an implementation of the [MDR protocol](https://github.com/AndreasOlofsson/mdr-protocol) which is used by Sony headphones to change various settings.

Please note that this project is a work in progress and should not be expected to be very correct, complete or reliable.

The library is built to use the Bluez stack to connect to connect to the headphones and is not designed to be cross-platform.

See [mdrd](https://github.com/AndreasOlofsson/mdrd) for a daemon that exposes MDR-devices on the system D-Bus.

Contributions are very welcome, send a pull request if you've implemented some more of the protocol or have found a bug.
