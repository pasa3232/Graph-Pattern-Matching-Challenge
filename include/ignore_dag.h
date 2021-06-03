#ifndef IGNOREDAG_H_
#define IGNOREDAG_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include "backtrack.h"

#include <queue>
#include <bitset>
#include <map>
#include <set>

/*

순서 : query DAG를 만든다 -> leaf 중 동일한 것을 합친다

data graph is small => do it anyway we want..
DAG : edge를 std::vector< std::vector<Vertex> > 로 구현
leaf 중 동일한 것 : indegree 1 & outdegree 0이면 parent 저장 -> 묶기
묶는 방법 : std::vector< std::vector<Vertex> >로 구현 
묶인 index 따로 관리 (bitset)

iteration

필요한 것 : 각 vertex에 대하여 "내 부모가 전부 결정되었는가?"
결정된 것의 bitset을 global 하게 두면 해결 (크기 query.numvertices())

선택된 것의 bitset global 하게 두고
C_M(u) = C(u) \cap N( matched parent )
한 방에 하는 게 나을수도..

*/

// we assume size of query graph <= 200

class IgnoreDAG {
 public:
  IgnoreDAG();
  ~IgnoreDAG();

  void PrintAllMatches(const Graph &data, const Graph &query,
                       const CandidateSet &cs);
  void initialize(const Graph &data, const Graph &query, const CandidateSet &cs);
  void perf_ELPSM(size_t matched);
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
  std::bitset<200> decided;     /* decided dag vertices */

  Vertex nxt_extend(size_t matched);
};

#endif  // IGNOREDA_H_
