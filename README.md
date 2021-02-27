#The Shell

The purpose of the shell is to provide the user with commands to control the execution of the simulator.
The shell accepts one or more program les as command line arguments and loads them into the memory
image. In order to extract information from the simulator, a le named dumpsim will be created to hold
information requested from the simulator. The shell supports the following commands:

- go: simulate the program until it indicates that the simulator should halt. (As we dene below, this is
when a SWI instruction is executed with a value of 0x0A.)
- run <n>: simulate the execution of the machine for n instructions.
- mdump <low> <high>: dump the contents of memory, from location low to location high to the
screen and the dump file (dumpsim).
- rdump: dump the current instruction count, the contents of R0 - R14, R15 (PC), and the CPSR to
the screen and the dump file (dumpsim).
- input reg_num reg_val: set general purpose register reg_num to value reg_val.
- ?: print out a list of all shell commands.
- quit: quit the shell.