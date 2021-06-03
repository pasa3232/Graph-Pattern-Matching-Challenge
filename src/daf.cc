#include "daf.h"
using namespace std;

DAF::DAF() {}
DAF::~DAF() {}

void DAF::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  this->data = &data;
  this->query = &query;
  this->cs = &cs;
  size_t num_qv = query.GetNumVertices();
  
  cout << "t " << num_qv << "\n";
  M.resize(num_qv);
  qdd.resize(num_qv);
  check.resize(num_qv);
  parents.resize(num_qv);
  C_m.resize(num_qv);
  used.resize(data.GetNumVertices());

  // find root
  Vertex root = 0;
  for(size_t i=1; i<num_qv; i++) {
    if(cs.GetCandidateSize(i) < cs.GetCandidateSize(root))
      root = i;
  }
  
  // find query dag and set degree
  fill(check.begin(), check.end(), 0);
  fill(qdd.begin(), qdd.end(), 0);
  for(size_t i=0; i<num_qv; i++) parents[i].clear();
  dag(root);

  // candidate size order backtracking
  fill(used.begin(), used.end(), false);
  fill(check.begin(), check.end(), 0);
  for(size_t i=0; i<num_qv; i++) C_m[i].clear();
  for(size_t i=0; i<cs.GetCandidateSize(root); i++){
    Vertex v = cs.GetCandidate(root, i);
    M[root] = v;
    used[v] = true;
    candidate_size_order(root, 1);
    used[v] = false;
  }
}

void DAF::dag(Vertex id) {
  /* check
   *  0: not pushed
   *  1: in que
   *  2: poped
   */
  check[id] = 1;
  queue<Vertex> q;
  q.push(id);
  while(!q.empty()) {
    id = q.front();
    q.pop();
    check[id] = 2;

    size_t st = query->GetNeighborStartOffset(id);
    size_t en = query->GetNeighborEndOffset(id);

    for(size_t i = st; i < en; i++) {
      Vertex u = query->GetNeighbor(i);
      if(check[u] == 2){
        qdd[id]++;
        parents[id].push_back(u);
      }
      if(check[u] == 0) {
        q.push(u);
        check[u] = 1;
      }
    }
  }
}

void DAF::candidate_size_order(Vertex id, size_t matched) {
  if(matched == query->GetNumVertices()){
    Backtrack::printMatch(M);
    return;
  }
  check[id] = 1;

  size_t st = query->GetNeighborStartOffset(id);
  size_t en = query->GetNeighborEndOffset(id);

  for(size_t i = st; i < en; i++) {
    Vertex u = query->GetNeighbor(i);
    if(--qdd[u]) continue;
    
    // find valid candidate set of u
    C_m[u].clear();
    for(size_t j = 0; j < cs->GetCandidateSize(u); j++) {
      Vertex v = cs->GetCandidate(u, j);
      // check if v is already used
      if(used[v]) continue;

      // check if v is connected with parents
      bool flag = true;
      for(Vertex p : parents[u])
        if(!data->IsNeighbor(v, M[p])) flag = false;
      
      if(flag) C_m[u].push_back(v);
    }
  }
  
  Vertex u = -1;
  size_t m = data->GetNumVertices() + 100;
  for(size_t i = 0; i < query->GetNumVertices(); i++) {
    if(qdd[i] == 0 && check[i] == 0){
      if(C_m[i].size() != 0 && m > C_m[i].size()){
        u = i;
        m = C_m[i].size();
      }
    }
  }

  if(u != -1)
    for(size_t i = 0; i < C_m[u].size(); i++){
        Vertex v = C_m[u][i];
        if(used[v]) continue;
        M[u] = v;
        used[v] = true;
        candidate_size_order(u, matched+1);
        used[v] = false;
    }

  for(size_t i = st; i < en; i++) {
    u = query->GetNeighbor(i);
    qdd[u]++;
  }
  check[id] = 0;
}