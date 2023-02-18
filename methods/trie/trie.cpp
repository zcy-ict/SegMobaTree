#include "trie.h"

using namespace std;

TrieNode* TrieNode::Create(nexthop_t _port, bool _solid_node) {
    TrieNode* trie_node = (TrieNode*)malloc(sizeof(TrieNode));
    trie_node->child[0] = NULL;
    trie_node->child[1] = NULL;
    trie_node->port = _port;
    trie_node->solid_node = _solid_node;
    return trie_node;
}

int TrieNode::CountNum() {
    int num = 1;
    for (int i = 0; i < 2; ++i)
        if (child[i] != NULL)
            num += child[i]->CountNum();
    return num;
}

void TrieNode::Free() {
    free(this);
}

void TrieNode::FreeAll() {
    for (int i = 0; i < 2; ++i)
        if (child[i] != NULL)
            child[i]->FreeAll();
    free(this);
}

int Trie::Create(vector<Rule> &rules, bool insert) {
    if (version != 4 && version != 6) {
        printf("version should be 4 or 6\n");
        exit(1);
    }
    root = root->Create(0, true);
    solid_node_num = 1;

    if (insert) {
        int rules_num = rules.size();
        for (int i = 0; i < rules_num; ++i)
            InsertRule(rules[i].ip, rules[i].prefix_len, rules[i].port);
    }
    return 0;
}

int Trie::Update(Ip ip, uint8_t prefix_len, nexthop_t port, int operation) {
    if (prefix_len == 0) {
        printf("prefix can not be 0\n");
        exit(1);
    }
    TrieNode *pre_node = NULL;
    TrieNode *node = root;
    pre_nodes[0] = node;
    pre_nodes_num = 1;
    for (int i = 0; i < prefix_len; ++i) {
        if (node->solid_node)
            pre_node = node;
        int bit = ip.GetBit(i, version);
        if (node->child[bit] == NULL)
            node->child[bit] = node->child[bit]->Create(0, false);
        node = node->child[bit];
        
        pre_nodes[pre_nodes_num] = node;
        ++pre_nodes_num;
    }

    if (operation == INSERT) {
        if (node->solid_node) {
            printf("exist rule\n");
            exit(1);
        }
        node->port = port;
        node->solid_node = true;
    } else if (operation == DELETE) {
        if (!node->solid_node) {
            printf("no such rule\n");
            exit(1);
        }
        node->port = 0;
        node->solid_node = false;
        for (int i = pre_nodes_num - 1; i > 0; --i) {
            if(pre_nodes[i]->child[0] == NULL && pre_nodes[i]->child[1] == NULL && !pre_nodes[i]->solid_node) {
                int bit = pre_nodes[i] == pre_nodes[i - 1]->child[0] ? 0 : 1;
                pre_nodes[i - 1]->child[bit] = NULL;
                free(pre_nodes[i]);
            } else {
                break;
            }
        }
    }
    return 0;
}
     
int Trie::InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port) {
    return Update(ip, prefix_len, port, INSERT);
}

int Trie::DeleteRule(Ip ip, uint8_t prefix_len) {
    return Update(ip, prefix_len, 0, DELETE);
}

nexthop_t Trie::LookupV4(uint32_t ip) {
    TrieNode *node = root;
    nexthop_t port = root->port;
    for (int i  = 0; i < 32; ++i) {
        int bit = (ip >> (31 - i)) & 1;
        node = node->child[bit];
        if (node == NULL)
            return port;
        if (node->solid_node)
            port = node->port;
    }
    return port;
}

nexthop_t Trie::LookupV4_MemoryAccess(uint32_t ip, ProgramState *program_state) {
    program_state->memory_access.ClearNum();

    TrieNode *node = root;
    nexthop_t port = root->port;
    for (int i  = 0; i < 32; ++i) {
        program_state->memory_access.AddNum();
        program_state->cache_access[(uint64_t)node / 64]++;
        int bit = (ip >> (31 - i)) & 1;
        node = node->child[bit];
        if (node == NULL) {
            program_state->memory_access.Update();
            return port;
        }
        if (node->solid_node)
            port = node->port;
    }
    program_state->memory_access.Update();
    return port;
}

nexthop_t Trie::LookupV6(Ip ip) {
    TrieNode *node = root;
    nexthop_t port = root->port;
    for (int i  = 0; i < 128; ++i) {
        int bit = ip.GetBit(i, 6);
        node = node->child[bit];
        if (node == NULL)
            return port;
        if (node->solid_node)
            port = node->port;
    }
    return port;
}

nexthop_t Trie::LookupV6_MemoryAccess(Ip ip, ProgramState *program_state) {
    TrieNode *node = root;
    nexthop_t port = root->port;
    for (int i  = 0; i < 128; ++i) {
        program_state->memory_access.AddNum();
        program_state->cache_access[(uint64_t)node / 64]++;
        int bit = ip.GetBit(i, 6);
        node = node->child[bit];
        if (node == NULL)
            return port;
        if (node->solid_node)
            port = node->port;
    }
    return port;
}

uint64_t Trie::MemorySize() {
    uint64_t memory_size = sizeof(Trie);
    memory_size += root->CountNum() * sizeof(TrieNode);
    return memory_size;
}

int Trie::Free() {
    if (root != NULL)
        root->FreeAll();
    return 0;
}

int Trie::Test(void *ptr) {
    return 0;
}