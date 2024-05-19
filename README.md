# $N$ Puzzle Solver
Author: [Jack Robbins](https://www.github.com/jackr276)

## Introduction
The $N$ Puzzle problem is on the surface a simple problem to solve. The problem is as follows:
> Given an initial configuration of an $N\times N$ grid of tiles, each labeled with distinct numbers from $0$ to $N^2 - 1$, determine the *smallest* sequence of single step moves that can be made with the $0$ tile such that the puzzle is in numerical *row-major* order, with the 0 slider in the $N-1$ row and $N-1$ column.

The definition can be a bit hard to parse without an example, so let's look at an example starting configuration and goal configuration.

#### Sample $4 \times 4$ starting configuration
| 1 | 7 | 15 | 4 |
|--|---|---|--|
|**0**|**6**|**3**|**8**|  
|**2**|**5**|**14**|**11**|
|**9**|**13**|**10**|**12**|

#### $4 \times 4$ goal configuration
The goal configuration for all $N \times N$ puzzles is the same for each starting configuration of the same $N$. For a $4 \times 4$ puzzle, here is the goal configuration:
| 1 | 2 | 3 | 4 |
|--|---|---|--|
|**5**|**6**|**7**|**8**|  
|**9**|**10**|**11**|**12**|
|**13**|**14**|**15**|**0**|

While this puzzle may look easy to the uninitiated, writing a program that finds the solution in a reasonable amount of time is quite the challenge. This project contains several programs, written in C, that use an A* heuristic algorithm to solve the N-Puzzle. Additionally, contained in this README.md is a full writeup on the theoretical basis and ideas implemented in the programs.

## Exploring the $N$ Puzzle Problem With Complexity Theory
The $N$ Puzzle problem is in the class of NP-Hard. A full proof will not be given here, but those interested can click [here](https://dspace.mit.edu/bitstream/handle/1721.1/134978/1707.03146.pdf?sequence=2) for a very interesting proof using rectilinear Steiner trees to prove that the $N$ puzzle problem is NP-Hard. More important for our purposes here is the result of this classification. Problems in NP can be solved in nondeterminsitic polynomial time, meaning that any solver for the $N$-Puzzle has "branching" possibilies. This is where the NP in NP-Hard comes from. The "hard" part comes from the fact that this problem is, for lack of a better term, extremely hard to solve. Problems classified as NP hard also tend to be hard to verify as well. It is not difficult to see why this is the case. For any given $N$-Puzzle, the number of possible states is always $N^2!$, with the number of solveable states being $N^2! \div 2$. It is not difficult to find a solution, but finding the optimal solution is a considerable computational hurdle.

