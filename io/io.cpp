#include "io.h"
#include "../methods/trie/trie.h"
#include <map>

using namespace std;


vector<string> StrSplit(const string& str, const string& pattern) {
    vector<string> ret;
    if(pattern.empty())
        return ret;
    int start = 0, index = str.find_first_of(pattern, 0);
    while(index != str.npos) {
        if(start != index)
            ret.push_back(str.substr(start, index-start));
        start = index + 1;
        index = str.find_first_of(pattern, start);
    }
    if(!str.substr(start).empty())
        ret.push_back(str.substr(start));
    return ret;
}

Ip GetIpv4Comma(string ip_str) {
    Ip ip;
    ip.high = 0;
    ip.low = 0;
    vector<string> vc = StrSplit(ip_str, ".");
    for (int i = 0; i < 4; ++i)
        ip.low = ip.low << 8 | atoi(vc[i].c_str());
    return ip;
}

Ip GetIpv4Hex(string ip_str) {
    Ip ip;
	ip.high = 0;
	sscanf(ip_str.c_str(), "%lx", &ip.low);
    return ip;
}

Ip GetIpv6Colon(string ip_str) {
	int part[8];
	memset(part, 0, sizeof(part));
	int colon = 0, index = 0;
	int len = ip_str.length();
	for (int i = 0; i < len; ++i)
		if (ip_str[i] == ':')
			++colon;
	for (int i = 0; i < len; ++i) {
		if (ip_str[i] == ':') {
			++index;
			if (i > 0 && ip_str[i - 1] == ':')
				index += 7 - colon;
		} else {
			int k = 0;
			if ('0' <= ip_str[i] && ip_str[i] <= '9')
				k = ip_str[i] - '0';
			else if ('a' <= ip_str[i] && ip_str[i] <= 'f')
				k = ip_str[i] - 'a' + 10;
			else if ('A' <= ip_str[i] && ip_str[i] <= 'F')
				k = ip_str[i] - 'A' + 10;
			part[index] = part[index] * 16 + k;
		}
	}
	Ip ip;
	ip.high = ip.low = 0;
	for (int i = 0; i < 4; ++i)
		ip.high = (ip.high << 16) | part[i];
	for (int i = 4; i < 8; ++i)
		ip.low = (ip.low << 16) | part[i];
	return ip;
}

Ip GetIpv6Hex(string ip_str) {
    Ip ip;
	sscanf(ip_str.substr(0, 16).c_str(), "%lx", &ip.high);
	sscanf(ip_str.substr(16, 32).c_str(), "%lx", &ip.low);
    return ip;
}
Ip GetIpv6Comma(string ip_str) {
    Ip ip;
    sscanf(ip_str.c_str(), "%lx.%lx", &ip.high, &ip.low);
    return ip;
}

Ip GetIpv6(string ip_str) {
    int len = ip_str.length();
    bool colon = false;
    bool comma = false;
    for (int i = 0; i < len; ++i)
        if (ip_str[i] == ':')
            colon = true;
        else if (ip_str[i] == '.')
            comma = true;
    if (colon)
        return GetIpv6Colon(ip_str);
    if (comma)
        return GetIpv6Comma(ip_str);
    else
        return GetIpv6Hex(ip_str);

}

bool CmpRulePrefixlen(Rule rule1, Rule rule2) {
    return rule1.prefix_len < rule2.prefix_len;
}

void TestRule(vector<Rule> &rules) {
    printf("TestRule\n");
    map<uint64_t, uint64_t> match[4];
    int rules_num = rules.size();
    int num[4];
    int hit_null_num[4];
    memset(num, 0, sizeof(num));
    memset(hit_null_num, 0, sizeof(hit_null_num));
    vector<Rule> rules_vc[4];
    for (int i = 0; i < rules_num; ++i) {
        int k = min(3, (int)rules[i].prefix_len / 16);
        ++num[k];
        rules_vc[k].push_back(rules[i]);
        ++match[k][rules[i].ip.high >> (64 - k * 16)];
        if (k == 2) {
            uint64_t p = rules[i].ip.high >> (16);
            if (match[3].find(p) == match[3].end())
                ++hit_null_num[3];
        }
    }

    printf("%d %d %d %d\n", num[0], num[1], num[2], num[3]);
    printf("%ld %ld %ld %ld\n", match[0].size(), match[1].size(), match[2].size(), match[3].size());
    printf("%d\n", hit_null_num[3]);
    exit(1);
}

