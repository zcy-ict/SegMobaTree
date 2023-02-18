#include "mobatree.h"

using namespace std;

int MobaTree::Create(vector<Rule> &rules, bool insert) {
    node_num = 0;
    root = NULL;

    ip_mask[0][0] = 0;
    ip_mask[0][1] = 0;
    for (int i = 1; i <= 128; ++i) {
        ip_mask[i][0] = ip_mask[i - 1][0];
        ip_mask[i][1] = ip_mask[i - 1][1];
        if (i <= 64)
            ip_mask[i][0] |= 1ULL << (64 - i);
        else 
            ip_mask[i][1] |= 1ULL << (128 - i);
    }
    if (insert) {
        int rules_num = rules.size();
        for (int i = 0; i < rules_num; ++i)
            InsertRule(rules[i].ip, rules[i].prefix_len, rules[i].port);
    }
    return 0;
}

int MobaTree::InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port) {
    //printf("insert %016lx %016lx %u\n", rule.ip.high, rule.ip.low, rule.prefix_len);
    Ip ip_max = ip;
    ip_max.high |= ~ip_mask[prefix_len][0];
    ip_max.low |= ~ip_mask[prefix_len][1];
    MobaTreeNode *insert_node = MobaTreeNodeCreate(ip, ip_max, port, NULL);
    MobaTreeNodeInsertNodeLevel(root, insert_node);
    ++node_num;
    return 0;
}

int MobaTree::DeleteRule(Ip ip, uint8_t prefix_len) {
    //printf("delete %016lx %016lx %u\n", rule.ip.high, rule.ip.low, rule.prefix_len);
    Ip ip_max = ip;
    ip_max.high |= ~ip_mask[prefix_len][0];
    ip_max.low |= ~ip_mask[prefix_len][1];
    MobaTreeNodeDeleteNodeLevel(root, ip, ip_max);
    --node_num;
    return 0;
}

nexthop_t MobaTree::LookupV6(Ip ip) {
    int ans = 0;
    MobaTreeNode *node = root;
    while (node != NULL) {
        if (ip < node->ip_min) {
            node = node->child[0];
        } else if (ip > node->ip_max) {
            node = node->child[1];
        } else {
            ans = node->port;
            node = node->sub_node;
        }
    }
    return ans;
}

nexthop_t MobaTree::LookupV6_MemoryAccess(Ip ip, ProgramState *program_state) {
    int ans = 0;
    MobaTreeNode *node = root;
    while (node != NULL) {
            program_state->memory_access.AddNum();
        if (ip < node->ip_min) {
            node = node->child[0];
        } else if (ip > node->ip_max) {
            node = node->child[1];
        } else {
            ans = node->port;
            node = node->sub_node;
        }
    }
    return ans;
}

uint64_t MobaTree::MemorySize() {
    uint32_t size = sizeof(MobaTree);
    size += sizeof(MobaTreeNode) * node_num;
    return size;
}

int MobaTree::Free() {
    if (root != NULL)
        root->Free();
    return 0;
}

int MobaTree::Test(void *ptr) {
    return 0;
}