# bish

Shell scripting with a modern feel.

## What

Bish is a lightweight language created to bring shell scripting into the 21st century. It gives programmers the comfort of modern syntax but compiles to Bash, resulting in good portability (in as much as Bash is portable).

Currently, Bish is still a very new language. I am adding new features as fast as I can, but it is bound not to support something you might like. In that case, see if you can implement it and submit a pull request!

As a side note, I won't stop you, but you would probably be crazy to use Bish in any sort of production environment until it stabilizes a bit more.

Some small examples are below, but **see the [introduction](https://github.com/tdenniston/bish/wiki) wiki page for a more comprehensive introduction**.

## Examples

##### Function calls

    def fib(n) {
        if (n < 2) {
            return 1
        }
        return fib(n-1) + fib(n-2)
    }

##### Shell commands

    def printall(files) {
        for (f in files) {
            println(f)
        }
    }
    # cwd, ls, and cd are all builtin functions.
    dir = cwd()
    files = ls()
    println("Files in current directory $dir:")
    printall(files)
    cd("/")
    println("Files in root directory:")
    printall(ls());

##### Shell-like features

    # I/O redirection (like pipes) is a first-class language feature.
    pattern = "bish"
    matches = ls() | @(grep $pattern)
    println("Files in current directory matching $pattern:")
    for (m in matches) {
        println(m)
    }

## How

Just clone and build with 'make'. There are no dependencies other than the C++ standard library. Compile bish to bash with the command:

    $ ./bish input.bish > output.bash
    
Then run the compiled file:

    $ bash output.bash

## Why

I can't count the number of times when I wanted to write a quick shell script to automate an easy task, only to waste hours tracking down idiosyncrasies in Bash syntax and semantics. Bish tries to fill this niche: when you want a lightweight shell scripting language and don't wish to break out the larger hammer of Python, Perl, etc...

However, I created this language first and foremost as a weekend project and exercise in writing a compiler from scratch. I've dealt with pieces of compilers before, but have never written one from start to finish. Thus, I hope this project can also serve as a good teaching tool. I tried to design it with clarity in mind, so it should be simple to extend the language, or write a new back end.

## To do

The language and compiler are pretty bare-bones as of now, but quite functional. There are numerous things in the pipeline (see the open issues for a list). Longer term, here are some ideas of features that might be implemented:

* Additional compiler targets, e.g. `sh` for even wider compatability, or `ksh` for vanilla AIX support.
* Multidimensional arrays, and potentially arrays of mixed types.
* Syntactical sugar for more shell-like function invocation. For example, typing `cd("path")` or `ls("-l")` might get old when writing bish scripts. Bish could support an alternate invocation syntax of `function arg0 arg1...` so that these functions could be invoked like `cd path` or `ls -l` directly in the bish script.
* Language support for executing portions of bish scripts as other users (e.g. the administrator).
* Language support for executing commands on remote machines.
