#include "elpsm.h"
using namespace std;

ELPSM::ELPSM() {}
ELPSM::~ELPSM() {}

/**
 * @brief initializes basic arrays and vectors 
 * 
 * @return void
*/
void ELPSM::initialize(const Graph &data, const Graph &query, const CandidateSet &cs) {
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
            cands[i].insert(cs.GetCandidate(i, j));
    }
    query_dag.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) query_dag[i].clear();
    query_dag_rev.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) query_dag_rev[i].clear();
    merged.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) merged[i].clear();
    pathex.resize(num_qv);
    for(size_t i=0 ; i<num_qv ; i++) pathex[i].reset();
    visit.resize(data.GetNumVertices(), 0);
    decided.reset();
    fake.reset();
    largeleaf.reset();
    qdid.resize(num_qv, 0);
}

/**
 * @brief selects the root of the query DAG
 * 
 * @return Vertex
*/
void ELPSM::find_root(void) {
    size_t cur = 0;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=1 ; i<num_qv ; i++) 
        if(cands[cur].size() * query->GetDegree(i) 
           > cands[i].size() * query->GetDegree(cur)) cur = i;
    this->root = cur;
}

/**
 * @brief builds the query DAG using BFS
 * 
 * @return void
*/
void ELPSM::DAG_CREATE(void) {
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
 * @brief merges equivalent leaf vertices of the query DAG
 * 
 * @return void
*/
void ELPSM::DAG_merge(void) {
    map< pair<Vertex, Label>, vector<Vertex> > merger;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(qdid[i] == 1 && query_dag[i].empty()) { // i is a leaf!
            size_t parent = query_dag_rev[i][0];
            pair<Vertex, Label> app = make_pair(parent, query->GetLabel(i));
            if(merger.find(app) == merger.end()) {
                vector<Vertex> val; val.push_back(i);
                merger[app] = val;
            }
            else {
                merger[app].push_back(i);
            }
        }   
    }
    map< pair<Vertex, Label>, vector<Vertex> >::iterator it;
    for(it = merger.begin() ; it != merger.end() ; it++) {
        if(it->second.size() >= 2) {
            vector<Vertex> newneighbor;
            merged[it->second[0]] = it->second;
            Vertex parent = it->first.first;
            Label label = it->first.second;
            size_t cnt = 0;
            newneighbor.resize(query_dag[parent].size() - it->second.size() + 1);
            for(size_t i=0 ; i<query_dag[parent].size() ; i++) {
                Vertex nxt = query_dag[parent][i];
                if(query->GetLabel(nxt) == label && qdid[nxt] == 1 && query_dag[nxt].empty()) continue;
                newneighbor[cnt++] = nxt;
            }
            for(size_t i=1 ; i<it->second.size() ; i++) {
                fake.set(it->second[i]);
            }
            largeleaf.set(it->second[0]);
            newneighbor[cnt++] = it->second[0];
            query_dag[parent] = newneighbor;
        }
    }
    for(size_t i=0 ; i<num_qv ; i++) {
        if(fake.test(i)) continue;
        size_t sz = cs->GetCandidateSize(i);
        for(size_t j=0 ; j<sz ; j++) {
            size_t cand = cs->GetCandidate(i, j);
            cand_rev[cand].push_back(i);
        }
    }
}

