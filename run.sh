force_test=1
traces_shuffle=1
repeat_num=100
lookup_round=1
print_mode=0

output="output"

loop_times=1

RunProgram() {
    method_name="$1"
    version="$2"
    rules_file="$3"
    traces_file="$4"

    i=0
    while [ $i -lt $loop_times ]
    do
        if [ $loop_times -gt 1 ]; then
            echo "  \c" >> ${output}
        else
            echo "${method_name}\t${rule_file}\t\c" >> ${output}
        fi

        ./main --version ${version} --method_name ${method_name} \
               --rules_file ${rules_file} --traces_file ${traces_file} \
               --traces_shuffle ${traces_shuffle} \
               --lookup_round ${lookup_round} --repeat_num ${repeat_num} \
               --force_test ${force_test} --print_mode ${print_mode} \
               # >> ${output}

        i=`expr $i + 1`
    done
        
    if [ $loop_times -gt 1 ]; then
        echo "${method_name}\t${rule_file}" >> ${output}
    fi
}

rm -rf ${output}
date > ${output}

make

for rule in 00 00\_64 #00\_96 00\_128
do
    version=6
    rules_file=data/ipv${version}_rrc${rule}
    traces_file="NULL"
    echo ${rule_file} >> ${output}

    # RunProgram Trie ${version} ${rules_file} ${traces_file}
    RunProgram MobaTree ${version} ${rules_file} ${traces_file}
    RunProgram SegMobaTree ${version} ${rules_file} ${traces_file}

done

date >> ${output}