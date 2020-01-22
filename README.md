# FaceGen Base Library

Copyright (c) 2020 Singular Inversions Inc.

A simple cross-platform C++11 library for developing computational 3D graphics applications.

## Features

* 3D mesh IO, manipulation, hardware and software rendering.
* Imaging IO, manipulation
* Linear algebra with solvers
* Ultra-simple declarative GUI (currently only implemented for Windows)
* Cross platform build file construction tool.
* Command-line interface tools with pretty-print.
* Unicode, filesystem, diagnostics, data packaging, etc.
* Support for VS2015,2017,2019 on Win, gcc & clang on Ubuntu and macOS. Warning-free compiles on -Wall.

## Design

* Emphasis on clear, correct code rather than premature optimization.
* Functional and declarative design in as much as makes sense for C++.
  * Extensive use of return value elision.
  * Declarative GUI layout, declarative dataflow graph for computations.
* Minimal OOP-fuscation; Some member functions and constructors, few private members, no impl inheritance.
* Templates are used when they provide clear benefit; Avoid complex template metaprogramming.
* Exceptions and RAII used throughout.
* Compiles out of the box; No dependencies, install scripts or special make packages required.
* Different compiler / configuration object files are kept in different directories for fast switching.
* Conventions: camelCase, K&R brackets, why-not-what comments.
* namespace Fg { varNoun, funcVerb, cNoun short for calcNoun, ClassName, FG_MACRO }
