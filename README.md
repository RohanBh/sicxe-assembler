# SIC/XE Assembler
A simple SIC/XE assembler written in cpp.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites
You need to have `cmake` and `make` installed on your system. 
- Install `cmake`:
	- Visit [cmake.org](https://cmake.org/download/) to download the latest binaries.
	- Make the script executable by running
	```bash
	chmod +x path/to/cmake/-3.x.x-Linux-x86_64.sh
	```
	- Run the script
	```bash
	sudo path/to/cmake/-3.x.x-Linux-x86_64.sh
	```
	- The script installs cmake in the /opt/ directory. Therefore, to add the `cmake` to one of directories in `$PATH`, run
	 ```bash
	 sudo ln -s /opt/cmake-3.x.x-Linux-x86_64/bin/* /usr/local/bin
	 ```

### Building
A step by step series of examples that tell you have to build a executable of the assembler yourself.
- Clone the repository.
- Run `cmake .` in the project's root directory.
- Run `make` in the project's root directory.
- That's it. You now have a executable named *sicxe_assembler* in the project's root directory.

## Running the Tests
https://en.wikipedia.org/wiki/Simplified_Instructional_Computer
