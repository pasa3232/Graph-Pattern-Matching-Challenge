/**
 * @file main.cc
 *
 */

#include "backtrack.h"
#include "candidate_set.h"
#include "common.h"
#include "graph.h"

#include <cstring>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

static bool CHECK_MODE = false;

/*
 * Default 0 
 * 0 : ignore dag2
 * 1 : DAF
 * 2 : ELPSM
 * 3 : optimize_da
 * 4 : ignore dag
 */
static int MODE = 0;

static void parse(int argc, char* argv[]);
static size_t check(const Graph &data, const Graph &query, const CandidateSet &cs);

int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: ./program <data graph file> <query graph file> "
                 "<candidate set file>\n";
    return EXIT_FAILURE;
  }

  std::string data_file_name = argv[1];
  std::string query_file_name = argv[2];
  std::string candidate_set_file_name = argv[3];

  parse(argc, argv);

  Graph data(data_file_name);
  Graph query(query_file_name, true);
  CandidateSet candidate_set(candidate_set_file_name);

  Backtrack backtrack;

  if(CHECK_MODE) {    /* CHECK MODE */
    pid_t pid;
    auto t1 = chrono::high_resolution_clock::now();
    if((pid = fork()) == 0){    /* child process run */
      if(!freopen("result.txt", "w", stdout)) return 1;
      backtrack.PrintAllMatches(data, query, candidate_set, MODE);
      return EXIT_SUCCESS;
    }
    wait(NULL);                 /* child sub process */
    auto t2 = chrono::high_resolution_clock::now();
    chrono::duration<int64_t,nano> elapsed = t2 - t1;   /* running time */
    
    // print check result
    cout<<"--------------------check result--------------------"<<"\n";
    cout<<"running time (Nanosec) : " << elapsed.count() << "\n";
    cout<<"running time (Millisec): " << (double)elapsed.count()/1000000 << "\n";
    cout<<"running time (Seconds) : " << (double)elapsed.count()/1000000000 << "\n";
    if(!freopen("result.txt", "r", stdin)) return 1;
    size_t found = check(data, query, candidate_set);
    cout<<"found: "<<found<<"\n";
    cout<<"Check succeed."<<"\n";
    return EXIT_SUCCESS;
  }
  else {
    backtrack.PrintAllMatches(data, query, candidate_set, MODE);
    return EXIT_SUCCESS;
  }
}

/*
 * static void parse(int argc, char* argv[])
 * 
 * parse command line
 *  <output file>
 *  -c
 *  -0 -1 -2 -3 -4
 */
static void parse(int argc, char* argv[]) {
  if(argc <= 4) return;
  if(argv[4][0] != '-')
    if(!freopen(argv[4], "w", stdout)) exit(0);
  for(int i = 4; i < argc; i++){
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == 'c'){   /* CHECK-MODE */
      CHECK_MODE = true;
    }
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '0'){
      MODE = 0;
    }
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '1'){
      MODE = 1;
    }
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '2'){
      MODE = 2;
    }
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '3'){
      MODE = 3;
    }
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '4'){
      MODE = 4;
    }
  }
}

static void error(string msg){
  cout<<msg<<"\n";
  exit(0);
}

/*
 * size_t check(const Graph &data, const Graph &query, const CandidateSet &cs)
 * 
 * return number of matches that our algorithm found
 * and check validation of matches
 */
static size_t check(const Graph &data, const Graph &query, const CandidateSet &cs){
  size_t n, cnt=0, fail=0;
  char tmp;
  if(scanf("%c %ld", &tmp, &n)!=2 || tmp != 't') error("invalid output");
  while(scanf(" %c", &tmp)!=EOF) {
    bool isok = true;
    if(tmp!='a') error("invalid output");
    vector<Vertex> v = vector<Vertex>(n);
    for(size_t i = 0; i < n; i++) if(scanf("%d", &v[i])!=1) isok = false;
    for(size_t i = 0; i < n; i++) {
      size_t st = query.GetNeighborStartOffset(i);
      size_t en = query.GetNeighborEndOffset(i);
      for(size_t j = st; j < en; j++)
        if(!data.IsNeighbor(v[i], v[query.GetNeighbor(j)])) 
        { isok = false;
            //cerr << i << " " << query.GetNeighbor(j) << "\n";
            break;
        }
    }
     //cerr << isok << "\n";
    if(isok) cnt += 1;
    else fail += 1;
  }
  cout << "failed " << fail << " times" << "\n";
  return cnt;
}