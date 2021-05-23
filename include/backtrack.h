/**
 * @file backtrack.h
 *
 */

#ifndef BACKTRACK_H_
#define BACKTRACK_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "daf.h"

class Backtrack {
 public:
  Backtrack();
  ~Backtrack();

  const static int LIMIT=100000;        /* Maximum number of match prints */

  inline static void printMatch(std::vector<Vertex> &M);
  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);

private:
  static int print_cnt;          /* Number of match prints */
};

/**
 * @brief Print one match according to the format & check limit
 *
 * @return void
 */
inline void Backtrack::printMatch(std::vector<Vertex> &M) {
    std::cout << "a ";
    for(auto& e : M)
        std::cout << e << " ";
    std::cout << "\n";
    
    print_cnt += 1;
    if(CHECK_MODE) exit(0);
    if(print_cnt >= LIMIT) exit(0);
}

#endif  // BACKTRACK_H_