vector<Rule> ReadRules(string rules_file, int version) {
    vector<Rule> rules;
	int rules_num = 0;
    char buf[1025];

	FILE *fp = fopen(rules_file.c_str(), "rb");
	if (!fp)
		printf("Cannot open the file %s\n", rules_file.c_str());
    
    vector<string> vc;
	while (fgets(buf,1000,fp)!=NULL) {
        string str = buf;
        vc = StrSplit(str, "/");
        //printf("%s %s\n", vc[0].c_str(), vc[1].c_str());

        struct Rule rule;
        if (version == 4)
            rule.ip = GetIpv4Comma(vc[0]);
        else if (version == 6)
            rule.ip = GetIpv6(vc[0]);
        rule.prefix_len = atoi(vc[1].c_str());
        rule.rule_id = ++rules_num;
#ifdef NEXTHOP1B
        rule.port = (rule.rule_id % 254) + 1;
#endif
#ifdef NEXTHOP2B
        rule.port = (rule.rule_id % 65534) + 1;
#endif
#ifdef NEXTHOP4B
        rule.port = rule.rule_id;
#endif
        if (rule.prefix_len == 0)
            continue;
        // if (rule.prefix_len > 64)
        //     continue;
        // if (rule.prefix_len <= 31)
        //     continue;

        // if (rule.prefix_len != 64)
        //     continue;
        // rule.prefix_len += 16;
        // rule.ip.high >>= 16;
        // rule.prefix_len = 26;
        // rule.ip.high >>= 38;
        // if (rule.prefix_len < 16)
        //     continue;

        // if (rule.prefix_len <= 31)
        // if (4001 <= rules_num  && rules_num <= 5000)
        rules.push_back(rule);

        // rule.ip.Print(version); 
        // if (rules.size() >= 50000) break;
	}
	fclose(fp);
    // random_shuffle(rules.begin(),rules.end());
    //int maxn_num = 12000; if (rules_num > maxn_num) rules.resize(maxn_num);
    //sort(rules.begin(), rules.end(), CmpRulePrefixlen);
    /*reverse(rules.begin(),rules.end());
    for (int i = rules.size() - 1; i > 0; --i)
        if (rules[i - 1].prefix_len > rules[i].prefix_len) {
            printf("rules[%d].prefix_len > rules[%d].prefix_len\n", i - 1, i);
            printf("%d %d\n", rules[i - 1].port, rules[i].port);
            exit(1);
        }*/
	// printf("rules_num = %ld\n", rules.size());
    //TestRule(rules);
	return rules;
}

vector<Rule> UniqueRules(vector<Rule> &rules) {
    map<Ip, int> match[129];
    int rules_num = rules.size();
    vector<Rule> rules2;
    for (int i = 0; i < rules_num; ++i) {
        int prefix_len = rules[i].prefix_len;
        Ip ip = rules[i].ip;
        if (match[prefix_len].find(ip) == match[prefix_len].end()) {
            match[prefix_len][ip] = 1;
            rules2.push_back(rules[i]);
        }
    }
    // printf("UniqueRules %ld\n", rules2.size());
    // exit(1);
    return rules2;

}

void PrintRules(vector<Rule> &rules, string ouput_file, int version) {
    FILE *fp = fopen(ouput_file.c_str(), "wb");
	if (!fp)
		printf("Cannot open the file %s\n", ouput_file.c_str());
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i) {
        if (version == 4)
            fprintf(fp, "%ld.%ld.%ld.%ld/%d\n", rules[i].ip.low >> 24 & 255, rules[i].ip.low >> 16 & 255,
                 rules[i].ip.low >> 8 & 255, rules[i].ip.low & 255, rules[i].prefix_len);
        else if (version == 6)
            fprintf(fp, "%016lx%016lx/%d\n", rules[i].ip.high, rules[i].ip.low, rules[i].prefix_len);
    }

}
//Ip IpRand(Ip ip, int prefix_len) {
//    Ip ret = ip;
//    ret.high |= rand() & (~ip_mask[prefix_len][0]);
//    ret.low |= rand() & (~ip_mask[prefix_len][1]);
//    return ret;
//}

