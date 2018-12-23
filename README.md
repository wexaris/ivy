# ivy

This is the source code repository for the work-in-progress _ivy_ compiler.

## Usage

Using the compiler is very simple. All you need to do is pass it some input files.
An output location can also be specified via the option `-o <path>`.

```
ivy [options] <input>
```
> Currently there is no use for passing output locations, since the compiler does not yet
produce machine code.

You can also find other command line options in the help menu accessed via `-h`


## Building from Source

1. To build the compiler you will need the following dependencies:
    * `g++`
    * `cmake`
    * `make`
    * `git`

2. Start off by cloning the source code from this repository:
    ```
    git clone https://github.com/wexaris/ivy.git
    cd ivy
    ``` 

3. After that you can just build the compiler by running one of
    ```
    make
    make release
    make debug
    ```
    > The default build type is set to Release.

    The actual build system is CMake, but the Makefile is used as a shorthand for
    specifying build types, making directories and cleaning up.