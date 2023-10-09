[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/wtw9xmrw)

#COP4610 Project 1 Group 20

Building a shell.

## Table of Contents

-[Project 1]
	-[Group Members]
	-[How to Use the Makefile]
	-[File List]
	-[Division of Labor]
	-[Completed extra credit]
	-[Possible bugs]

## Group Members

Sophia Elliott, Ivan Quinones, & Christopher Lindner

## How to Use the Makefile

User must ensure that they are in the main 'project-1-group-20' folder. Type either 'make run'
or './bin/shell' to run and start the program.

## File List

We solely used 'shell.c' for all of the functionality of our shell. In addition,
we used the provided 'lexer.c' and 'lexer.h' files for provided funtionality.

## Division of Labor

All groups members worked collaboratively on all parts of this project.

## Completed Extra Credit

None.

## Possible bugs

When using I/O direction, only one '<' or '>' can be used at a time. When using piping,
a singular pipe can run successfully, but once ran again it will buffer. Two pipes causes
the program to buffer.
