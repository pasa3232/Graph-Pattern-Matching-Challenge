#include "daf.h"
using namespace std;

DAF::DAF() {}
DAF::~DAF() {}

void DAF::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  this->data = &data;
  this->query = &query;
  this->cs = &cs;
  
  cout << "t " << query.GetNumVertices() << "\n";
  M.resize(query.GetNumVertices());
  qdd.resize(query.GetNumVertices());
  check.resize(query.GetNumVertices());
  used.resize(data.GetNumVertices());

  // find root
  Vertex root = 0;
  for(size_t i=1; i<query.GetNumVertices(); i++) {
    if(cs.GetCandidateSize(i) < cs.GetCandidateSize(root))
      root = i;
  }
  
  // find query dag and set degree
  fill(check.begin(), check.end(), 0);
  fill(qdd.begin(), qdd.end(), 0);
  dag(root);

  // candidate size order backtracking
  fill(check.begin(), check.end(), 0);
  fill(used.begin(), used.end(), false);
  for(size_t i=0; i<cs.GetCandidateSize(root); i++){
    M[root] = cs.GetCandidate(root, i);
    used[cs.GetCandidate(root, i)] = true;
    candidate_size_order(root, 1);
    used[cs.GetCandidate(root, i)] = false;
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
  while(!q.empty()){
    id = q.front();
    q.pop();
    check[id] = 2;

    size_t st = query->GetNeighborStartOffset(id);
    size_t en = query->GetNeighborEndOffset(id);

    for(size_t i = st; i < en; i++) {
      Vertex u = query->GetNeighbor(i);
      if(check[u] == 2) qdd[id]++;
      if(check[u] == 0) {
        q.push(u);
        check[u] = 1;
      }
    }
  }
}

void DAF::candidate_size_order(Vertex id, size_t matched) {
  check[id] = 1;
  if(matched == query->GetNumVertices()){
    Backtrack::printMatch(M);
    return;
  }

  size_t st = query->GetNeighborStartOffset(id);
  size_t en = query->GetNeighborEndOffset(id);

  for(size_t i = st; i < en; i++) {
    Vertex u = query->GetNeighbor(i);
    if(--qdd[u]) continue;

    // find parents
    vector<Vertex> parents;
    for(size_t j = query->GetNeighborStartOffset(u); j < query->GetNeighborEndOffset(u); j++) {
      Vertex v = query->GetNeighbor(j);
      if(check[v])
        parents.push_back(v);
    }
    
    // find valid candidate set of u
    vector<Vertex> valid_cs;
    for(size_t j = 0; j < cs->GetCandidateSize(u); j++) {
      Vertex v = cs->GetCandidate(u, j);
      // check if v is already selected
      if(used[v]) continue;

      // check if v is connected with parents
      bool flag = true;
      for(Vertex p : parents)
        if(!data->IsNeighbor(v, M[p])) flag = false;
      
      if(flag) valid_cs.push_back(v);
    }

    if(valid_cs.size()) children.push((vcs){u, valid_cs});
  }
  
  if(!children.empty()) {
    vcs child = children.top();
    children.pop();
    for(Vertex v: child.cs){
        M[child.id] = v;
        used[v] = true;
        candidate_size_order(child.id, matched+1);
        used[v] = false;
    }
  }

  for(size_t i = st; i < en; i++) {
    Vertex u = query->GetNeighbor(i);
    qdd[u]++;
  }
  check[id] = 0;
}