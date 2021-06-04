#ifndef DAF_H_
#define DAF_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "backtrack.h"

#include <queue>

class DAF {
 public:
  DAF();
  ~DAF();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);

 private:
  std::vector<Vertex> M;        /* Match */
  const Graph *data;            /* pointer of data graph */
  const Graph *query;           /* pointer of query graph */
  const CandidateSet *cs;       /* pointer of CandidateSet */

  std::vector<int> check;       /* Check visited while searching */
  std::vector<bool> used;       /* Check used vertex of data */
  std::vector<int> qdd;         /* query dag degree */
  std::vector<std::vector<Vertex> > children;
  std::vector<std::set<Vertex> > C_m; /* candidate */

  void dag(Vertex id);
  void initialize();
  void candidate_size_order(Vertex id, size_t matched);
};

#endif  // DAF_H_
