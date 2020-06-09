# Simplethon
Simplethon is a high-level, compiled programming language. It compiles into C++. As of the latest version (A-0.1), the binaries have only been generated for Windows devices, however the source code should be valid on unix machines as well.

## Installation
### Windows 10

First install C++ by downloading [MinGW](http://www.mingw.org/). Follow all the instructions in the installer.

To install Simplethon, download the .smpl folder for you respective machine (file hosting coming shortly).
Edit your environment variables by typing into the search bar "Environment Variables" and selecting the first result to come up.
Click on Environment Variables and click the New button under user variables. Name it SMPL_PATH and put the path to the .smpl folder as the value. For example, it may look like: `C:\Users\salipshitz\.smpl`.
Then, press OK. Double click on the user variable `Path`, and press New. Type in `%SMPL_PATH%` and press OK.

Test the installation by going into a command prompt window and typing `smpl -v` into the command prompt.