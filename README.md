# $N$ Puzzle Solver
Author: [Jack Robbins](https://www.github.com/jackr276)

## Introduction
The $N$ Puzzle problem is on the surface a simple problem to solve. The problem is as follows:
> Given an initial configuration of an $N\times N$ grid of tiles, each labeled with distinct numbers from $0$ to $N^2 - 1$, determine the *smallest* sequence of single step moves that can be made with the $0$ tile such that the puzzle is in numerical *row-major* order, with the 0 slider in the $N-1$ row and $N-1$ column.

The definition can be a bit hard to parse without an example, so let's look at an example starting configuration and goal configuration.

#### Example $4 \times 4$ starting configuration
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

While this puzzle may look easy to the uninitiated, writing a program that finds the solution in a reasonable amount of time is quite the challenge. This project contains several programs, written in C, that use an A* heuristic algorithm to solve the N-Puzzle. Additionally, contained in this README.md is a full writeup on the theoretical basis and ideas implemented in the program.

## Exploring the $N$ Puzzle Problem More
The $N$ Puzzle problem is in the class of NP-Hard. A full proof will not be given here, but those interested can click [here](https://dspace.mit.edu/bitstream/handle/1721.1/134978/1707.03146.pdf?sequence=2) for a 2017 proof using rectilinear steiner trees to prove that the $N$ puzzle problem is NP-Hard.

## Configs solved in reasonable amount of time
 9  3  5  4   1  6  8  7   2 13 15 10  11 12 14  0   
 
  6 10  1  3  14 13  4  0   5 12  2 15   9  7  8 11  
  
   5  3  2  1   7 13 14  0   6 11  8  4  15  9 10 12    
   
   11  1  4  8   3  9 10  2   7 13 14 12   5  6 15  0   
   
    2  5 10  3  14  1  8  7  13  6  0 15   9 11 12  4  
    
 8  5  0  1  10 11  6  4  3  2 12  7 9 13 14 15  

  7  9  2  4  14  1 10 12   3  8  5 15   6 13 11  0   

   1  4  6 10   5  3  7  2  15  9  8  0  14 12 13 11   
    
6  1  3  8 2 11  7 12 14  9  0 15 5 13  4 10 
