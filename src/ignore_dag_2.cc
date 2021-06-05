#include "ignore_dag_2.h"
using namespace std;

IgnoreDAG2::IgnoreDAG2() {}
IgnoreDAG2::~IgnoreDAG2() {}

/**
 * @brief initializes basic arrays and vectors 
 * 
 * @return void
*/
void IgnoreDAG2::initialize(const Graph &data, const Graph &query, const CandidateSet &cs) {
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
    undecided_neighbor.resize(num_qv);
    query_edge.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) {
        size_t st = query.GetNeighborStartOffset(i);
        size_t en = query.GetNeighborEndOffset(i);
        for(size_t j = st ; j < en ; j++) {
            size_t nxt = query.GetNeighbor(j);
            query_edge[i].push_back(nxt); 
            undecided_neighbor[i]++;
            query_edge[nxt].push_back(i);
            undecided_neighbor[nxt]++;
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
void IgnoreDAG2::set_root(Vertex v) {
    this->root = v;
}

/**
 * @brief main backtracking framework
 * 
 * @return void
*/
void IgnoreDAG2::perf_backtrack(size_t matched) {
    if(matched == M.size()) {
        Backtrack::printMatch(M);
        return;
    }
    Vertex nxt = nxt_extend(matched);
    if(nxt == -1) return;
    set<Vertex>::iterator it1, it2;
    vector< pair<Vertex, Vertex> > lostcand;
    for(it1 = cands[nxt].begin() ; it1 != cands[nxt].end() ; it1++) {
        Vertex match = (*it1); decided.set(nxt);
        M[nxt] = match; visit[match] = 1;
        // losing candidates due to C_M(u) definition
        for(size_t j=0 ; j<query_edge[nxt].size() ; j++) {
            Vertex des = query_edge[nxt][j];
            undecided_neighbor[des]--;
            if(decided.test(des)) continue;
            // cands[des] <- cands[des] \cap N(match)
            for(it2 = cands[des].begin() ; it2 != cands[des].end() ; ) {
                Vertex cand = (*it2);
                if(!data->IsNeighbor(match, cand)) {
                    lostcand.push_back(make_pair(des, cand));
                    it2 = cands[des].erase(it2);
                }
                else { it2++; }
            }
        }
        // losing candidates due to match being taken away
        for(size_t j=0 ; j<cand_rev[match].size() ; j++) {
            Vertex loc = cand_rev[match][j];
            if(cands[loc].count(match) && !decided.test(loc)) {
                cands[loc].erase(match);
                lostcand.push_back(make_pair(loc, match));
            }
        }
        // cerr << "matched " << nxt << " with " << match << endl;
        perf_backtrack(matched + 1);
        // restore everything
        decided.reset(nxt); visit[match] = 0;
        for(size_t j=0 ; j<query_edge[nxt].size() ; j++) {
            Vertex des = query_edge[nxt][j];
            undecided_neighbor[des]++;
        }
        for(size_t i=0 ; i<lostcand.size() ; i++) {
            cands[lostcand[i].first].insert(lostcand[i].second);
        }
        lostcand.clear();
    }
}

/**
 * @brief next vertrex to match 
 * 
 * @return Vertex
*/
Vertex IgnoreDAG2::nxt_extend(size_t matched) {
    // now work...
    Vertex ret = -1; 
    size_t cursz = 10000000;
    int cur_neighbor = 0;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(decided.test(i)) continue;
        if(cands[i].size() < cursz) {
            cursz = cands[i].size();
            cur_neighbor = undecided_neighbor[i];
            if(cands[i].size() == 0) return -1;
            ret = i;
        }
        else if(cands[i].size() == cursz && undecided_neighbor[i] > cur_neighbor) {
            ret = i;
            cur_neighbor = undecided_neighbor[i];
        }
    }
    return ret;
}

/**
 * @brief get root
 * 
 * @return Vertex
*/
Vertex IgnoreDAG2::get_root(void) {
    return root;
}