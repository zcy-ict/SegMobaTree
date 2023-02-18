#include "elementary.h"


using namespace std;

CommandStruct ParseCommandLine(int argc, char *argv[]) {
	CommandStruct command;
    char short_opts[] = "";
	struct option long_opts[] = {
       {"version", required_argument, NULL, 0},
       {"method_name", required_argument, NULL, 0},
       {"rules_file", required_argument, NULL, 0},
       {"traces_file", required_argument, NULL, 0},
       {"ans_file", required_argument, NULL, 0},
       {"output_file", required_argument, NULL, 0},
       {"traces_shuffle", required_argument, NULL, 0},
       {"lookup_round", required_argument, NULL, 0},
       {"repeat_num", required_argument, NULL, 0},
       {"force_test", required_argument, NULL, 0},
       {"print_mode", required_argument, NULL, 0},
       {0, 0, 0, 0}
   };
    int opt, option_index;
	bool flag = true;
    
    while((opt = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
    	if (opt == 0) {
	        if (strcmp(long_opts[option_index].name, "version") == 0) {
	            command.version = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "method_name") == 0) {
	            command.method_name = optarg;
			} else if (strcmp(long_opts[option_index].name, "rules_file") == 0) {
	            command.rules_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "traces_file") == 0) {
	            command.traces_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "ans_file") == 0) {
	            command.ans_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "output_file") == 0) {
	            command.output_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "traces_shuffle") == 0) {
	            command.traces_shuffle = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "lookup_round") == 0) {
	            command.lookup_round = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "repeat_num") == 0) {
	            command.repeat_num = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "force_test") == 0) {
            	command.force_test = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "print_mode") == 0) {
	            command.print_mode = strtoul(optarg, NULL, 0);
			}
		}
	}
    if (command.version == 0) {
        printf("Need version 4 or 6\n");
        flag = false;
    }
	if (!flag)
		exit(1);
	return command;
}

// 0 < num < 128
void Ip::RightShift(int num) {
	if (num == 0) {
		return;
	} else if (num < 64) {
		low = (high << (64 - num)) | (low >> num);
		high = high >> num;
	} else if (num == 64) {
		low = high;
		high = 0;
	} else {
		low = high >> (num - 64);
		high = 0;
	}
}

void Ip::LeftShift(int num) {
	if (num == 0) {
		return;
	} else if (num < 64) {
		high = (high << num) | (low >> (64 - num));
		low = low << num;
	} else if (num == 64) {
		high = low;
		low = 0;
	} else {
		high = low << (num - 64);
		low = 0;
	}
}

int Ip::GetBit(int num, int version) {
	if (version == 4) {
		return (low & (1ULL << (31 - num))) > 0 ? 1 : 0; 
	} else {
		if (num <= 63)
			return (high & (1ULL << (63 - num))) > 0 ? 1 : 0; 
		else
			return (low & (1ULL << (127 - num))) > 0 ? 1 : 0; 
	}
}

void Ip::Print(int version) {
	if (version == 4)
		printf("%ld.%ld.%ld.%ld\n", low >> 24 & 255, low >> 16 & 255, low >> 8 & 255, low & 255);
	else if (version == 6)
		printf("%016lx %016lx\n", high, low);
}

bool RuleTraceMatch(Rule &rule, Ip &trace, int version) {
	int shift_num = (version == 4 ? 32 : 128) - rule.prefix_len;

	Ip rule_ip = rule.ip;
	rule_ip.RightShift(shift_num);
	Ip trace_ip = trace;
	trace_ip.RightShift(shift_num);
	
	if (rule_ip.high == trace_ip.high && rule_ip.low == trace_ip.low)
		return true;
	return false;
}

bool RuleTraceMatchV6(Rule &rule, Ip &trace) {
	int shift_num = 128 - rule.prefix_len;

	Ip rule_ip = rule.ip;
	rule_ip.RightShift(shift_num);
	Ip trace_ip = trace;
	trace_ip.RightShift(shift_num);
	
	if (rule_ip.high == trace_ip.high && rule_ip.low == trace_ip.low)
		return true;
	return false;
}

uint64_t GetRunTimeUs(timeval timeval_start, timeval timeval_end) {  // us
	return 1000000 * (timeval_end.tv_sec - timeval_start.tv_sec)+ timeval_end.tv_usec - timeval_start.tv_usec;
}

uint64_t GetAvgTime(vector<uint64_t> &lookup_times) {
	int num = lookup_times.size();
	if (num == 0)
		return 0;
	sort(lookup_times.begin(), lookup_times.end());
	int l = num / 4;
	int r = num - l;
	//printf("GetAvgTime num %d l %d r %d\n", num, l, r);
	uint64_t sum = 0;
	for (int i = l; i < r; ++i)
		sum += lookup_times[i];
	return sum / (r - l);
}
