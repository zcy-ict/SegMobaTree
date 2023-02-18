#ifndef MOBATREENODEH
#define MOBATREENODEH

#define RED 0
#define BLACK 1

#include "../../elementary.h"

using namespace std;

struct MobaTreeNode {
    MobaTreeNode *child[2];
    MobaTreeNode *sub_node;
    bool color; // 0 red, 1 black
    nexthop_t port;

    Ip ip_min;
    Ip ip_max;
    MobaTreeNode *parent;

    void Rotate(MobaTreeNode* &root, int d); // 0左旋，1右旋
    void HandleReorient(MobaTreeNode* &root, Ip &ip);
    void SwapKey(MobaTreeNode *node);
    void Transplant(MobaTreeNode* &root, MobaTreeNode* node);
    MobaTreeNode* DeleteNode(MobaTreeNode* &root, Ip &ip);
    void Free();
    void GetNodes(vector<MobaTreeNode*> &sub_nodes);
};

bool Color(MobaTreeNode* node);
void ChangeColor(MobaTreeNode* node, bool color);
MobaTreeNode* FindMin(MobaTreeNode* node);
MobaTreeNode* FindMax(MobaTreeNode* node);
MobaTreeNode* FindLessOrEqual(MobaTreeNode* node, Ip &ip);
MobaTreeNode* FindMoreOrEqual(MobaTreeNode* node, Ip &ip);

MobaTreeNode* MobaTreeNodeCreate(Ip &ip_min, Ip &ip_max, nexthop_t port, MobaTreeNode *parent);
int MobaTreeNodeInsertNodeLevel(MobaTreeNode* &root, MobaTreeNode *insert_node);
int MobaTreeNodeDeleteNodeLevel(MobaTreeNode* &root, Ip &ip_min, Ip &ip_max);

void TrbNodeTest();

#endif