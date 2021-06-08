#include "ignore_dag.h"
using namespace std;

IgnoreDAG::IgnoreDAG() {}
IgnoreDAG::~IgnoreDAG() {}

/**
 * @brief initializes basic arrays and vectors 
 * 
 * @return void
*/
void IgnoreDAG::initialize(const Graph &data, const Graph &query, const CandidateSet &cs) {
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
    query_edge.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) {
        size_t st = query.GetNeighborStartOffset(i);
        size_t en = query.GetNeighborEndOffset(i);
        for(size_t j = st ; j < en ; j++) {
            size_t nxt = query.GetNeighbor(j);
            query_edge[i].push_back(nxt); 
            query_edge[nxt].push_back(i);
        }
    }
    visit.resize(data.GetNumVertices(), 0);
    decided.reset(); 
}

/**
 * @brief set root
 * 
 * @return void
*/
void IgnoreDAG::set_root(Vertex v) {
    this->root = v;
}

/**
 * @brief main backtracking framework
 * 
 * @return void
*/
void IgnoreDAG::perf_backtrack(size_t matched) {
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
        for(size_t j=0 ; j<query_edge[nxt].size() ; j++) {
            Vertex des = query_edge[nxt][j];
            if(decided.test(des)) continue;
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
Vertex IgnoreDAG::nxt_extend(size_t matched) {
    // take the undecided vertex with minimum candidate size, disregarding DAG
    Vertex ret = -1; 
    size_t cursz = 10000000;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(decided.test(i)) continue;
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
Vertex IgnoreDAG::get_root(void) {
    return root;
}