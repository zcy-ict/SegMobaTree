#include "segmobatree.h"

using namespace std;


int SegMobaTree::Create(vector<Rule> &rules, bool insert) {
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

    node_num = 0;

    vector<RuleRange> ranges;
    if (range_type == DPRANGES)
        ranges = GetIpv6RuleRanges(rules);
    else if (range_type == HABIRANGES)
        ranges = GetHaBiRuleRanges(rules);

    tuples_num = ranges.size();
    for (int i = 0; i < tuples_num; ++i) {
        tuples[i].size = 16;
        while (tuples[i].size * 0.1 < ranges[i].prefix_num)
            tuples[i].size *= 2;
        tuples[i].mask = tuples[i].size - 1;
        tuples[i].buckets = (MobaTreeNode**)malloc(sizeof(MobaTreeNode*) * tuples[i].size);
        for (int j = 0; j < tuples[i].size; ++j)
            tuples[i].buckets[j] = NULL;
        // printf("%d %d\n", ranges[i].start, ranges[i].end);
        // printf("buckets %d\n", tuples[i].size);

        tuples[i].prefix_len = ranges[i].start;
        if (i == 0)
            tuples[i].reduce_bits = 128 - ranges[i].start;
        else
            tuples[i].reduce_bits = ranges[i - 1].start - ranges[i].start;
    }

    if (insert) {
        for (int i = 0; i < rules.size(); ++i)
            InsertRule(rules[i].ip, rules[i].prefix_len, rules[i].port);
    }
    return 0;
}

int SegMobaTree::InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port) {
    Ip ip_max = ip;
    ip_max.high |= ~ip_mask[prefix_len][0];
    ip_max.low |= ~ip_mask[prefix_len][1];
    MobaTreeNode *insert_node = MobaTreeNodeCreate(ip, ip_max, port, NULL);
    
    for (int i = 0; i < tuples_num; ++i) {
        if (prefix_len < tuples[i].prefix_len)
            continue;
        Ip ip2 = ip;
        ip2.RightShift(128 - tuples[i].prefix_len);
        uint32_t hash = (ip2.low >> 32) ^ ip2.low;
        uint32_t index = ((hash >> 16) ^ hash) & tuples[i].mask;
        MobaTreeNodeInsertNodeLevel(tuples[i].buckets[index], insert_node);
        break;
    }

    ++node_num;
    return 0;
}

int SegMobaTree::DeleteRule(Ip ip, uint8_t prefix_len) {
    Ip ip_max = ip;
    ip_max.high |= ~ip_mask[prefix_len][0];
    ip_max.low |= ~ip_mask[prefix_len][1];
    
    for (int i = 0; i < tuples_num; ++i) {
        if (prefix_len < tuples[i].prefix_len)
            continue;
        Ip ip2 = ip;
        ip2.RightShift(128 - tuples[i].prefix_len);
        uint32_t hash = (ip2.low >> 32) ^ ip2.low;
        uint32_t index = ((hash >> 16) ^ hash) & tuples[i].mask;
        MobaTreeNodeDeleteNodeLevel(tuples[i].buckets[index], ip, ip_max);
        break;
    }
    --node_num;

    return 0;
}

nexthop_t SegMobaTree::LookupV6(Ip ip) {
    int port = 0;
    uint32_t hash;
    Ip ip2 = ip;
    for (int i = 0; i < tuples_num; ++i) {
        ip2.RightShift(tuples[i].reduce_bits);
        hash = (ip2.low >> 32) ^ ip2.low;
        hash ^= hash >> 16;
        MobaTreeNode *node = tuples[i].buckets[hash & tuples[i].mask];
        while (node != NULL) {
            if (ip < node->ip_min) {
                node = node->child[0];
            } else if (ip > node->ip_max) {
                node = node->child[1];
            } else {
                port = node->port;
                node = node->sub_node;
            }
        }
        if (port > 0)
            return port;
    }
    return 0;
}

nexthop_t SegMobaTree::LookupV6_MemoryAccess(Ip ip, ProgramState *program_state) {
    int port = 0;
    uint32_t hash;
    Ip ip2 = ip;
    for (int i = 0; i < tuples_num; ++i) {
        program_state->memory_access.AddNum();
        ip2.RightShift(tuples[i].reduce_bits);
        hash = (ip2.low >> 32) ^ ip2.low;
        hash ^= hash >> 16;
        MobaTreeNode *node = tuples[i].buckets[hash & tuples[i].mask];
        while (node != NULL) {
            program_state->memory_access.AddNum();
            if (ip < node->ip_min) {
                node = node->child[0];
            } else if (ip > node->ip_max) {
                node = node->child[1];
            } else {
                port = node->port;
                node = node->sub_node;
            }
        }
        if (port > 0)
            return port;
    }
    return 0;
}

uint64_t SegMobaTree::MemorySize() {
    uint32_t size = sizeof(SegMobaTree);
    for (int i = 0; i < tuples_num; ++i)
        size += sizeof(MobaTreeNode*) * tuples[i].size;
    size += sizeof(MobaTreeNode) * node_num; 
    return size;
}

int SegMobaTree::Free() {
    for (int i = 0; i < tuples_num; ++i) {
        for (int j = 0; j < tuples[i].size; ++j)
            if (tuples[i].buckets[j] != NULL)
                tuples[i].buckets[j]->Free();
        free(tuples[i].buckets);
    }
    return 0;
}

int SegMobaTree::Test(void *ptr) {
    return 0;
}