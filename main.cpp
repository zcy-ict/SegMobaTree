#include "elementary.h"
#include "io/io.h"
#include "methods/trie/trie.h"
#include "methods/segmobatree/mobatree.h"
#include "methods/segmobatree/segmobatree.h"

#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

void PerformClassifier(Classifier &classifier, int version, vector<Rule> &rules,
                       vector<Ip> &traces, vector<int> ans,
                       CommandStruct command, ProgramState *program_state,
                       nexthop_t (Classifier::*LookupV4)(uint32_t ip),
                       nexthop_t (Classifier::*LookupV4_MemoryAccess)(uint32_t ip, ProgramState *program_state),
                       nexthop_t (Classifier::*LookupV6)(Ip ip),
                       nexthop_t (Classifier::*LookupV6_MemoryAccess)(Ip ip, ProgramState *program_state)) {
    int rules_num = rules.size();
    int traces_num = traces.size();
    timeval timeval_start, timeval_end;
    vector<uint32_t> traces_v4;
    if (version == 4)
        for (int i = 0; i < traces_num; ++i)
            traces_v4.push_back(traces[i].low);
    
    // printf("build\n");
    // build
    gettimeofday(&timeval_start,NULL);

    classifier.Create(rules, true);

    gettimeofday(&timeval_end,NULL);
    program_state->build_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;
    
    // printf("lookup\n");
    // lookup
    vector<uint64_t> lookup_times;
    int lookup_round = max(1, command.lookup_round);
    for (int k = 0; k < lookup_round; ++k) {
        gettimeofday(&timeval_start,NULL);
        if (version == 4) {
            for (int i = 0; i < traces_num; ++i) {
                (classifier.*LookupV4)(traces_v4[i]);
            }
        } else if (version == 6) {
            for (int i = 0; i < traces_num; ++i) {
                (classifier.*LookupV6)(traces[i]);
            }
        }
        gettimeofday(&timeval_end,NULL);
        lookup_times.push_back(GetRunTimeUs(timeval_start, timeval_end));
    }
    uint64_t lookup_time = GetAvgTime(lookup_times);
    program_state->lookup_speed = traces_num / (lookup_time / 1.0);

    // printf("program_state\n");
    for (int i = 0; i < traces_num; ++i) {
        program_state->memory_access.ClearNum();
        if (version == 4)
            (classifier.*LookupV4_MemoryAccess)(traces_v4[i], program_state);
        else if (version == 6)
            (classifier.*LookupV6_MemoryAccess)(traces[i], program_state);
        program_state->memory_access.Update();
    }

    // printf("delete\n");
    // delete
    gettimeofday(&timeval_start,NULL);
    for (int i = 0; i < rules_num; i += 4) {
        classifier.DeleteRule(rules[i].ip, rules[i].prefix_len);
    }
    gettimeofday(&timeval_end,NULL);
    uint64_t delete_time = GetRunTimeUs(timeval_start, timeval_end);
    int delete_num = (rules_num + 3) / 4;
    program_state->delete_speed = delete_num / (delete_time / 1.0);

    // printf("insert\n");
    // insert
    gettimeofday(&timeval_start,NULL);
    if (command.force_test <= 1)
    for (int i = 0; i < rules_num; i += 4) {
        classifier.InsertRule(rules[i].ip, rules[i].prefix_len, rules[i].port);
    }
    gettimeofday(&timeval_end,NULL);
    uint64_t insert_time = GetRunTimeUs(timeval_start, timeval_end);
    int insert_num = (rules_num + 3) / 4;
    program_state->insert_speed = insert_num / (insert_time / 1.0);
    program_state->update_speed = (program_state->insert_speed + program_state->delete_speed) / 2;

    // verify
    if (command.force_test > 0) {
        for (int i = 0; i < traces_num; ++i) {
            nexthop_t port;
            if (version == 4)
                port =  (classifier.*LookupV4)(traces_v4[i]);
            else if (version == 6)
                port =  (classifier.*LookupV6)(traces[i]);
            if (ans[i] != port) {
                printf("May be wrong %d : ans %d lookup %d\n", i + 1, ans[i], port);
                exit(1);
            }
        }
    }

    // memory
    program_state->memory_size = classifier.MemorySize() / 1024.0 / 1024.0;
    
    // free
    classifier.Free();
}

