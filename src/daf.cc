#include "daf.h"
using namespace std;

DAF::DAF() {}
DAF::~DAF() {}

void DAF::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  this->data = &data;       /* data */
  this->query = &query;     /* query */
  this->cs = &cs;           /* candidate set */
  size_t num_qv = query.GetNumVertices();
  
  // first line
  cout << "t " << num_qv << "\n";
  initialize();

  // find root
  Vertex root = 0;
  for(size_t i=1; i<num_qv; i++) {
    if(cs.GetCandidateSize(i) < cs.GetCandidateSize(root))
      root = i;
  }
  
  // find query dag and set degree
  fill(check.begin(), check.end(), 0);
  fill(qdd.begin(), qdd.end(), 0);
  for(size_t i=0; i<num_qv; i++) children[i].clear();
  dag(root);

  // candidate size order backtracking
  fill(used.begin(), used.end(), false);
  fill(check.begin(), check.end(), 0);
  for(size_t i=0; i<cs.GetCandidateSize(root); i++){
    Vertex v = cs.GetCandidate(root, i);
    M[root] = v;
    used[v] = true;
    candidate_size_order(root, 1);
    used[v] = false;
  }
}

/* 
 * initialize class variables
 */
void DAF::initialize() {
  size_t num_qv = query->GetNumVertices();
  M.resize(num_qv);
  qdd.resize(num_qv);
  check.resize(num_qv);
  children.resize(num_qv);
  C_m.resize(num_qv);
  used.resize(data->GetNumVertices());

  // copy cs to C_m
  for(size_t i = 0; i < num_qv; i++) {
    C_m[i].clear();
    for(size_t j = 0; j < cs->GetCandidateSize(i); j++) {
      Vertex u = cs->GetCandidate(i, j);
      C_m[i].insert(u);
    }
  }
}

/*
 * Creage query DAG in BFS order.
 * and update qdd ; number of incomming edges
 */
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
      if(check[u] == 2){    /* there is u -> id edge in DAG */
        qdd[id]++;
        children[u].push_back(id);
      }
      if(check[u] == 0) { 
        q.push(u);
        check[u] = 1;
      }
    }
  }
}

/*
 * Backtracking with candidate_size order
 * We call this function after match id
 */
void DAF::candidate_size_order(Vertex id, size_t matched) {
  if(matched == query->GetNumVertices()){     /* find one matches */
    Backtrack::printMatch(M);
    return;
  }
  // visited
  check[id] = 1;

  size_t st = query->GetNeighborStartOffset(id);
  size_t en = query->GetNeighborEndOffset(id);
  vector<set<Vertex> > save_del(query->GetNumVertices());   /* need to delete cause of M[id] */
  set<Vertex>::iterator it;

  for(size_t i = st; i < en; i++) {
    Vertex u = query->GetNeighbor(i);
    qdd[u]--;
    if(qdd[u]<0) continue;

    // qdd[u] >= 0 means u is children of id in DAG order
    for(it = C_m[u].begin(); it != C_m[u].end(); ++it) {
      Vertex v = *it;
      // check if v is connected with parents
      if(!data->IsNeighbor(v, M[id]))
        save_del[u].insert(v);
    }

    set<Vertex> result;
    set_difference(C_m[u].begin(), C_m[u].end(), save_del[u].begin(), save_del[u].end(), std::inserter(result, result.end()));
    C_m[u] = result;
  }
  for(size_t i = 0; i < children[id].size(); i++) {
    Vertex u = children[id][i];
    if(C_m[u].find(M[id]) != C_m[u].end()){
      save_del[u].insert(M[id]);
    }
  }

  // set Difference C_m, save_del
  for(size_t i = 0; i < query->GetNumVertices(); i++) {
      if(save_del[i].size()) {
        set<Vertex> result;
        set_difference(C_m[i].begin(), C_m[i].end(), save_del[i].begin(), save_del[i].end(), std::inserter(result, result.end()));
        C_m[i] = result;
      }
  }
  
  // Find minimum candidate size
  Vertex u = -1;
  size_t m = data->GetNumVertices() + 100;
  for(size_t i = 0; i < query->GetNumVertices(); i++) {
    if(qdd[i] == 0 && check[i] == 0){           /* The condition can be a next nodes */
      if(C_m[i].size() != 0 && m > C_m[i].size()){
        u = i;
        m = C_m[i].size();
      }
    }
  }

  if(u != -1)
    // Backtrack with u
    for(it = C_m[u].begin(); it != C_m[u].end(); ++it){
        Vertex v = *it;
        if(used[v]) continue;
        M[u] = v;
        used[v] = true;
        candidate_size_order(u, matched+1);
        used[v] = false;        /* Return to original value. */
    }

  // return to original C_m
  // set Union C_m and save_del
  for(size_t i = 0; i < query->GetNumVertices(); i++) {
      if(save_del[i].size()) {
        set<Vertex> result;
        set_union(C_m[i].begin(), C_m[i].end(), save_del[i].begin(), save_del[i].end(), std::inserter(result, result.end()));
        C_m[i] = result;
      }
  }
  // return to original qdd
  for(size_t i = st; i < en; i++) {
    u = query->GetNeighbor(i);
    qdd[u]++;
  }
  // unvisited
  check[id] = 0;
}