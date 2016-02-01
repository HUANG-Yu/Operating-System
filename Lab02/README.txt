############################################################################
 Yu HUANG
 N18447876 
 NetID: yh1456
 
 Lab02 Scheduler (C++ version)
 03.09.15
############################################################################
0. Link to CIMS
Source code runs on energon1 and using G++48 compiler

1. Compilation command
Unzip the archive. Use cd enter the relevant folder and run the following command.

	$ g++ main.cpp -o main	// compiles the source code

2. Running Program
Run the following command:

	$ chmod 777 runit.sh     // given permission
	$ chmod 777 gradeit.sh	// given permission
	$ ./runit.sh <generate_ouput_folder> ./main		// execute all the input
	$ ./diffit.sh <given_output_folder> <generate_ouput_folder>	//calculate score