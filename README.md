# Graph Pattern Matching Challenge
## Compile and Execute 
### main program
```
mkdir build
cd build
cmake ..
make
./main/program <data graph file> <query graph file> <candidate set file>
```

### Specify the output file.
```
./main/program <data graph file> <query graph file> <candidate set file> <output file>
```

### check option -c
```
./main/program <data graph file> <query graph file> <candidate set file> -c
```
Result is saved in result.txt file and the checker log is printed to stdout.
### backtrack mode option -0 -1 -2 -3
```
./main/program <data graph file> <query graph file> <candidate set file> -0
./main/program <data graph file> <query graph file> <candidate set file> -1
./main/program <data graph file> <query graph file> <candidate set file> -2
./main/program <data graph file> <query graph file> <candidate set file> -3
./main/program <data graph file> <query graph file> <candidate set file> -4
0 : ignore dag2 (default)
1 : DAF
2 : ELPSM
3 : optimize_da
4 : ignore dag
```
Can use with -c option
### executable program that outputs a candidate set
```
./executable/filter_vertices <data graph file> <query graph file>
```
### References
[1] Myoungji Han, Hyunjoon Kim, Geonmo Gu, Kunsoo Park, and Wook-Shin Han. 2019. Efficient Subgraph Matching: Harmonizing Dynamic Programming, Adaptive Matching Order, and Failing Set Together. In Proceedings of the 2019 International Conference on Management of Data (SIGMOD '19). Association for Computing Machinery, New York, NY, USA, 1429–1446. DOI:https://doi.org/10.1145/3299869.3319880
