# Vms, Version Control Made Simple

Vms is a version control system designed for simplicity, inspired by Git.

Some of the features Vms has adopted from Git include:

- **a branching and merging model** which allows users to create and manage independent branches that may be used for logically separating, carrying out, and experimenting with different lines of development
- **localized operations** which offer speed advantages relative to centralized systems that must constantly communicate with a remote server.
- **localized repositories** that contain the entire commit history of a project, enabling possible support in the future for a variety of distributed and centralized workflows
- **data integrity and assurance** by comparing every file retrieved from the repository with the cryptographic hash of the file’s contents at the time it was committed— in this way, serving as a checksum, ensuring archived data cannot be corrupted or altered without the system knowing
- **an intermediate staging area** where users may review and manage changes before committing them to the repository

For a quick look at its functionality, see the [Tutorial](docs/TUTORIAL.md).

To see all of the commands it supports, see the [Command Specification](docs/COMMANDS.md).

Vms was implemented with the goal of maintaining many of the design philosophies, structures, and features that make Git a powerful version control system while also providing a simpler interface. It features a reduced set of commands and options, instead focusing on the use of easy-to-understand primitives to accomplish version control tasks. It was also designed to abstract lower level structures like the HEAD pointer, exhibit intuitive behavior and syntax, and provide a safe, fast, and robust experience.

This project was not developed to be a replacement for Git. At its core, it is a personal learning project to develop a deeper understanding of the decisions and challenges underlying the design of a software system like Git.

Nonetheless, I hope that the end product will contribute ideas for making version control more accessible to users of diverse backgrounds, whether by serving as a simple alternative to some of the more complex version control systems or a stepping stone for learning the basics of version control.

![Demo](images/vms-demo-compressed.gif)

## Getting started

Before using Vms, please note that this is an early release primarily intended for demonstration and trial purposes. At this stage, I do not recommend using Vms as your primary mode of source control for any professional or otherwise important work.

### Platforms

- macOS

### Prerequisites

- A C++11 Compiler (e.g. g++ or clang++)
- GNU Make
- Boost 1.67.0 (or later)

If you are on macOS and you have the Xcode Command Line Tools, you should already have a compatible C++ compiler and make utility. If you don't, then you can install them by opening the Terminal application and typing the command:

```bash
xcode-select --install
```

The easiest way to get set up with [Boost](https://www.boost.org) on macOS is with the [Homebrew](https://brew.sh) package manager. If you do not have Homebrew already, you can install it by opening the Terminal application and pasting into the prompt:

```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)
```

Once you have Homebrew installed, you can install Boost by typing into your Terminal the command:

```bash
brew install boost
```

Once the installation is complete, you can verify the Boost version with `brew info boost`.

With that, the preparation should be complete. If you have git installed, you may clone the source by moving with `cd` into the location you wish to store the application and inputting the command:

```bash
git clone https://github.com/davidkml/vc-made-simple.git
```

Finally, move into the cloned directory and compile the source:

```bash
cd vc-made-simple
make
```

If no errors occurred and compilation was successful, then you are finished.

## How to use it

**For those who only want to demo the application**, I have provided a simple bash script that will temporarily add the directory to your \$PATH variable and move you into a new subdirectory called `sandbox` in which you can add files, modify them, and experiment with the application for the duration of Terminal session. To do this, move into the `vc-made-simple` directory and run:

```bash
source init_sandbox
```

Every time you use the script, your previous directory will be deleted so you can start fresh, so **be careful to not leave any important files in the** `sandbox` **directory!**

**Alternatively**, you may more permanently add the application to your \$PATH by adding to your `~/.bash_profile` file the line:

```bash
export PATH=/path/to/vc-made-simple/bin:$PATH
```

where `/path/to/vc-made-simple/` should be replaced with the actual path to the directory in your file system. Then, to effect the change, run:

```bash
source ~/.bash_profile
```

**To get started quickly or walk through a demonstration of how to use Vms, take a look at the [Tutorial](docs/TUTORIAL.md).**

### Commands

Below is a reference list of commands and a brief description of their purpose. **For more detailed specifications on their behavior, please see the [command specification](docs/COMMANDS.md).**

- `vms init`: Create an empty Vms repository.

- `vms status`: Display the status of the working tree.

- `vms stage [<filenames>] [<dirnames>]`: Add snapshots of the given files to the staging area.

- `vms unstage [<filenames>] [<dirnames>]`: Remove the snapshot associated with the given files from the staging area.

- `vms commit <message>`: Save the snapshots of the files in the staging area into the repository with an associated message and add a new commit to the history

- `vms log`: Display a chronological log of your commit history.

- `vms checkout files <commitid> [<filenames>]`: Restore the version of all (or optionally, only the given) files as they exist in the commit corresponding to the given id, overwriting the versions in your current working directory, if they exist.

- `vms checkout branch <branchname>`: Switch branches and update files in your current working directory with the versions as they exist in the given branch.

- `vms mkbranch <branchname> [commitid]`: Create a new branch at the same location of your current branch (or optionally, at the location of the commit corresponding to the given id).

- `vms rmbranch <branchname>`: Remove the branch with the given name.

- `vms info <commitid> [file]`: Display information for the commit corresponding to the given id (or optionally, the contents of the given file as they exist in that commit).

- `vms merge <branchname>`: Merge files from the given branch into the current branch, joining two development histories together.

## Future Roadmap

There is still a much to be done before this application can be practically used. In addition to small improvements to the existing codebase, some major goals include:

- a framework or strategy for automated testing
- simpler installation procedure and resolution of dependencies
- portability to other platforms
- benchmarking and optimization for speed, memory, and scale
- extensions for remote features

## Testing

A majority of the testing to this point has been manual. The frequent changes to the user interface, outputs, and file system make it so that, by nature, the application is challenging and laborious to author complete automated tests for. Moreover, manual testing has the advantage of providing opportunities to obtain insight into the user experience, yielding information with which to derive additional tests and features.

The testing of this application in the prototyping stage has been guided by a high level test plan and domain knowledge of desired behavior to assist in testing completeness. However, as the project continues to develop, I expect there will be a growing need for automated tests.

## Contributing

I would love to have some help! Pull requests, feedback, bug reports, and ideas are welcome and appreciated.

For any changes, please open an issue first to discuss what you would like to change. More formal documentation and guidelines for contributions is a work in progress.

## Authors and acknowledgements

David Lee - _Author_ - [davidkml](https://github.com/davidkml)

## License

Vms is licensed under the terms of the MIT License and is available for free.
