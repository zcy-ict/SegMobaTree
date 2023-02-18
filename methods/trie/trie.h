#ifndef  TRIE_H
#define  TRIE_H

#include "../../elementary.h"

using namespace std;

struct TrieNode {
    TrieNode *child[2];
    nexthop_t port;
    bool solid_node;

    TrieNode* Create(nexthop_t _port, bool _solid_node);
    int CountNum();
    void Free();
    void FreeAll();
};

class Trie : public Classifier {
public:
    int Create(vector<Rule> &rules, bool insert);
    
    int InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port);
    int DeleteRule(Ip ip, uint8_t prefix_len);

    nexthop_t LookupV4(uint32_t ip);
    nexthop_t LookupV4_MemoryAccess(uint32_t ip, ProgramState *program_state);

    nexthop_t LookupV6(Ip ip);
    nexthop_t LookupV6_MemoryAccess(Ip ip, ProgramState *program_state);
    nexthop_t LookupV6_2(Ip ip) { return 0; };
    nexthop_t LookupV6_2_MemoryAccess(Ip ip, ProgramState *program_state) {return 0;};

    uint64_t MemorySize();
    int Free();
    int Test(void *ptr);
// private:

    int Update(struct Ip ip, uint8_t prefix_len, nexthop_t port, int operation);

    TrieNode* root;
    int solid_node_num;

    TrieNode* pre_nodes[129];
    int pre_nodes_num;
};


#endif