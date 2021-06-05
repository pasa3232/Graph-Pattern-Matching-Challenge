#ifndef OPTIMIZEDA_H_
#define OPTIMIZEDA_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "backtrack.h"

#include <queue>
#include <bitset>
#include <map>
#include <set>


// we assume size of query graph <= 200

class OPDA {
 public:
  OPDA();
  ~OPDA();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);
  void initialize(const Graph &data, const Graph &query, const CandidateSet &cs);
  void find_root(void);
  void DAG_CREATE(void);
  void DAG_preprocess(void);
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
  std::vector< std::vector<Vertex> > query_dag; /* query dag edges */
  std::vector< std::vector<Vertex> > query_dag_rev; /* query dag edges reversed */
  std::vector< std::bitset<200> > pathex; /* path exist on dag (parent) */

  std::vector< std::vector<Vertex> > cand_rev; /* candidate */
  std::vector<int> visit;       /* Check visited while searching */
  std::bitset<200> decided;     /* decided dag vertices */
  std::vector<int> qdid;        /* query dag indegree */

  Vertex nxt_extend(size_t matched);
};

#endif  // OPTIMIZEDA_H_