/**
 * @brief preprocess computation regarding parent relations
 * 
 * @return void
*/
void ELPSM::DAG_preprocess(void) {
    // we do this via DAG DP with topological sorting
    queue<Vertex> Q;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) if(qdid[i] == 0 && !fake.test(i)) Q.push(i);
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
void ELPSM::perf_ELPSM(size_t matched) {
    if(matched == M.size()) {
        print_matches(0);
        return;
    }
    Vertex nxt = nxt_extend(matched);
    if(nxt == -1) return;
    // cerr << matched << " " << nxt << endl;
    // cerr << matched << " " << nxt << " " << cands[nxt].size() << endl;
    // view_vertex(nxt);
    if(!largeleaf.test(nxt)) {
        set<Vertex>::iterator it1, it2;
        vector< vector<Vertex> > lostcand;
        lostcand.resize(query->GetNumVertices()); 
        for(it1 = cands[nxt].begin() ; it1 != cands[nxt].end() ; it1++) {
            Vertex match = (*it1); decided.set(nxt);
            assert(visit[match] == 0);
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
            // cerr << "matched " << nxt << " with " << match << endl;
            perf_ELPSM(matched + 1);
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
    else {
        // cands[nxt] vs merged[nxt].size()
        vector<Vertex> cand; cand.resize(cands[nxt].size());
        set<Vertex>::iterator it; size_t cnt = 0; decided.set(nxt);
        for(it = cands[nxt].begin() ; it != cands[nxt].end() ; it++) cand[cnt++] = (*it);
        fast_leaf_match(matched, nxt, cand, 0, 0); decided.reset(nxt);
    }
}

/**
 * @brief we are now matching merged leafs fast : note that we don't need to care about descendants here
 * 
 * @return void 
*/
void ELPSM::fast_leaf_match(size_t matched, Vertex whi, std::vector<Vertex> &cand, size_t idx, size_t this_matched) {
    if(merged[whi].size() == this_matched) {
        perf_ELPSM(matched);
        return;
    }
    for(size_t nxt = idx ; nxt < cand.size() ; nxt++) {
        if(nxt >= cand.size() + 1 + this_matched - merged[whi].size()) break;
        Vertex match = cand[nxt]; visit[match] = 1;
        M[merged[whi][this_matched]] = match;
        set<Vertex> loss; loss.clear();
        for(size_t j=0 ; j<cand_rev[match].size() ; j++) {
            Vertex loc = cand_rev[match][j];
            if(cands[loc].count(match) && !decided.test(loc)) {
                cands[loc].erase(match);
                loss.insert(loc);
            }
        }
        fast_leaf_match(matched + 1, whi, cand, nxt + 1, this_matched + 1);
        set<Vertex>::iterator it; visit[match] = 0;
        for(it = loss.begin() ; it != loss.end() ; it++) cands[(*it)].insert(match);
    }
} 

/**
 * @brief next vertrex to match 
 * 
 * @return Vertex
*/
Vertex ELPSM::nxt_extend(size_t matched) {
    if(matched == 0) return root;
    // now work...
    Vertex ret_1 = -1, ret_2 = -1; 
    size_t cursz_1 = 10000000, cursz_2 = 10000000;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(fake.test(i) || decided.test(i)) continue;
        if((decided & pathex[i]) != pathex[i]) continue;
        if(largeleaf.test(i) && cands[i].size() < merged[i].size()) return -1;
        if(largeleaf.test(i) && cands[i].size() == merged[i].size()) return i;
        if(largeleaf.test(i)) {
                if(cands[i].size() < cursz_2) {
                cursz_2 = cands[i].size();
                if(cands[i].size() == 0) return -1;
                ret_2 = i;
            }
        }
        else {
            if(cands[i].size() < cursz_1) {
                cursz_1 = cands[i].size();
                if(cands[i].size() == 0) return -1;
                ret_1 = i;
            }
        }
    }
    if(ret_1 == -1 && ret_2 == -1) return -1;
    if(ret_1 == -1) return ret_2;
    return ret_1;
}

/**
 * @brief printing matches
 * 
 * @return void
*/
void ELPSM::print_matches(size_t cur) {
    if(cur == query->GetNumVertices()) {
        Backtrack::printMatch(M);
        return;
    }
    if(!largeleaf.test(cur)) print_matches(cur + 1);
    else 
    {
        size_t bunch = merged[cur].size();
        vector<Vertex> partial_match; partial_match.resize(bunch);
        for(size_t i=0 ; i<bunch ; i++) partial_match[i] = M[merged[cur][i]];
        do
        {
            for(size_t i=0 ; i<bunch ; i++) M[merged[cur][i]] = partial_match[i];
            print_matches(cur + 1);
        } while(next_permutation(partial_match.begin(), partial_match.end()));
        for(size_t i=0 ; i<bunch ; i++) M[merged[cur][i]] = partial_match[bunch - 1 - i];
    }
}

/**
 * @brief debug : print details of a vertex
 * 
 * @return void
*/
void ELPSM::view_vertex(Vertex cur) {
    cerr << "Viewing " << cur << endl;
    cerr << decided.test(cur) << " " << largeleaf.test(cur) << " " << fake.test(cur) << " " << endl;
    set<Vertex>::iterator it;
    for(it = cands[cur].begin() ; it != cands[cur].end() ; it++) cerr << (*it) << " ";
    cerr << endl;
}

/**
 * @brief debug : display the DAG merge
 * 
 * @return void
 * 
*/
void ELPSM::DAG_display(void) {
    cerr << "root is at " << root << endl;
    size_t num_qv = query->GetNumVertices();
    for(size_t i=0 ; i<num_qv ; i++) {
        if(largeleaf.test(i)) {
            cerr << "merge at " << i << ": " << endl;
            for(size_t j=0 ; j<merged[i].size() ; j++) {
                cerr << merged[i][j] << " ";
            }
            cerr << endl;
        }
    }
}