#ifndef  SEGMOBATREE_H
#define  SEGMOBATREE_H

#include "../../elementary.h"
#include "../trie/trie.h"
#include "mobatreenode.h"
#include "dpranges.h"

using namespace std;

struct SegMobaTreeHashTable {

    MobaTreeNode **buckets;
    uint32_t mask;
    uint32_t size;
    uint32_t n;
    uint32_t prefix_len;
    uint32_t reduce_bits;
};

class SegMobaTree : public Classifier {
public:
    int Create(vector<Rule> &rules, bool insert);
    
    int InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port);
    int DeleteRule(Ip ip, uint8_t prefix_len);
    
    nexthop_t LookupV4(uint32_t ip) {return 0;};
    nexthop_t LookupV4_MemoryAccess(uint32_t ip, ProgramState *program_state) {return 0;};

    nexthop_t LookupV6(Ip ip);
    nexthop_t LookupV6_MemoryAccess(Ip ip, ProgramState *program_state);
    nexthop_t LookupV6_2(Ip ip) {return 0;};
    nexthop_t LookupV6_2_MemoryAccess(Ip ip, ProgramState *program_state) {return 0;};

    uint64_t MemorySize();
    int Free();
    int Test(void *ptr);

    int range_type; // 0 dp, 1 hi

private:

    int tuples_num;
    SegMobaTreeHashTable tuples[129];
    MobaTreeNode *mbt_node;  // last_layer

    int node_num;


    uint64_t ip_mask[129][2];
};

#endif