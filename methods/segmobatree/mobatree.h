#ifndef  MOBATREE_H
#define  MOBATREE_H

#include "../../elementary.h"
#include "mobatreenode.h"

using namespace std;

class MobaTree : public Classifier {
public:
    int Create(vector<Rule> &rules, bool insert);
    
    int InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port);
    int DeleteRule(Ip ip, uint8_t prefix_len);

    nexthop_t LookupV4(uint32_t ip) {return 0;};
    nexthop_t LookupV4_MemoryAccess(uint32_t ip, ProgramState *program_state) {return 0;};

    nexthop_t LookupV6(Ip ip);
    nexthop_t LookupV6_MemoryAccess(Ip ip, ProgramState *program_state);
    nexthop_t LookupV6_2(Ip ip) { return 0; };
    nexthop_t LookupV6_2_MemoryAccess(Ip ip, ProgramState *program_state) { return 0; };

    uint64_t MemorySize();
    int Free();
    int Test(void *ptr);

private:
    int node_num;
    MobaTreeNode *root;

    uint64_t ip_mask[129][2];
};

#endif 