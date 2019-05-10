#!/bin/bash

: ${MAX_PROCS:=4}
: ${BATCH_CONF:=batch.conf}

if [ "$#" -ge 1 ]; then
    BATCH_CONF=$1
fi

sleep 1
echo -e "\nRunning batch simulations with $MAX_PROCS workers."
echo "Reading batch configuration from '$BATCH_CONF'."

if [ ! -f "$BATCH_CONF" ]; then
    echo "Error: unable to find configuration file."
    exit
fi

mkdir -p job_logs
mkdir -p jobs_completed

function job {
    # $1 -> file name (config YAML)
    conf_file=$(echo $1 | awk '{print $1}')
    load_file=$(echo $1 | awk '{print $2}')
    echo -n "Starting job $conf_file == "
    if [ -f "$load_file" ]; then
        echo "Load $load_file."
    else
        echo "New."
    fi
    data_path=../data/  # Where to save results.
    bin=../bin/prot-3   # The binary to execute.
    dirdate=$(date +%Y_%m_%d_%H%M%S)
    dircount=0
    simulation_name=$(basename $conf_file | sed 's/\.[^.]*$//')
    resdir="$data_path$dirdate"
    resdir+="_$simulation_name"
    resdir+="_$dircount"
    log_file="job_logs/$simulation_name.log"
    count=0
    while [ ${count} -lt 3 ]; do
        # Loop until the folder name doesn't exist
        while [ -d $resdir ]; do
            dircount+=1
            resdir="$data_path$dirdate"
            resdir+="_$simulation_name"
            resdir+="_$dircount"
        done
        if [ -f "$load_file" ]; then
            # echo "-f ../batch/$conf_file -l $load_file -d $resdir/"
            # sleep 1
            # $bin --simple-log -g0 -f ../batch/$conf_file -l $load_file -d $resdir/ > $log_file 2>&1
            $bin -g0 -f ../batch/$conf_file -l $load_file -d $resdir/ > $log_file 2>&1
            exit_value=$?
        else
            # echo "-f ../batch/$conf_file -d $resdir/"
            # sleep 1
            # $bin --simple-log -g0 -f ../batch/$conf_file -d $resdir/ > $log_file 2>&1
            $bin -g0 -f ../batch/$conf_file -d $resdir/ > $log_file 2>&1
            exit_value=$?
        fi
        if [[ $exit_value != 0 ]]; then
            completed_data=$(date +"%F %T")
            printf "$completed_data $simulation_name -- Failed ($count): exit value = $exit_value\n"
            printf "$completed_data %20s -- [ FAIL ] : $resdir [C:$count, E:$exit_value]\n" $simulation_name | tee -a batch.log
            count=$(($count + 1))
        else
            break
        fi
    done
    completed_data=$(date +"%F %T")
    if [ ${count} -ge 3 ]; then
        # Failed!
        printf "$completed_data %20s -- [ FAIL ] : $resdir [aborted]\n" $simulation_name | tee -a batch.log
    else
        # Succeeded:
        printf "$completed_data %20s -- [ OK-$count ] : $resdir\n" $simulation_name | tee -a batch.log
        cp $conf_file jobs_completed/ 2> /dev/null
    fi
}

export -f job   # Exports the function so that it can be used in xargs.

# NOTE: `xargs` has been used here to parallelize calls to the function job. This is the meaning of
# its arguments:
#   -n 1          --> For each line passed as input, executes one single command.
#   -P $PROC_NUM  --> Use up to $PROC_NUM concurrent threads.
#   -I {}         --> Delimiter.
#
# See:  http://coldattic.info/post/7/
#       https://stackoverflow.com/a/11003457/1876268

sed -i '/^$/d' batch.conf
cat $BATCH_CONF | xargs -n 1 -P $MAX_PROCS -I {} bash -c 'job "{}"'
