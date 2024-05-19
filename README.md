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
The $N$ Puzzle problem is in the class of NP-Hard. A full proof will not be given here, but those interested can click [here](https://dspace.mit.edu/bitstream/handle/1721.1/134978/1707.03146.pdf?sequence=2) for a very interesting proof using rectilinear Steiner trees to prove that the $N$ puzzle problem is NP-Hard. More important for our purposes here is the result of this classification. Problems in NP can be solved in nondeterminsitic polynomial time, meaning that any solver for the $N$-Puzzle has "branching" possibilies. This is where the NP in NP-Hard comes from. The "hard" part comes from the fact that this problem is, for lack of a better term, extremely hard to solve. Problems classified as NP hard also tend to be hard to verify as well. It is not difficult to see why this is the case for this given problem. For any given $N$-Puzzle, the number of possible states is always $N^2!$, with the number of solveable states being $N^2! \div 2$. It is not difficult to find any given solution, but finding the optimal solution requires considerable amounts of computation.   

## A* Informed Search Algorithm
Given a great deal of time and memory, it would be possible to explore every possible solution path to each given problem instance. However, we can do much better than this. To manage the complexity of the $N$ puzzle and find an optimal solution relatively quickly, we will use the A* best-first search algorithm. The idea here is to explore the most promising states first, in the hopes of quickly finding an optimal solution. 

### Pseudo-Code
Here is the general idea of our search algorithm:
```
A* SEARCH ALGORITHM

Add starting state to fringe
Initialize closed list to be null

while fringe is not empty:
  dequeue a state from fringe

  if state is goal:
      trace back solution path
      exit

  expand state into its predecessors
  for each predecessor generated:
    if state is a repeat(in closed or fringe already):
        destroy state
        continue
    else:
        update prediction function for state
        priority queue insert state into fringe
  end for

  add current state into closed 
end while

If we reach here, no solution 
```
In the algorithm above, we see that there are two data structures: **fringe** and **closed**. **Fringe** is a priority queue, where the states with the lowest prediction function value(i.e., closest to goal) are put first. This ensures that when the algorithm dequeues, it is always dequeueing and expanding the most promising state. **Closed** is simply a linked list that stores every node that we have previously expanded. Maintaining these two linked lists is of vital importance to the performance of our algorithm. If we don't keep track of which states we have already visited, it is likely that the algorithm would re-explore states, and there is even a chance that it could get stuck in an infinite loop.  

### Prediction Function
The other important part of this algorithm is the prediction function for each state. This prediction function is the most importnat part of our algorithm. Effectively, this algorithm "guesses" how promising a state is. The lower the value of this prediction function, the more promising the state seems. Good "guesses" allow the algorithm to finish faster, and bad "guesses" will cause the algorithm to explore dead end paths and waste time. The prediction function implemented here focuses on estimating the cost to get from the current state to the goal state.    

The format for our prediction function is as follows: `f(n) = g(n) + h(n)`. `g(n)` is referred to as the "previous travel", and is the simplest to calculate. It is calculated by counting the number of predecessors that a node has. This allows us to factor in the cost to reach a state so far, which is important if we want to find the **shortest** solution path possible. However, the heuristic function `h(n)` is where most of the heavy lifting happens for the algorithm. As the heuristic function, it uses calculations to guess the number of moves required to change the current state into the goal state.
