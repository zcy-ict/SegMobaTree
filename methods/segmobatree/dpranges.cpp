#include "dpranges.h"

using namespace std;

double dp_time;

bool CmpDpRulePrefixLen(DpRule rule1, DpRule rule2) {
	return rule1.prefix_len < rule2.prefix_len;
}

void DpInfo::GetDpRules(vector<Rule> &_rules) {
	rules_num = _rules.size();
	for (int i = 0; i < rules_num; ++i) {
		DpRule rule;
		rule.prefix_len = _rules[i].prefix_len;
		int k = 128 - rule.prefix_len;
		rule.ip = _rules[i].ip;
		rule.ip.RightShift(k);
		rule.ip.LeftShift(k);
		rules.push_back(rule);
	}
	sort(rules.begin(), rules.end(), CmpDpRulePrefixLen);
	rules_num = rules.size();
}

void DpInfo::Init() {
	for (int i = 0; i < rules_num; ++i) {
		++prefix_len_num[rules[i].prefix_len];
	}
	prefix_len_sum[0] = prefix_len_num[0];
	for (int i = 1; i <= 128; ++i)
		prefix_len_sum[i] = prefix_len_sum[i - 1] + prefix_len_num[i];
}

uint32_t DpInfo::GetPrefixSum(int start, int end) {
	if (start > end)
		return 0;
	if (start == 0)
		return prefix_len_sum[end];
	return prefix_len_sum[end] - prefix_len_sum[start - 1];
}

uint64_t DpInfo::CalculateMapCost() {
	map<Ip, int>::iterator iter;
	uint64_t cost = 0;
	int index = 0;
	for (iter = prefix_map.begin(); iter != prefix_map.end(); ++iter) {
		uint64_t num = iter->second;
		cost += (log(num) / log(2) + 1) * num;
	}
	return cost;
}

void DpInfo::CalculateRangeX(int x) {
	if (prefix_len_num[x] == 0) {
		for (int i = x; i <= 128; ++i)
			if (GetPrefixSum(x, i) == 0)
				tuple_cost[x][i] = 0;
			else
				tuple_cost[x][i] = 1ULL << 60;
		return;
	}
	for (int i = 0; i < rules_num; ++i)
		if (rules[i].prefix_len >= x) {
			int k = 128 - x;
			rules[i].reduced_ip = rules[i].ip;
			rules[i].reduced_ip.RightShift(k);
		}
	sort(rules.begin(), rules.end(), CmpDpRulePrefixLen);

	prefix_map.clear();
	int pre_end = x - 1;
	for (int i = 0; i < rules_num; ++i) {
		if (rules[i].prefix_len < x)
			continue;
		++prefix_map[rules[i].reduced_ip];
		if (i == rules_num - 1 || rules[i].prefix_len != rules[i + 1].prefix_len) {
			uint64_t cost = CalculateMapCost();
			int end;
			if (i == rules_num - 1)
				end = 128;
			else
				end = rules[i + 1].prefix_len - 1;
			for (int j = pre_end + 1; j <= end; ++j) {
				tree_cost[x][j] = cost;
				tree_prefix[x][j] = prefix_map.size();
				int num = GetPrefixSum(0,  j);
				rule_cost[x][j] = num;
				tuple_cost[x][j] = tree_cost[x][j] + rule_cost[x][j];
			}
			pre_end = end;
		}
	}
}

void DpInfo::DynamicProgramming() {
	for (int w = 0; w <= 128; ++w)
		for (int i = 0; i + w <= 128; ++i) {
			int j = i + w;
			tuple_sum[i][j] = tuple_cost[i][j];
			for (int k = j - 1 ; k >= i; --k)
				if (tuple_sum[i][j] > tuple_sum[i][k] + tuple_sum[k + 1][j])
					tuple_sum[i][j] = tuple_sum[i][k] + tuple_sum[k + 1][j];
		}
}

void DpInfo::GetDpRange(int i, int j) {
	// printf("GetDpRange %d %d, %lu %lu\n", i, j, tuple_sum[i][j], tuple_cost[i][j]);
	if (tuple_sum[i][j] == tuple_cost[i][j] || GetPrefixSum(i, j) == 0) {

		RuleRange range;
		range.start = i;
		range.end = j;
		range.prefix_num = tree_prefix[i][j];
		range.rules_num = GetPrefixSum(i, j);
		ranges.push_back(range);
		return;
	}
	for (int k = j - 1 ; k >= i; --k)
		if (tuple_sum[i][j] == tuple_sum[i][k] + tuple_sum[k + 1][j] &&
			GetPrefixSum(k + 1, j) > 0) {
			GetDpRange(k + 1, j);
			GetDpRange(i, k);
			break;
		}
}

uint64_t DpInfo::CalculateRange() {
	for (int i = 128; i >= 0; --i)
		CalculateRangeX(i);
	DynamicProgramming();
	GetDpRange(0, 128);
	return 0;
}
void DpInfo::Test(int i, int j) {
	printf("%d %d  rule_num %d tuple_cost = %lu, tree_cost %lu rule_cost %lu\n", 
			i, j, GetPrefixSum(i, j), tuple_cost[i][j], tree_cost[i][j], rule_cost[i][j]);
}

vector<RuleRange> GetIpv6RuleRanges(vector<Rule> &rules) {
    timeval timeval_start, timeval_end;
    gettimeofday(&timeval_start,NULL);

	DpInfo *dp_info = new DpInfo();
	dp_info->GetDpRules(rules);
	dp_info->Init();
	dp_info->CalculateRange();

    gettimeofday(&timeval_end,NULL);
    dp_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;

	return dp_info->ranges;
}

vector<RuleRange> GetHaBiRuleRanges(vector<Rule> &rules) {
	// printf("GetIpv6RuleRanges\n");
	int k = rules[rules.size() - 1].prefix_len - 16;
	DpInfo *dp_info = new DpInfo();
	dp_info->GetDpRules(rules);
	dp_info->Init();

	RuleRange range;
	int i, j;

	i = 48 + k, j = 128;
	dp_info->CalculateRangeX(i);
	range.start = i;
	range.end = j;
	range.prefix_num = dp_info->tree_prefix[i][j];
	range.rules_num = dp_info->GetPrefixSum(i, j);
	dp_info->ranges.push_back(range);

	i = 32 + k, j = 47 + k;
	dp_info->CalculateRangeX(i);
	range.start = i;
	range.end = j;
	range.prefix_num = dp_info->tree_prefix[i][j];
	range.rules_num = dp_info->GetPrefixSum(i, j);
	dp_info->ranges.push_back(range);

	i = 16 + k, j = 31 + k;
	dp_info->CalculateRangeX(i);
	range.start = i;
	range.end = j;
	range.prefix_num = dp_info->tree_prefix[i][j];
	range.rules_num =dp_info->GetPrefixSum(i, j);
	dp_info->ranges.push_back(range);

	i = k, j = 15 + k;
	dp_info->CalculateRangeX(i);
	range.start = i;
	range.end = j;
	range.prefix_num = dp_info->tree_prefix[i][j];
	range.rules_num = dp_info->GetPrefixSum(i, j);
	dp_info->ranges.push_back(range);

	if (k > 0) {
		i = 0, j = k - 1;
		dp_info->CalculateRangeX(i);
		range.start = i;
		range.end = j;
		range.prefix_num = dp_info->tree_prefix[i][j];
		range.rules_num = dp_info->GetPrefixSum(i, j);
		dp_info->ranges.push_back(range);
	}

	return dp_info->ranges;
}