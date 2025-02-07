#ifndef IGNOREDAG2_H_
#define IGNOREDAG2_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "backtrack.h"

#include <queue>
#include <bitset>
#include <map>
#include <set>

// the basics are same as IgnoreDAG, only minor implementation details are different
// also, a small change in the matching order for tiebreaking : refer to ignore_dag_2.cc

class IgnoreDAG2 {
 public:
  IgnoreDAG2();
  ~IgnoreDAG2();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);
  void initialize(const Graph &data, const Graph &query, const CandidateSet &cs);
  void perf_backtrack(size_t matched);
  void set_root(Vertex v);
  Vertex get_root(void);
 private:
  Vertex root;
  std::vector<Vertex> M;        /* Match */
  const Graph *data;            /* pointer of data graph */
  const Graph *query;           /* pointer of query graph */
  const CandidateSet *cs;       /* pointer of CandidateSet */

  std::vector< std::set<Vertex> > cands; /* candidate */
  std::vector< std::vector<Vertex> > query_edge; /* query edge */

  std::vector< std::vector<Vertex> > cand_rev; /* candidate */
  std::vector<int> visit;       /* Check visited while searching */
  std::vector<int> undecided_neighbor; /* number of undecided neighbors */
  std::bitset<200> decided;     /* decided dag vertices */

  Vertex nxt_extend(size_t matched);
};

#endif  // IGNOREDAG2_H_