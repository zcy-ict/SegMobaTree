#include "mobatreenode.h"

using namespace std;

MobaTreeNode* MobaTreeNodeInsertNode(MobaTreeNode* &root, MobaTreeNode *node);
MobaTreeNode* MobaTreeNodeDeleteNode(MobaTreeNode* &root, Ip &ip);


MobaTreeNode* MobaTreeNodeCreate(Ip &ip_min, Ip &ip_max, nexthop_t port, MobaTreeNode *parent) {
    MobaTreeNode *node = (MobaTreeNode*)malloc(sizeof(MobaTreeNode));
    node->ip_min = ip_min;
    node->ip_max = ip_max;
    node->port = port;
    node->color = RED;
    node->child[0] = NULL;
    node->child[1] = NULL;
    node->parent = parent;
    node->sub_node = NULL;
    return node;
}

bool Color(MobaTreeNode *node) {
    if (node == NULL)
        return BLACK;
    return node->color;
}

void ChangeColor(MobaTreeNode *node, bool _color) {
    if (node != NULL)
        node->color = _color;
}

void MobaTreeNode::Rotate(MobaTreeNode* &root, int d) { //left rotate d=0, right rotate d=1
    MobaTreeNode* y = child[!d];
    // b
    child[!d] = y->child[d];
    if (child[!d] != NULL)
        child[!d]->parent = this;
    
    // y->parent
    y->parent = parent;
    if (parent == NULL)
        root = y;
    else if (y->ip_min < y->parent->ip_min)
        y->parent->child[0] = y;
    else
        y->parent->child[1] = y;
    
    // y->child[d]
    y->child[d] = this;
    parent = y;
}

void MobaTreeNode::HandleReorient(MobaTreeNode* &root, Ip &ip) {
    //printf("HandleReorient %d\n", key);
    color = RED;
    ChangeColor(child[0], BLACK);
    ChangeColor(child[1], BLACK);

    if (Color(parent) == RED) {
        parent->parent->color = RED;
        int parent_pos = parent->ip_min < parent->parent->ip_min ? 0 : 1;
        int node_pos = ip < parent->ip_min ? 0 : 1;
        if (parent_pos == node_pos) {
            parent->color = BLACK;
            parent->parent->Rotate(root, !parent_pos);
        } else {
            parent->Rotate(root, !node_pos);
            color = BLACK;
            parent->Rotate(root, !parent_pos);
        }
    }
    root->color = BLACK;
}

MobaTreeNode* MobaTreeNodeInsertNode(MobaTreeNode* &root, MobaTreeNode *node) {
    MobaTreeNode *x = root;
    MobaTreeNode *p = NULL;

    while (x != NULL) {
        if (x->ip_min == node->ip_min) {
            printf("exist node insert %d same %d\n", node->port, x->port);
            exit(1);
        }
        if (Color(x->child[0]) == RED && Color(x->child[1]) == RED)
            x->HandleReorient(root, node->ip_min);
        p = x;
        if (node->ip_min < x->ip_min)
            x = x->child[0];
        else if (node->ip_min > x->ip_min)
            x = x->child[1];
        else {
            printf("exist k\n");
            return x;
        }
    }

    x = node;
    x->parent = p;
    x->color = RED;

    if (root == NULL)
        root = x;
    else if (node->ip_min < p->ip_min)
        p->child[0] = x;
    else
        p->child[1] = x;
    
    x->HandleReorient(root, node->ip_min);
    return x;
}

void MobaTreeNode::SwapKey(MobaTreeNode *node) {
    swap(sub_node, node->sub_node);
    swap(ip_min, node->ip_min);
    swap(ip_max, node->ip_max);
    swap(port, node->port);
}

MobaTreeNode* FindMin(MobaTreeNode *node) {
    if (node == NULL)
        return NULL;
    while (node->child[0] != NULL)
        node = node->child[0];
    return node;
}

MobaTreeNode* FindMax(MobaTreeNode *node) {
    if (node == NULL)
        return NULL;
    while (node->child[1] != NULL)
        node = node->child[1];
    return node;
}

// replace this with node
void MobaTreeNode::Transplant(MobaTreeNode* &root, MobaTreeNode* node) {
    if (parent == NULL)
        root = node;
    else if (this == parent->child[0])
        parent->child[0] = node;
    else
        parent->child[1] = node;
    if (node != NULL)
        node->parent = parent;
}

MobaTreeNode* MobaTreeNodeDeleteNode(MobaTreeNode* &root, Ip &ip) {
    if (root == NULL) {
        printf("No such node k\n");
        return NULL;
    }
    MobaTreeNode *delete_node = root;
    MobaTreeNode *x;
    int d;

    while (delete_node != NULL && delete_node->ip_min != ip) {
        if (ip < delete_node->ip_min)
            delete_node = delete_node->child[0];
        else
            delete_node = delete_node->child[1];
    }
    if (delete_node == NULL) {
        printf("No such node k\n");
        return NULL;
    }

    if (delete_node->child[0] != NULL && delete_node->child[1] != NULL) {
        MobaTreeNode *alternative = FindMin(delete_node->child[1]);
        delete_node->SwapKey(alternative);
        delete_node = alternative;
    }

    d = delete_node->child[0] != NULL ? 0 : 1;
    x = delete_node->child[d];
    MobaTreeNode* p = delete_node->parent;
    delete_node->Transplant(root, x);

    // x, p
    if (delete_node->color == BLACK) {
        while (x != root && Color(x) == BLACK) {
            d = x == p->child[0] ? 0 : 1;
            MobaTreeNode* w = p->child[!d];

            // 1 to 2 3 4
            if (w->color == RED) {
                w->color = BLACK;
                p->color = RED;
                p->Rotate(root, d);
                w = p->child[!d];
            }
            // 2 to break or continue
            if (Color(w->child[0]) == BLACK && Color(w->child[1]) == BLACK) {
                if (p->color == RED) {
                    p->color = BLACK;
                    w->color = RED;
                    break;
                } else {
                    w->color = RED;
                    x = p;
                    p = x->parent;
                    continue;
                }
            }
            // 3 to 4
            if (Color(w->child[!d]) == BLACK) {
                ChangeColor(w->child[d], BLACK);
                w->color = RED;
                w->Rotate(root, !d);
                w = p->child[!d];
            }

            // 4
            w->color = p->color;
            p->color = BLACK;
            ChangeColor(w->child[!d], BLACK);
            p->Rotate(root, d);
            x = root;
        }
        ChangeColor(x, BLACK);
    }

    delete_node->parent = NULL;
    delete_node->child[0] = NULL;
    delete_node->child[1] = NULL;

    return delete_node;
}

