/**
 * @file main.cc
 *
 */

#include "backtrack.h"
#include "candidate_set.h"
#include "common.h"
#include "graph.h"

#include <cstring>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

static bool CHECK_MODE = false;

void parse(int argc, char* argv[]);
size_t check(const Graph &data, const Graph &query, const CandidateSet &cs);

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

  if(CHECK_MODE) {
    pid_t pid;
    time_t start = clock(), end;
    if((pid = fork()) == 0){
      if(!freopen("../result.txt", "w", stdout)) return 1;
      backtrack.PrintAllMatches(data, query, candidate_set);
      return EXIT_SUCCESS;
    }
    wait(NULL);
    end = clock();
    cout<<"--------------------check result--------------------"<<"\n";
    cout<<"running time: " << (double )end - start << "(ms)" << "\n";
    if(!freopen("../result.txt", "r", stdin)) return 1;
    cout<<"found: "<<check(data, query, candidate_set)<<"\n";
    cout<<"Check succeed."<<"\n";
    return EXIT_SUCCESS;
  }
  else {
    backtrack.PrintAllMatches(data, query, candidate_set);
    return EXIT_SUCCESS;
  }
}

void parse(int argc, char* argv[]) {
  if(argc <= 4) return;
  if(argv[4][0] != '-')
    if(!freopen(argv[4], "w", stdout)) exit(0);
  for(int i = 4; i < argc; i++){
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == 'c'){   /* CHECK-MODE */
      CHECK_MODE = true;
    }
  }
}

void error(string msg){
  cout<<msg<<"\n";
  exit(0);
}

size_t check(const Graph &data, const Graph &query, const CandidateSet &cs){
  size_t n, cnt=0;
  char tmp;
  if(scanf("%c %ld", &tmp, &n)!=2 || tmp != 't') error("invalid output");
  while(scanf(" %c", &tmp)!=EOF) {
    cnt++;
    if(tmp!='a') error("invalid output");
    vector<Vertex> v = vector<Vertex>(n);
    for(size_t i = 0; i < n; i++)
      if(scanf("%d", &v[i])!=1) error("invalid output");
    
    for(size_t i = 0; i < n; i++) {
      size_t st = query.GetNeighborStartOffset(i);
      size_t en = query.GetNeighborEndOffset(i);
      for(size_t j = st; j < en; j++)
        if(!data.IsNeighbor(v[i], v[query.GetNeighbor(j)])) {
          printf("Check failed.\n");
          printf("a ");
          for(size_t k = 0; k < n; k++) printf("%d ", v[k]);
          printf("\n");
          exit(0);
        }
    }
  }
  return cnt;
}
