/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"

Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

int Backtrack::print_cnt = 0;

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs, int MODE) {
  if(MODE == 0) {
    // ignore dag
    IgnoreDAG igdag;
    igdag.initialize(data, query, cs);
    igdag.perf_backtrack(0);
  }
  else if(MODE == 1) {
    // daf
    DAF daf;
    daf.PrintAllMatches(data, query, cs);
  }
  else if(MODE == 2) {
    // elpsm
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
  }
  else if (MODE == 3) {
    // optimize-da
    OPDA opda;
    opda.initialize(data, query, cs);
    opda.find_root();
    opda.DAG_CREATE();
    opda.DAG_preprocess();
    opda.perf_backtrack(0);
  }
  else if(MODE == 4) {
      IgnoreDAG2 igdag2;
      igdag2.initialize(data, query, cs);
      igdag2.perf_backtrack(0);
  }
}
