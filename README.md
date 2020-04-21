### Basic information
Name: Ting Luo (Alicia)

NYU ID: tl2180


### List of files:
1. philo.c: codes to solve the "Dinner Plans" problem
2. Makefile: file containing the compiler directives
3. README.pdf: PDF file that reports on your work submission

### Explanation of the compilation rules:
```
philo: philo.c
	mpicc -Wall -o philo philo.c
```
The target of this compilation is `philo` and its prerequisite is file `philo.c`

The command to compile is `mpicc -Wall -o philo philo.c`.

The objective of the Makefile is to generate an executable file `philo`

The detailed list of source files you submitted in the directory (for each file, list the questions it provides a solution for)

### Comments on my submission
1. It works well for 2 philosophers. However, when N is larger then 2, sometimes there will be execution error in MPI_Recv.
2. The sorting feature is still incomplete.

### Questions & Answers
* Question 1:​ What​ c​ould​ h​appen​ ​then?​ ​Explain​ ​your​ a​nswer.
Deadlock could happen. As each of them has picked up the chopstick to their left, they are not able to pick up the chopstick to their right, becuase the right side chopstick has been occupied. Every philosopher has occuppied the left side chopstick and waits their right side chopsticks. There is a circle and deadlock happens.   


* Question 2:​ What​ type of clock do you need for this to work?​ ​Explain​ ​your​ a​ nswer.
Scalar clock works for this situation, because scalar clock can depict the total ordering of the system.