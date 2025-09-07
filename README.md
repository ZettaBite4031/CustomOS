# zOS
## A light weight, custom-built operating system written just to learn more ;P

<p align="center">
<img src="https://img.shields.io/badge/Version-v0.0.1a-red" alt="Version 0.0.1 ALPHA">
<img src="https://img.shields.io/badge/Last Commit-September 4th-green" alt="Most recent commit was September 4th, 2025">
<img src="https://img.shields.io/badge/License-MIT-blue" alt="MIT license!">
<img src="https://img.shields.io/badge/Lines-8800+-green" alt="Currently over 8,800 lines of code!">
</p>

---
### Table of Contents
1. [Overview](#overview)
2. [Repository Structure](#repository-structure)  
3. [Prerequisites](#prerequisites)  
4. [Building zOS](#building-zos)  
5. [Booting & Running](#booting--running)  
6. [Development Workflow](#development-workflow)  
7. [Contributing](#contributing)  
8. [Project Goals & Features](#project-goals--features)  
9. [License](#license)  
10. [Contacts & Support](#contacts--support)
---
## Overview
zOS was mainly just a toy project for me to screw around with a few years back, and lately I've been picking at it on my own, so why not make it a little more public so people can see what I've been up to, and hey if you wanted to add something, that's alwys open.
zOS not only serves as a time killer, it's a sandbox for:
- Learning OS Fundamantals: Bootloaders, File Systems, Memory Mapped IO, and other hardware interactions
- Experimenting and discovering custom toolchains and build pipelines
- Building, debugging, and troubleshooting multi-language massive project environments

## Repo Structure
CustomOS/\
├── build_scripts/\
│&emsp;&emsp;Python and shell scripts to assist with building the OS. i.e. generating ISRS\
├── image/\
│&emsp;&emsp;The structure of the FAT file system that is being put into the image.\
├── scripts/\
│&emsp;&emsp;Helper scripts for debugging, inspecting, running, or setting up the toolchain\
├── src/\
│&emsp;&emsp;├ bootloader/\
│&emsp;&emsp;│&emsp;&emsp;├ stage1 \
│&emsp;&emsp;│&emsp;&emsp;│ &emsp;&emsp; The initial bootloader. 512 bytes for the bios to launch, which finds and executes stage2.\
│&emsp;&emsp;│&emsp;&emsp;└ stage2 \
│&emsp;&emsp;│&emsp;&emsp;&emsp;&emsp;&ensp; The secondary bootloader which is launched by stage1. It must be split due to the 512-byte size constraint of legacy bios.\
│&emsp;&emsp;├ kernel/ \
│&emsp;&emsp;&emsp; zOS kernel. Where all the magic happens.\
│&emsp;&emsp;└ libs/ \
│&emsp;&emsp;&emsp; This is where most of the kernel code is, but is separate due to a lot of it eventually being used in stage2. (File system, i686 stuff, cpp stdlib implementations)\
└── SConstruct\
&emsp;&emsp;&ensp; Build configuration file for SCons\

## Prerequisites
The `./scripts/install_deps.sh` script should install all dependencies, but in case\
 you want to explore zOS, ensure your environment includes:
- **Python 3.x** for the SCons scripts
- **Python Environment Manager** i.e. Anaconda3
- **SCons** for actually building zOS

Sometimes installing python packages through apt (apt install python-\<package>) doesn't work. This seems to only be the issue for scons, so install it through pip, and everything should work. 
Sometimes, libguestfs is an issue as well, so make sure the image it's using in /boot is able to be read, or you can just swap to using mount in `./build_scripts/config.py`.\
I'd just give out a requirements.txt, but my environment is so polluted from accidentally using it elsewhere, I don't want to do the same with whoever tries using this.

## Building zOS
It's pretty simple to install.\
First create your environment. I use Anaconda.
Then clone
```
conda create -n OSDev
conda activate OSDev
git clone https://github.com/ZettaBite4031/CustomOS.git
cd CustomOS
sudo bash ./scripts/install_deps.sh (it might fail to install scons here)
```
Next we need a toolchain. If you already have one, make sure you specify its path in `./build_scripts/config.py`, and if not, that's where the `./scripts/setup_toolchain.sh` is involved. Currently, it's in a broken state, and NEWLIB is **not** needed, so you can ignore it, and copy paste commands in. Stupid, I know, but I'm new to this stuff.

## Booting & Running
Once everything is installed and setup, though, all you need to do is just `scons run` and you should see zOS appear on your screen.\
 It will look like it freezes, but at the moment, it's just waiting for UDP messages, so connect to localhost:6001 with any UDP sender, and you should see it echo back your messages! You can use `quit` to exit the receive loop. 

Now you're all setup to mess around with it! I'll happily accept any contributions.

## Development Workflow
1. **Develop**: Edit kernel, driver, or bootloader code.
2. **Build**: Run `scons` to build, or `scons run` to build & run.
3. **Test**: Did it explode and die? 
4. **Debug**: Debugging is quite simple, either use VSCode's built in debugger, or GDB with the `debug.sh` and `debug_stage2.sh` scripts in `./scripts/`. 
5. **Rinse and repeat**: Refactor, recompile, retest until you've made it. 
### Script Automation
A lot of things that needs to be done is automatically handled in `./build_scripts/` or `./scripts/`, so make sure to look through them if you're ever curious, and if something is broken (as `setup_toolchain.sh` is) try fixing it. 
### Contributing
1. Fork CustomOS
2. Clone your fork:
```
git clone https://github.com/You/CustomOS.git
cd CustomOS
```
3. Create a new branch:
```
git checkout -b feature/my-feature
```
4. Go through the [Development Workflow](#development-workflow)  
5. Commit with descriptive messages:
```
feat(bootloader): Added UEFI support
fix(kernel): Network stack corruption
```
7. Push and open a Pull Request for review!

## Project Goals & Features
A few features and goals that could be added by myself or you! <sub>no guarantee anything here actually gets added</sub>
### Features
- It's an educations OS! To show how computers actually work.
- Fully modular components! Currently, nothing is intertwined, and if you wanted to swap one part out for another, you could!
- Rapid iteration setup! SCons-driven builds allow for instant feedback 
### Goals
- [ ] Memory Virtualization
    - [ ] SWAP memory
    - [x] MMU alive and worked
- [ ] Keyboard Interface
- [ ] Proper disk read/write for file system interactions
    - [ ] FAT
    - [ ] EXT
    - [ ] NTFS
- [ ] Proper ELF loading for programs and dynamic libraries
- [ ] Full network support
    - [x] UDP
    - [ ] TCP
    - [ ] Socket API
- [ ] Drivers for modern hardware
    - [ ] ACHI / NVMe 
- [ ] Graphics
- [ ] Shell
- [ ] Multi-core processing
- [ ] User/Kernel mode separation
    - [ ] An actual User-Space
- [ ] Syscall interface
- [ ] Scheduler
- [ ] Inter-Process Communication (pipes, message queues, shared memory)

This isn't complete, and there's plenty else to be done, and some of these are MASSIVE projects in-of-their-own, but that's why they're goals, so there's plenty to be done.
## License
```
MIT License

Copyright (c) 2025 Zettabite Pragmara 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
