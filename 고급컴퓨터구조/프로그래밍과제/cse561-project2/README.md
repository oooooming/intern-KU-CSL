## CSE561/SCE412 Project2 - Game of Life with Pthread and CUDA

Repository for project2 of CSE561/SCE412 Advanced Computer Architecture @ Ajou University (Spring 2021)

### Overview

The goal of this project is to implement the **Game of Life** using *pthread* and *CUDA*. Print the right results and compare the execution time of each implementation by running several sample inputs. Follow the guidelines and refer to the detailed information from the given handout. 

### Lab Environment
* Ubuntu 18.04
* gcc/g++ version 6.5.0
* CUDA 10.0
* 12 Physical cores (24 hardware threads) 
* NVIDIA TITAN X (Pascal)

### Build and Run

- We provide `Makefile` to compile both `.cu` and `.cpp` files. 
- To compile and run:
```
$ make

$ ./glife sample_inputs/<file_name> <display> <nprocs> <# of generation> <width> <height>
```
- If `display = 1`, the program will dump the index output. If you run on GPU, you need to set `nprocs = 0`. `nprocs = 1` will run by a single thread, and `nprocs > 1` will run by multiple threads.
- You need to specify enough number of `width` and `height` to avoid segmentation fault errors.
- If you use your own NVIDIA GPU, you need to add compatibility number on `Makefile`. For example, if you use NVIDIA GeForce RTX 3090 which of the compatibility number is 8.6, you have to add `GENCODE_SM86 = -gencode=arch=compute_86,code=\"sm_86,compute_86\"` on the first line. 

### TAs 

* Taeklim Kim (limkim4233 at ajou dot ac dot kr)
* Wonkyo Choe (heysid at ajou dot ac dot kr)

If you have any questions, please post on Ajou BB and TAs will give you an answer.
