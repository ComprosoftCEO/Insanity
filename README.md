# Insanity
An esoteric programming language that consists of single symbol commands.

<br />

## About
Insanity is an [Esoteric Programming Language](https://en.wikipedia.org/wiki/Esoteric_programming_language) designed by Bryan McClain. Similar to assembly, Insanity uses single-symbol commands such as < + and $ to represent simple operations. Although Insanity should be thought of as a full-blown computer (with registers, memory, and a stack), this C program is an Insanity interpreter that simulates the computer. With this program, Insanity programs are compiled to bytecode, then run through the Insanity bytecode interpreter.

The Insanity interpreter is designed to use terminal colors when running the program. To make it cross-platform, several versions of the code exist:
* __Insanity.c__ - Runs on any platform and does no color output
* __Insanity-Windows.c__ - Designed to run on Windows. Color is handled through Window OS calls.
* __Insanity-Linux.c__ - Designed to run on Linux-based platforms. Color is handled by terminal escape codes.

<br />

## Compiling

### Compiling on Linux
Compile with the GCC compiler:
```
gcc Insanity_Linux.c -o Insanity.exe
```

## Compiling on Windows
1. Install [MinGW](http://www.mingw.org/) for Windows, which includes the GCC compiler and other necessary components.
2. Compile with the command:
```
gcc Insanity_Windows.c -o Insanity.exe
```

_Note: requires access to conio.h and windows.h_

<br />

## Usage
```
Insanity.exe <Program1> <Program2> ...
```
Insanity programs should be stored in text documents, and passed as parameters into the program. Insanity can run any number of programs, one after another.

<br/>
<br/>

## Insanity Programming Language
To read about writing programs for Insanity, please check out the official [language definition](Insanity.md).