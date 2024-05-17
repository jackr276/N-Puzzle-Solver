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
Before writing programs and attempting to solve the $N$ puzzle, we should first ask if this problem is even solveable at all. To do this, we will use a very simple proof to show that the $N$ puzzle is **decidable**. As a reminder, here is what it means for a problem to be **decidable**:
> A problem(or more specifically, the language made from that problem) is **decidable** if $\exists$ some Turing Machine $M$ that, on input $w \in \Omega$ where $\Omega$ is the universe of all possible problem instances:   
>      1. Always halts and accpets if $w \in A$, where $A$ is the set of all "yes instances"   
>      2. Always halts and rejects if $w \notin A$
>         
> Importantly, $M$ must **never** get stuck in an infinite loop.       

With this in mind, let's prove that the $N$ Puzzle Problem is decidable:

**Begin Proof**     
Let $w \in \Omega$ be a problem instance of the N puzzle. Additionally let $\langle w \rangle$ be the string encoding of this problem instance that contains the value for $N$ and the starting configuration afterwards in row-major order.
Now define decider $D$ that operates as follows:     
$D$ = "On input $\langle w \rangle$:   
$\quad$ 0. Read through $\langle w \rangle$ to ensure it is a valid problem instance. If not, reject.   
$\quad$ 1. Mathematically generate goal configuration for value of $N$ given, store on tape. Also store the starting configuration given in $\langle w \rangle$.
$\quad$ 2. Generate all possible successor configurations from the starting configuration. For each one of these configurations, put it     
$\quad$    on the tape one after another. Additionally, store the position of the start of its predecessor at the end of the configuration.     
$\quad$ 3. Repeat this process. Each time, before generating more configurations, first check if the configuration being expanded is the    
$\quad$   goal, if it is, trace back the solution path using the predecessors and halt and accept. Additionally, on generating each           
$\quad$   configuration, first check if it is repeating by scanning over the entire tape and checking it against other configurations        
$\quad$   that have already been made. If it is a repeat, erase it from the tape.    
$\quad$ 4. If eventually, no more non-repeating configurations can be generated and the solution isn't found, halt and reject."   

Since there is always a finite(although very large) number of possible successor states for each $N$ puzzle problem instance, $D$ will always eventually either halt and accept or halt and reject, making it a decider. Therefore, since $\exists$ a TM $D$ that decides this problem, it is *decidable*.   
**End Proof**
  


## Proof that the $N$ Puzzle is NP-Hard

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
