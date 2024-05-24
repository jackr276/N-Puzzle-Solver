# $N$ Puzzle Solver
Author: [Jack Robbins](https://www.github.com/jackr276)

## Introduction
The $N$ Puzzle problem is on the surface a simple problem to solve. The problem is as follows:
> Given an initial configuration of an $N\times N$ grid of tiles, each labeled with distinct numbers from $0$ to $N^2 - 1$, determine the *smallest* sequence of single step moves that can be made with the $0$ tile such that the puzzle is in numerical *row-major* order, with the 0 slider in the $N-1$ row and $N-1$ column position.

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

While this puzzle may look easy to the uninitiated, writing a program that finds the solution in a reasonable amount of time is quite the challenge. This particular example is solved optimally in 24 moves of the 0 slider.  

This project contains several programs, written in C, that use an A* heuristic algorithm to solve the N-Puzzle. Additionally, contained in this README.md is a full writeup on the theoretical basis and ideas implemented in the programs.

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
  end if

  expand state into its predecessors

  for each predecessor generated:
    if state is a repeat(in closed or fringe already):
        destroy state
        continue
    else:
        update prediction function for state
        priority queue insert state into fringe
    end if
  end for

  add current state into closed 
end while

If we reach here, no solution 
```
In the algorithm above, we see that there are two data structures: **fringe** and **closed**. **Fringe** is a priority queue, where the states with the lowest prediction function value(i.e., closest to goal) are put first. This ensures that when the algorithm dequeues, it is always dequeueing and expanding the most promising state. **Closed** is simply a linked list that stores every node that we have previously expanded. Maintaining these two linked lists is of vital importance to the performance of our algorithm. If we don't keep track of which states we have already visited, it is likely that the algorithm would re-explore states, and there is even a chance that it could get stuck in an infinite loop.  

### Prediction Function
The other important part of this algorithm is the prediction function for each state. This prediction function is the most importnat part of our algorithm. Effectively, this algorithm "guesses" how promising a state is. The lower the value of this prediction function, the more promising the state seems. Good "guesses" allow the algorithm to finish faster, and bad "guesses" will cause the algorithm to explore dead end paths and waste time. The prediction function implemented here focuses on estimating the cost to get from the current state to the goal state.    

The format for our prediction function is as follows: `f(n) = g(n) + h(n)`. `g(n)` is referred to as the "previous travel", and is the simplest to calculate. It is calculated by counting the number of predecessors that a node has. This allows us to factor in the cost to reach a state so far, which is important if we want to find the **shortest** solution path possible. Additionally, `g(n)` can be calculated during state generation by simply adding one to the predecessors current travel, making it basically free to compute.

#### Heuristic Function
However, the heuristic function `h(n)` is where most of the heavy lifting happens for the algorithm. This function uses calculations guess the number of moves required to change the current state into the goal state. These calculations are referred to as *heuristics*, and more specifically as *admissable heuristics*. For a heuristic to be admissable, it must always **understimate or equal** the cost to reach the goal for any given state. This ensures that we are always following the most optimal, meaning shortest, path to the solution. Heuristics that violate admissablility will not crash the program or cause any drastic errors, they will just simply lead to suboptimal solutions.

This project currently uses two heuristics in combination: `manhattan distance` and `generalized linear conflict`.

#### Manhattan Distance
The Manhattan Distance heuristic is quite simple. Given a board and knowing the goal state, it calculates the sum of the horizontal and vertical distance from each tile's current position to its goal position. It can be imagined as taking each tile and sliding it horizontally and vertically into its goal position. This heuristic gives an effective lower bound on the number of moves required to reach a state, becuase it does not take into consideration the interactions between tiles. For this reason, Manhattan distance is an admissable heuristic, because it will always underestimate the cost of reaching the goal state.

From our example before:   
| 1 | 7 | 15 | 4 |
|--|---|---|--|
|**0**|**6**|**3**|**8**|  
|**2**|**5**|**14**|**11**|
|**9**|**13**|**10**|**12**|      

The Manhattan distance would be: `0 + 2 + 3 + 0 + 0 + 1 + 0 + 3 + 1 + 2 + 1 + 1 + 1 + 2 + 1 = 18`   
It is important to note that we skip calculation anything for the 0 tile, as it is allowed to move. As it turns out, skipping the 0 tile for Manhattan distance calculations leads to much better cost estimation, as the 0 tile is what we use to manipulate the board.   

As stated before, Manhattan Distance does not take into account the interactions between tiles that are required for movement. This means that it is quite a drastic underestimate of how many moves are required to reach a state, leaving room for improvement. We will do this improvement with our next heuristic, generalized linear conflict.   
#### Generalized Linear Conflict
Generalized Linear Conflict can make up for Manhattan Distance's ignorance of tile interactions. Traditional Linear Conflict looks for two tiles that are both already in their goal row or goal column, but who are "swapped", meaning that their Manhattan Distances are both 1, and counts them as a linear conflict. Even though both Manhattan Distances are 1, it will take at least 2 additional moves to swap the two tiles around, because we have to use the 0 tile to do this. In the traditional heuristic, we count the number of such swapped tiles and multiply this number by 2 to get the Linear Conflict value. In my version of the heuristic, I generalize this conflict to the entire row and column, and instead of only searching for tiles that are 1 apart, I search for any mismatched tiles in the entire row. With this improved version of the heuristic, we get a more accurate prediction of how many moves are required to properly rearrange the entire board. It will of course still be an underestimate, and therefore admissable, because counting sometimes it may take more than 2 moves to swap two tiles. In fact, it often requires more than 2 moves, but we must keep the heuristic an underestimate to ensure that we always find the **shortest** path.

>[!NOTE]
>Potential Improvement Idea: try and find a more accuracte prediction multiplication. Initial research online suggested that taking the linear conflict count and doing `linear_conlict_count / (N - 1) + linear_conflict_count % (N - 1)`, but my results with this method have shown that this violates admissability in some cases


