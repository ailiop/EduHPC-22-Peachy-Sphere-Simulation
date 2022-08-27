## Developer

- Isabel Rosa


## Instructions for Running the Binary:

Run 'make' as usual.

Run './main -h' to see your options.


## Instructions for Correctness Tester Tool:

Run 'make' as usual.

Run './main' with the '-m' flag. Running the main program should generate four
.txt files -- framesSimNew.txt, framesSimOld.txt, framesRenderOld.txt, and
framesRenderOld.txt.

Run './ref_test' in order to run correctness tests. Provide either the
render flag (-r) or the simulate flag (-s) as an argument to program, indicating
which function should be used to perform the tests. Exactly one of these options
must be given at runtime. You may also specify an optional graphics flag (-g) if
you wish to see a visual representation of your error.

In addition to a graphical heatmap showing which pixels are wrong in the output
of your new function as compared to the old function, the correctness tests will
print out several statistics regarding your error, including max error, min
error, and average error across all pixels in all frames. Please use this as a
measure of correctness.

The correctness tests currently assume that the original rendering function is
called renderOrig() and that the modified rendering function is called render().
Both functions are assumed to take the same input parameters. Similarly, the
tests assume that that the original simulation function is called simulateOrig()
and the new simulation function is called simulate(). Neither function should
take any input parameters.

When 'make clean' is run, all four text files containing image frames will be
removed.


## Instructions for Performance Testing:

Run 'make' as usual.

Run './main -t' to execute tiered performance testing.


## Instructions for Scalability Testing:

Run program with cilkscale by building with command 'make scale'.

To use the scalability benchmarking and visualization tool, compile the
necessary binaries with with commands 'make scale' and 'make bench'.


## File Overview:

Feel free to look around, but your performance grade will only depend on
changes to the following files:
- Files containing rendering code
	- render.c
	- render.h
- Files containing simulation code
	- simulate.c
	- simulate.h
- Makefile

All other files will be reverted for grading.
