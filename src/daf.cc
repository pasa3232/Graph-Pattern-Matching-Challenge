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
  children.resize(num_qv);
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
  for(size_t i=0; i<num_qv; i++) children[i].clear();
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
  while(!q.empty()){
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
  //cerr << "Backtrack : " << matched << " " << id << " " << M[id] << endl;
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
    vector<Vertex> valid_cs;
    for(size_t j = 0; j < cs->GetCandidateSize(u); j++) {
      Vertex v = cs->GetCandidate(u, j);
      // check if v is already used
      if(used[v]) continue;

      // check if v is connected with parents
      bool flag = true;
      for(Vertex p : parents[u])
        if(!data->IsNeighbor(v, M[p])) flag = false;
      
      if(flag) valid_cs.push_back(v);
    }

    if(!valid_cs.empty()){
      children[u] = valid_cs;
      //cerr << "t: " << u << " " << valid_cs.size() << endl; 
    }
  }
  
  Vertex u = -1;
  size_t m = data->GetNumVertices() + 100;
  for(size_t i = 0; i < query->GetNumVertices(); i++) {
    if(children[i].size() == 0 || check[i]) continue;
    if(children[i].size() < m) {
      m = children[i].size();
      u = i;
    }
  }
  //cerr << "u, m : " << u << " " << m << endl;

  if(u != -1)
    for(size_t i = 0; i < children[u].size(); i++){
        Vertex v = children[u][i];
        M[u] = v;
        used[v] = true;
        candidate_size_order(u, matched+1);
        used[v] = false;
    }

  for(size_t i = st; i < en; i++) {
    Vertex u = query->GetNeighbor(i);
    if(qdd[u] == 0) children[u].clear();
    qdd[u]++;
  }
  check[id] = 0;
}