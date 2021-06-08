#ifndef ELPSM_H_
#define ELPSM_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "backtrack.h"

#include <queue>
#include <bitset>
#include <map>
#include <set>

// we assume size of query graph <= 200

class ELPSM {
 public:
  ELPSM();
  ~ELPSM();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);
  void initialize(const Graph &data, const Graph &query, const CandidateSet &cs);
  void find_root(void);
  void DAG_CREATE(void);
  void DAG_merge(void);
  void DAG_display(void);
  void DAG_preprocess(void);
  void perf_ELPSM(size_t matched);

 private:
  Vertex root;
  std::vector<Vertex> M;        /* Match */
  const Graph *data;            /* pointer of data graph */
  const Graph *query;           /* pointer of query graph */
  const CandidateSet *cs;       /* pointer of CandidateSet */

  std::vector< std::set<Vertex> > cands; /* candidates for each vertex */
  std::vector< std::vector<Vertex> > query_dag; /* query dag edges */
  std::vector< std::vector<Vertex> > query_dag_rev; /* query dag edges reversed */
  std::vector< std::vector<Vertex> > merged;    /* merged vertices */
  std::vector< std::bitset<200> > pathex; /* checking whether path exists on dag */

  std::vector< std::vector<Vertex> > cand_rev; /* maps candidate to vertices that contain it */
  std::bitset<200> fake;         /* vertices that are not present in query dag since they are merged */
  std::bitset<200> largeleaf;    /* vertices that are merged vertices */
  std::vector<int> visit;       /* Check visited while searching */
  std::bitset<200> decided;     /* decided dag vertices */
  std::vector<int> qdid;        /* query dag indegree */

  void fast_leaf_match(size_t matched, Vertex whi, std::vector<Vertex> &cand, size_t idx, size_t this_matched);
  Vertex nxt_extend(size_t matched);
  void print_matches(size_t cur);
  void view_vertex(Vertex cur);
};

#endif  // ELPSM_H_
