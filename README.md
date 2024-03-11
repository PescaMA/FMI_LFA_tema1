# DFA, NFA, lambda-NFA acceptor

Finite automata acceptor written in C++ (C++11). Can Load an Lambda-Non deterministic finite automata and run on different inputs. The states are integers and the symbols are chars. 


I considered lambda as the character "@". The rest of the symbols can be any other ascii character (C++ char).

## Installation

Normal installation using git. Clone the repository (`git clone https://github.com/PescaMA/FMI_LFA_tema1.git`).

## Usage

You can change the input inside input.txt with the input you want.

Inside CodeBlocks running the main will work. With othe apps you can just compile with C++11 (or move the executable in the same folder as input.txt).

The result will be in output.txt for the accepted/rejected message, and in terminal for the paths.

## Creating an input

> 1. n -> Nr. of states

> 2. n states (integers)

> 3. m -> Nr. of transitions

> 4. nodeX nodeY charL -> transition from X to Y with symbol L

> 5. q0 -> initial(start) state

> 6. nrFin -> the number of final states

> 7. list of nrFin final states



> 8. wordCount -> amount of words to be tested/accepted

> 9. list of wordCount tests


## Contributing

I won't be accepting contributions, as this is more a piece of history than a collaborative project.

## Credits

- Original author: Pescariu Matei-Alexandru (https://github.com/PescaMA).
- The full original requirements are present in the folder [Requirements](/Requirements/), originally in Romanian, but translated by google translate.
- Project requirements were made by Oanea Smit.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
