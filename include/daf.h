#ifndef DAF_H_
#define DAF_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"

#include <queue>

class DAF {
 public:
  DAF();
  ~DAF();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);

 private:
  const int LIMIT = 100000;     /* Maximum number of match prints */
  int print_cnt = 0;            /* Number of match prints */

  std::vector<Vertex> M;        /* Match */
  const Graph *data;            /* pointer of data graph */
  const Graph *query;           /* pointer of query graph */
  const CandidateSet *cs;       /* pointer of CandidateSet */

  std::vector<bool> check;      /* Check visited while searching */
  std::vector<bool> used;       /* Check used vertex of data */
  std::vector<int> qdd;         /* query dag degree */

  struct vcs {
      Vertex id;
      std::vector<Vertex> cs;
      bool operator<(const vcs &a) const {
        return cs.size() == a.cs.size() ? id > a.id : cs.size() > a.cs.size();
      }
  };
  std::priority_queue<vcs> children;

  inline void printMach();
  void dag(Vertex id);
  void candidate_size_order(Vertex id, size_t matched);
};

/**
 * @brief Print one match according to the format & check limit
 *
 * @return void
 */
inline void DAF::printMach() {
    std::cout << "a ";
    for(auto& e : M)
        std::cout << e << " ";
    std::cout << "\n";
    
    print_cnt += 1;
    if(print_cnt >= LIMIT) exit(0);
}

#endif  // DAF_H_
