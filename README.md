# SegMobaTree: The Segmented Multilayer Online Balanced Tree for high-performance IPv6 Lookup in the Edge Network

## Introduction
The code is for the paper

If there are any problems or bugs, welcome to discuss with me

zhangchunyang@ict.ac.cn

zhangchunyangict@outlook.com

## Experimental environment

Ubuntu 16.04

g++ 5.4.0

## Parameters
**--version :**                 "4" for ipv4 lookup, "6" for ipv6 lookup

**--method_name :**             the method of alogrithm

**--rules_file :**              the file of rules

**--traces_file :**             the file of traces

**--traces_shuffle :**          "1" means shuffle the traces and "0" means not

**--lookup_round :**            the lookup process repeat "n" rounds, then get the average lookup time

**--repeat_num :**              each trace repeat "n" times to lookup

**--force_test :**              to verify the result of lookup, "0" means not verify, "1" means verify, "2" means verify after delete 25% rules

**--print_mode :**              the print mode, "0" means print with instructions, "1" means print without instructions


## Algorithms
**Trie :**                      (My Reproduction)    reproduction of Trie

**MobaTree :**                  (My Work)            the multilayer online balanced tree

**SegMobaTree :**               (My Work)            the segmented multilayer online balanced tree


## Sample
sh run.sh

make

./main --version 6 --method_name MobaTree    --rules_file data/ipv6_rrc00 --traces_file NULL --traces_shuffle 1 --lookup_round 1 --repeat_num 100 --force_test 1 --print_mode 0

./main --version 6 --method_name SegMobaTree --rules_file data/ipv6_rrc00 --traces_file NULL --traces_shuffle 1 --lookup_round 1 --repeat_num 100 --force_test 1 --print_mode 0
