# MIT 6.172 Assignment:<br/>Simulation and Rendering of Colliding Spheres

[![Zenodo](https://zenodo.org/badge/529455478.svg)](https://zenodo.org/badge/latestdoi/529455478)

This assignment was used as Project 2 for the MIT 6.172 (now 6.106) course
_Performance Engineering of Software Systems_ in the Fall semester of 2021.
The assignment was published as a Peachy Parallel Assignment in the [EduHPC-22
workshop][eduhpc-22].

This repo is separate from the course website to ensure that software materials
for the assignment are archived and publicly accessible.

[mit-6172-course-website-fall2021]: https://canvas.mit.edu/courses/11151/ 
[eduhpc-22]: https://tcpp.cs.gsu.edu/curriculum/?q=eduhpc22

| [**Link to the MIT 6.172 Fall 2021 course website**][mit-6172-course-website-fall2021] | <img src="logo-6172.png" alt="MIT 6.172 course logo" width="150"/> |
|:---------------------------------------------------------------------------------------|:-------------------------------------------------------------------|

## Contents of this repo

**Documents**

- `README.md` : This file
- `overview.pdf` : EduHPC-22 paper with high-level overview of the assignment
- `handout.pdf` : Assignment handout to students
- `INSTRUCTIONS.md` : Instructions to students

**Software**

- `Makefile` : Build system for the simulation program and related utilities
- `render.h`, `render.c` : Reference implementation for the rendering component
- `simulate.h`, `simulate.c` : Reference implementation for the simulation component
- `main.h`, `main.c` : Main program driver (calls into `render` and `simulate` components)

**Subdirectories**

- `simulations/` : Simulation input files for visualizations and debugging 
- `tiers/` : Simulation input files for tier performance evaluation
- `utils/` : Utilities for correctness and performance testing
