#include "optimize_da.h"
using namespace std;

OPDA::OPDA() {}
OPDA::~OPDA() {}

/**
 * @brief initializes basic arrays and vectors 
 * 
 * @return void
*/
void OPDA::initialize(const Graph &data, const Graph &query, const CandidateSet &cs) {
    this->data = &data;
    this->query = &query;
    this->cs = &cs;
    std::cout << "t " << query.GetNumVertices() << "\n";

    size_t num_qv = query.GetNumVertices();

    M.resize(num_qv, 0);
    cands.resize(num_qv);
    cand_rev.resize(data.GetNumVertices());
    for(size_t i=0 ; i<num_qv ; i++)
    {
        cands[i].clear();
        size_t num_cand = cs.GetCandidateSize(i);
        for(size_t j=0 ; j<num_cand ; j++)
        {
            size_t cand = cs.GetCandidate(i, j);
            cands[i].insert(cand);
            cand_rev[cand].push_back(i);
        }
    }
    query_dag.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) query_dag[i].clear();
    query_dag_rev.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) query_dag_rev[i].clear();
    pathex.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) pathex[i].reset();
    visit.resize(data.GetNumVertices(), 0);
    decided.reset(); qdid.resize(num_qv, 0);
}

/**
 * @brief selects the root of the query DAG
 * 
 * @return Vertex
*/
void OPDA::find_root(void) {
    size_t cur = 0;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=1 ; i<num_qv ; i++) 
        if(cands[cur].size() * query->GetDegree(i) 
           > cands[i].size() * query->GetDegree(cur)) cur = i;
    this->root = cur;
}

/**
 * @brief set root
 * 
 * @return void
*/
void OPDA::set_root(Vertex v) {
    this->root = v;
}

/**
 * @brief builds the query DAG using BFS
 * 
 * @return void
*/
void OPDA::DAG_CREATE(void) {
    queue<Vertex> Q;
    size_t num_qv = query->GetNumVertices();
    vector<size_t> dist(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) dist[i] = num_qv + 100;
    dist[root] = 0; Q.push(root);
    while(!Q.empty())
    {
        Vertex cur = Q.front(); Q.pop();
        size_t st = query->GetNeighborStartOffset(cur);
        size_t en = query->GetNeighborEndOffset(cur);
        for(size_t i=st ; i<en ; i++)
        {
            size_t nxt = query->GetNeighbor(i);
            if(dist[nxt] > dist[cur] + 1) {
                dist[nxt] = dist[cur] + 1;
                Q.push(nxt);
            }
        }
    }
    for(size_t i=0 ; i<num_qv ; i++) {
        size_t st = query->GetNeighborStartOffset(i);
        size_t en = query->GetNeighborEndOffset(i);
        for(size_t j = st ; j < en ; j++) {
            size_t nxt = query->GetNeighbor(j);
            if(dist[i] < dist[nxt] || (dist[i] == dist[nxt] && i < nxt)) {
                query_dag[i].push_back(nxt); qdid[nxt]++;
                query_dag_rev[nxt].push_back(i);
            }
        }
    }
}

/**
 * @brief preprocess computation regarding parent relations
 * 
 * @return void
*/
void OPDA::DAG_preprocess(void) {
    // we do this via DAG DP with topological sorting
    queue<Vertex> Q;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) if(qdid[i] == 0) Q.push(i);
    while(!Q.empty())
    {
        Vertex cur = Q.front(); Q.pop();
        for(size_t i=0 ; i<query_dag[cur].size() ; i++) {
            Vertex nxt = query_dag[cur][i];
            qdid[nxt]--; pathex[nxt].set(cur);
            for(size_t j=0 ; j<num_qv ; j++) if(pathex[cur].test(j)) pathex[nxt].set(j);
            if(qdid[nxt] == 0) Q.push(nxt);
        }
    }
}

/**
 * @brief main backtracking framework
 * 
 * @return void
*/
void OPDA::perf_backtrack(size_t matched) {
    if(matched == M.size()) {
        Backtrack::printMatch(M);
        return;
    }
    Vertex nxt = nxt_extend(matched);
    if(nxt == -1) return;
    set<Vertex>::iterator it1, it2;
    vector< vector<Vertex> > lostcand;
    lostcand.resize(query->GetNumVertices()); 
    for(it1 = cands[nxt].begin() ; it1 != cands[nxt].end() ; it1++) {
        Vertex match = (*it1); decided.set(nxt);
        M[nxt] = match; visit[match] = 1;
        // losing candidates due to C_M(u) definition
        for(size_t j=0 ; j<query_dag[nxt].size() ; j++) {
            Vertex des = query_dag[nxt][j];
            // cands[des] <- cands[des] \cap N(match)
            for(it2 = cands[des].begin() ; it2 != cands[des].end() ; ) {
                Vertex cand = (*it2);
                if(!data->IsNeighbor(match, cand)) {
                    lostcand[des].push_back(cand); it2 = cands[des].erase(it2);
                }
                else { it2++; }
            }
        }
        // losing candidates due to match being taken away
        for(size_t j=0 ; j<cand_rev[match].size() ; j++) {
            Vertex loc = cand_rev[match][j];
            if(cands[loc].count(match) && !decided.test(loc)) {
                cands[loc].erase(match);
                lostcand[loc].push_back(match);
            }
        }
        perf_backtrack(matched + 1);
        // restore everything
        decided.reset(nxt); visit[match] = 0;
        for(size_t i=0 ; i<lostcand.size() ; i++) {
            for(size_t j=0 ; j<lostcand[i].size() ; j++) {
                cands[i].insert(lostcand[i][j]);
            }
            lostcand[i].clear();
        }
    }
}

/**
 * @brief next vertrex to match 
 * 
 * @return Vertex
*/
Vertex OPDA::nxt_extend(size_t matched) {
    if(matched == 0) return root;
    // now work...
    Vertex ret = -1; 
    size_t cursz = 10000000;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(decided.test(i)) continue;
        if((decided & pathex[i]) != pathex[i]) continue;
        if(cands[i].size() < cursz) {
            cursz = cands[i].size();
            if(cands[i].size() == 0) return -1;
            ret = i;
        }
    }
    return ret;
}

/**
 * @brief get root
 * 
 * @return Vertex
*/
Vertex OPDA::get_root(void) {
    return root;
}