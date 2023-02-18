#ifndef IO_H
#define IO_H

#include "../elementary.h"

#include <cstdio>
#include <cstring>

using namespace std;

vector<Rule> ReadRules(string rules_file, int version);
vector<Rule> UniqueRules(vector<Rule> &rules);
void PrintRules(vector<Rule> &rules, string ouput_file, int version);
vector<Ip> ReadTraces(string traces_file, int version);
vector<Ip> GenerateTraces(vector<Rule> &rules, int traces_shuffle, int repeat_num);
vector<int> GenerateAns(vector<Rule> &rules, vector<Ip> &traces, int version, int flag);
void AnalyseRules(vector<Rule> &rules);
#endif