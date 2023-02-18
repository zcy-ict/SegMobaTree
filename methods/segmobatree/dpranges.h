#ifndef  DPRANGES_H
#define  DPRANGES_H

#include "../../elementary.h"

#include <cmath>

#define DPRANGES 0
#define HABIRANGES 1

using namespace std;

struct RuleRange {
	int start;
	int end;
	int prefix_num;
	int rules_num;
};

struct DpRule {
    Ip ip;
    Ip reduced_ip;
    int prefix_len;
};

struct DpInfo {
	vector<DpRule> rules;
	int rules_num;

	int prefix_len_num[130];
	int prefix_len_sum[130];

	uint64_t tree_cost[130][130];
	uint64_t rule_cost[130][130];
	int  tree_prefix[130][130];
	uint64_t tuple_cost[130][130];
	uint64_t tuple_sum[130][130];

	map<Ip, int> prefix_map;

	vector<RuleRange> ranges;


	void GetDpRules(vector<Rule> &rules);
	void Init();
	uint32_t GetPrefixSum(int start, int end);
	uint64_t CalculateMapCost();
	void CalculateRangeX(int x);
	void DynamicProgramming();
	void GetDpRange(int i, int j);
	void Test(int i, int j);
	uint64_t CalculateRange();
};


vector<RuleRange> GetIpv6RuleRanges(vector<Rule> &rules);
vector<RuleRange> GetHaBiRuleRanges(vector<Rule> &rules);

#endif