vector<Ip> ReadTraces(string traces_file, int version) {

    vector<Ip> traces;
    FILE *fp = fopen(traces_file.c_str(), "rb");
    if (!fp)
        printf("Cannot open the file %s\n", traces_file.c_str());

    Ip ip;
    char buf[1025];
    while (fgets(buf,1000,fp)!=NULL) {
        string str = buf;
        if (version == 4)
            ip = GetIpv4Comma(str);
        else if (version == 6)
            ip = GetIpv6(str);
        traces.push_back(ip);
    }

    // printf("traces_num = %ld\n", traces.size());
    return traces;
}

vector<Ip> GenerateTraces(vector<Rule> &rules, int traces_shuffle, int repeat_num) {
    vector<Ip> traces;
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i) {
        traces.push_back(rules[i].ip);
        //for (int j = 0; j < 100; ++j)
        //    traces.push_back(IpRand(rules[i].ip, rules[i].prefix_len));

    }
    if (traces_shuffle)
        random_shuffle(traces.begin(),traces.end());
    if (repeat_num >= 1) {
        vector<Ip> traces2 = traces;
        traces.clear();
        for (int i = 0; i < rules_num; ++i) {
            for (int j = 0; j < repeat_num; ++j)
                traces.push_back(traces2[i]);
            // if (i > rules_num / 10) break;
        }
        traces2.clear();
    }

	//printf("traces_num = %ld\n", traces.size());
    return traces;
}

vector<int> GenerateAns(vector<Rule> &rules, vector<Ip> &traces, int version, int force_test) {
    vector<int> ans;
    if (force_test == 0)
        return ans;
    int rules_num = rules.size();
    int traces_num = traces.size();
    int port;

    bool use_trie = true;
    if (use_trie) {
        Trie trie;
        trie.version = version;
        trie.Create(rules, false);
        for (int i = 0; i < rules_num; ++i) 
            if (!(force_test == 2 && i % 4 == 0))
                trie.InsertRule(rules[i].ip, rules[i].prefix_len, rules[i].port);
        for (int i = 0; i < traces_num; ++i) {
            if (i > 0 && traces[i] == traces[i - 1])
                port = ans[i - 1];
            else if (version == 4)
                port = trie.LookupV4(traces[i].low);
            else if (version == 6)
                port = trie.LookupV6(traces[i]);
            ans.push_back(port);
        }
        trie.Free();
    } else {
        for (int i = 0; i < traces_num; ++i) {
            int index = -1;
            for (int j = 0; j < rules_num; ++j) {
                if (force_test == 2 && j % 4 == 0) continue;
                if (RuleTraceMatch(rules[j], traces[i], version)) {
                    if (index == -1)
                        index = j;
                    else if (rules[j].prefix_len > rules[index].prefix_len)
                        index = j;
                    else if (rules[j].prefix_len == rules[index].prefix_len) {
                        printf("Same rule %d %d\n", index, j);
                        printf("rules %d traces %d\n", j, i);
                        exit(1);
                    }
                }
            }
            if (index == -1)
                ans.push_back(0);
            else
                ans.push_back(rules[index].port);
        }
    }

    // printf("ans_num = %ld\n", ans.size());
    return ans;
}

uint32_t prefix_rules_num[130];
uint32_t prefix_rules_sum[130];
// uint32_t prefix_range[]= {0, 16, 24, 32, 40, 48, 64, 96, 128};
// uint32_t prefix_range_len = 9;

uint32_t prefix_range[]= {0, 24, 31, 32, 39, 40, 47, 48, 128};
uint32_t prefix_range_len = 9;

// uint32_t prefix_range[]= {0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128};
// uint32_t prefix_range_len = 17;

void AnalyseRules(vector<Rule> &rules) {
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i)
        ++prefix_rules_num[rules[i].prefix_len];
    for (int i = 1; i <= 128; ++i)
        prefix_rules_sum[i] = prefix_rules_sum[i - 1] + prefix_rules_num[i];
    // for (int i = 1; i <= 128; ++i)
    //     printf("%d : %d %d\n", i, prefix_rules_num[i], prefix_rules_sum[i]);
    for (int i = 1; i < prefix_range_len; ++i)
        printf("range %d - %d: %d\n", prefix_range[i - 1] + 1, prefix_range[i], 
            prefix_rules_sum[prefix_range[i]] - prefix_rules_sum[prefix_range[i - 1]]);
    for (int i = 1; i < prefix_range_len; ++i)
        printf("%.2f\n", 100.0 * (prefix_rules_sum[prefix_range[i]] - prefix_rules_sum[prefix_range[i - 1]]) / rules_num);
}