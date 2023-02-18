#ifndef  ELEMENTARY_H
#define  ELEMENTARY_H

#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

#define INSERT 1
#define DELETE 2

// #define NEXTHOP1B
// #define NEXTHOP2B
#define NEXTHOP4B

using namespace std;

#ifdef NEXTHOP1B
typedef uint8_t nexthop_t;
#endif

#ifdef NEXTHOP2B
typedef uint16_t nexthop_t;
#endif

#ifdef NEXTHOP4B
typedef uint32_t nexthop_t;
#endif

struct CommandStruct {
    int version;

    string method_name;

	string rules_file;
	string traces_file;
	string ans_file;
	string output_file;

    int rules_num;
    int traces_num;

    int traces_shuffle;
    int repeat_num;

    int lookup_round;
    int print_mode;

    int force_test;

    int hash_table_size;
};

CommandStruct ParseCommandLine(int argc, char *argv[]);

struct AccessNum {
    int sum;
    int minn;
    int maxn;
    int num;

    void Update() {
        sum += num;
        minn = min(minn, num);
        maxn = max(maxn, num);
    }

    void ClearNum() {
        num = 0;
    }

    void AddNum() {
        ++num;
    }
};

struct ProgramState {
    int rules_num;
    int traces_num;

    double memory_size;  // MB

    double build_time;  // S
    double lookup_speed;  // Mlps
    double insert_speed;  // Mups
    double delete_speed;  // Mups
    double update_speed;  // Mups

    AccessNum memory_access;

    map<uint64_t, int> cache_access;
};


struct Ip {
	uint64_t high;
	uint64_t low;

    bool operator<(const Ip& ip)const{
        if (high == ip.high)
            return low < ip.low;
        return high < ip.high;
    }

    bool operator<=(const Ip& ip)const{
        if (high == ip.high)
            return low <= ip.low;
        return high <= ip.high;
    }

    bool operator>(const Ip& ip)const{
        if (high == ip.high)
            return low > ip.low;
        return high > ip.high;
    }

    bool operator>=(const Ip& ip)const{
        if (high == ip.high)
            return low >= ip.low;
        return high >= ip.high;
    }

    bool operator==(const Ip& ip)const {
        return high == ip.high && low == ip.low;
    }

    bool operator!=(const Ip& ip)const {
        return high != ip.high || low != ip.low;
    }

    uint32_t Index(int s, int n, int version) {
        if (version == 4) {
            return (low << (32 + s)) >> (64 - n);
        } else if (version == 6) {
            if (s < 64 && s + n <= 64) {
                return (high << s) >> (64 - n); 
            } else if (s >= 64 && s + n >= 64) {
                return (low << (s - 64)) >> (64 - n); 
            } else {
                printf("wrong part index %d %d\n", s, n);
            }
        }
    }

    void RightShift(int num);
    void LeftShift(int num);
    int GetBit(int num, int version);
    void Print(int version);
};


struct Rule {
	Ip ip;
	uint8_t prefix_len;
    nexthop_t port;
    int rule_id;
};

bool RuleTraceMatch(Rule &rule, Ip &trace, int version);
bool RuleTraceMatchV6(Rule &rule, Ip &trace);


class Classifier {
public:
    virtual int Create(vector<Rule> &rules, bool insert) = 0;

    virtual int InsertRule(Ip ip, uint8_t prefix_len, nexthop_t port) = 0;
    virtual int DeleteRule(Ip ip, uint8_t prefix_len) = 0;

    virtual nexthop_t LookupV4(uint32_t ip) = 0;
    virtual nexthop_t LookupV4_MemoryAccess(uint32_t ip, ProgramState *program_state) = 0;

    virtual nexthop_t LookupV6(Ip ip) = 0;
    virtual nexthop_t LookupV6_MemoryAccess(Ip ip, ProgramState *program_state) = 0;
    virtual nexthop_t LookupV6_2(Ip ip) = 0;
    virtual nexthop_t LookupV6_2_MemoryAccess(Ip ip, ProgramState *program_state) = 0;


    virtual uint64_t MemorySize() = 0;
    virtual int Free() = 0;
    virtual int Test(void *ptr) = 0;

    int version = 0;
};

uint64_t GetRunTimeUs(timeval timeval_start, timeval timeval_end);
uint64_t GetAvgTime(vector<uint64_t> &lookup_times);
#endif