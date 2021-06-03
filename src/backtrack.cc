/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"

Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

int Backtrack::print_cnt = 0;

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
                /*              
  ELPSM elpsm;
  elpsm.initialize(data, query, cs);
  std::cerr << "finished initialization" << std::endl;
  elpsm.find_root();
  std::cerr << "found root" << std::endl;
  elpsm.DAG_CREATE();
  std::cerr << "created dag" << std::endl;
  elpsm.DAG_merge();
  std::cerr << "merged dag" << std::endl;
  elpsm.DAG_preprocess();
  std::cerr << "preprocessed dag" << std::endl;
  elpsm.DAG_display();
  elpsm.perf_ELPSM(0);
  */
  DAF daf;
  daf.PrintAllMatches(data, query, cs);
  
}