void PrintProgramState(CommandStruct command, ProgramState *program_state) {

    if (command.print_mode == 0) {
        printf("\n");
        // PrintTree(program_state);
        printf("method_name: %s\n", command.method_name.c_str());
        printf("rules_file: %s\n", command.rules_file.c_str());

        printf("rules_num: %d ", program_state->rules_num);
        printf("traces_num: %d\n\n", program_state->traces_num);
        printf("memory_size: %.3f MB\n", program_state->memory_size);

        // printf("build_time: %.2f S\n", program_state->build_time);
        printf("lookup_speed: %.2f MLPS\n", program_state->lookup_speed);
        // printf("insert_speed: %.2f MUPS\n", program_state->insert_speed);
        // printf("delete_speed: %.2f MUPS\n", program_state->delete_speed);
        printf("update_speed: %.2f MUPS\n\n", program_state->update_speed);

        // printf("cache_num: %ld cache_size %.2f MB\n", program_state->cache_access.size(),
        //        1.0 * program_state->cache_access.size() * 64 / 1024 / 1024);

        printf("memory_access_sum: %d\tmemory_access_avg: %.2f\tmemory_access_max: %d\n",
               program_state->memory_access.sum,
               1.0 * program_state->memory_access.sum / program_state->traces_num,
               program_state->memory_access.maxn);

        printf("\n\n");
    } else if (command.print_mode == 1) {
        //printf("%s\t", command.method_name.c_str());
        printf("%d\t", program_state->rules_num);
        printf("%d\t", program_state->traces_num);
        printf("%.3f\t", program_state->memory_size);

        // printf("%.4f\t", program_state->build_time);
        printf("%.4f\t", program_state->lookup_speed);
        // printf("%.2f\t", program_state->insert_speed);
        // printf("%.2f\t", program_state->delete_speed);
        printf("%.2f\t", program_state->update_speed);

        printf("%ld\t%.2f\t", program_state->cache_access.size(),
               1.0 * program_state->cache_access.size() * 64 / 1024 / 1024);
        printf("%d\t%.2f\t%d\t",
               program_state->memory_access.sum,
               1.0 * program_state->memory_access.sum / program_state->traces_num,
               program_state->memory_access.maxn);

        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    CommandStruct command = ParseCommandLine(argc, argv);
    int version = command.version;

    vector<Rule> rules = ReadRules(command.rules_file, version);

    vector<Ip> traces;
    if (command.traces_file == "" || command.traces_file == "NULL")
        traces = GenerateTraces(rules, command.traces_shuffle, command.repeat_num);
    else 
        traces = ReadTraces(command.traces_file, version);
    vector<int> ans = GenerateAns(rules, traces, version, command.force_test);

    ProgramState *program_state = new ProgramState();
    program_state->rules_num = rules.size();
    program_state->traces_num = traces.size();

    if (command.method_name == "Trie") {
        Trie trie;
        trie.version = version;
        PerformClassifier(trie, version, rules, traces, ans, command, program_state,
                          &Classifier::LookupV4, &Classifier::LookupV4_MemoryAccess,
                          &Classifier::LookupV6, &Classifier::LookupV6_MemoryAccess);
    } else if (command.method_name == "MobaTree" && version == 6) {
        MobaTree mobatree;
        mobatree.version = version;
        PerformClassifier(mobatree, version, rules, traces, ans, command, program_state,
                          &Classifier::LookupV4, &Classifier::LookupV4_MemoryAccess,
                          &Classifier::LookupV6, &Classifier::LookupV6_MemoryAccess);
    } else if (command.method_name == "SegMobaTree" && version == 6) {
        SegMobaTree segmobatree;
        segmobatree.version = version;
        PerformClassifier(segmobatree, version, rules, traces, ans, command, program_state,
                          &Classifier::LookupV4, &Classifier::LookupV4_MemoryAccess,
                          &Classifier::LookupV6, &Classifier::LookupV6_MemoryAccess);
    }

    PrintProgramState(command, program_state);

    return 0;
}