MobaTreeNode* FindLessOrEqual(MobaTreeNode *node, Ip &ip) {
    MobaTreeNode *ans = NULL;
    while (node != NULL) {
        //printf("%d\n", node->key);
        if (node->ip_min <= ip) {
            ans = node;
            node = node->child[1];
        } else {
            node = node->child[0];
        }
    }
    return ans;
}

MobaTreeNode* FindMoreOrEqual(MobaTreeNode *node, Ip &ip) {
    MobaTreeNode *ans = NULL;
    while (node != NULL) {
        //printf("%d\n", node->key);
        if (node->ip_min >= ip) {
            ans = node;
            node = node->child[0];
        } else {
            node = node->child[1];
        }
    }
    return ans;
}

void MobaTreeNode::Free() {
    free(this);
}

int MobaTreeNodeInsertNodeLevel(MobaTreeNode* &root, MobaTreeNode *insert_node) {
    //MobaTreeNode *insert_node = root->Create(rule.ip_min, rule.ip_max, rule.port, NULL);
    MobaTreeNode *node = NULL;

    node = FindLessOrEqual(root, insert_node->ip_min);
    if (node != NULL && node->ip_min <= insert_node->ip_min && insert_node->ip_max <= node->ip_max) {
        if (node->ip_min == insert_node->ip_min && insert_node->ip_max == node->ip_max) {
            printf("Insert Same Rule1 %d %d\n", node->port, insert_node->port);
            printf("%016lx %016lx\n", node->ip_min.high, node->ip_max.high);
            printf("%016lx %016lx\n", insert_node->ip_min.high, insert_node->ip_max.high);
            exit(1);
        }
        int res = MobaTreeNodeInsertNodeLevel(node->sub_node, insert_node);
        return res;
    }

    int part = 0;
    int have_sub_node = insert_node->sub_node == NULL ? 0 : 1;
    while(true) {
        node = FindMoreOrEqual(root, insert_node->ip_min);
        if (node != NULL && insert_node->ip_min <= node->ip_min && node->ip_max <= insert_node->ip_max) {
            if (node->ip_min == insert_node->ip_min && insert_node->ip_max == node->ip_max) {
                printf("Insert Same Rule2 %d %d\n", node->port, insert_node->port);
                exit(1);
            }
            node = MobaTreeNodeDeleteNode(root, node->ip_min);

            MobaTreeNode* node2 = FindMoreOrEqual(root, insert_node->ip_min);
            if (node2 != NULL && node2 == node) {
                printf("Can not delete %d\n", node->port);
                exit(1);
            }
            MobaTreeNodeInsertNode(insert_node->sub_node, node);
            ++part;
            if (part == 1 && have_sub_node == 1) {
                printf("Wrong exist sub_node\n");
                exit(1);
            }
        } else {
            break;
        }
    }
    MobaTreeNodeInsertNode(root, insert_node);
    return 0;
}

void MobaTreeNode::GetNodes(vector<MobaTreeNode*> &sub_nodes) {
    if (child[0] != NULL)
        child[0]->GetNodes(sub_nodes);
    sub_nodes.push_back(this);
    if (child[1] != NULL)
        child[1]->GetNodes(sub_nodes);
}

int MobaTreeNodeDeleteNodeLevel(MobaTreeNode* &root, Ip &ip_min, Ip &ip_max) {
    //printf("DeleteNodeLevel %d\n", root->port);
    MobaTreeNode *node = NULL;
    node = FindLessOrEqual(root, ip_min);
    if (node->ip_min == ip_min && node->ip_max == ip_max) {
        vector<MobaTreeNode*> sub_nodes;
        if (node->sub_node != NULL)
            node->sub_node->GetNodes(sub_nodes);
        node = MobaTreeNodeDeleteNode(root, node->ip_min);
        node->Free();
        int sub_nodes_num = sub_nodes.size();
        for (int i = 0; i < sub_nodes_num; ++i) {
            sub_nodes[i]->child[0] = NULL;
            sub_nodes[i]->child[1] = NULL;
            sub_nodes[i]->parent = NULL;
            MobaTreeNodeInsertNode(root, sub_nodes[i]);
        }
        return 0;
    }
    if (node->ip_min <= ip_min && ip_max <= node->ip_max) {
        int res = MobaTreeNodeDeleteNodeLevel(node->sub_node, ip_min, ip_max);
        return res;
    }
    printf("No such node in level\n");
    return 1;
}
