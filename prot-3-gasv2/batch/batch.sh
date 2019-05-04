#!/bin/bash

: ${MAX_PROCS:=2}

mkdir -p job_logs
mkdir -p jobs_completed

function job {
    # $1 -> file name (config YAML)
    data_path=../data/  # Where to save results.
    bin=../bin/prot-3   # The binary to execute.
    dirdate=$(date +%Y_%m_%d_%H%M%S)
    dircount=0
    simulation_name=$(basename $1 | sed 's/\.[^.]*$//')
    resdir="$data_path$dirdate"
    resdir+="_$simulation_name"
    while [ -d $resdir ]; do
        resdir="$data_path$dirdate"
        resdir+="_$simulation_name"
        resdir+="_$dircount"
        dircount+=1
    done
    log_file="job_logs/$simulation_name.log"
    count=0
    while [ $count -lt 5 ]; do
        rm $resdir -r 2> /dev/null  # Clean previous contents if there were some.
        # echo "-f ../batch/set/$1 -d $resdir/" > $log_file
        $bin --simple-log -g0 -f ../batch/set/$1 -d $resdir/ > $log_file 2>&1
        if [[ $? != 0 ]]; then
            count+=1
        else
            break
        fi
    done
    completed_data=$(date +"%F %T")
    if [ $count -ge 5 ]; then
        # Failed!
        printf "$completed_data %20s -- [ FAIL ] : $resdir\n" $1 | tee -a batch.log
    else
        # Succeeded:
        printf "$completed_data %20s -- [ OK-$count ] : $resdir\n" $1 | tee -a batch.log
        mv set/$1 jobs_completed/ 2> /dev/null
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

ls set | xargs -n 1 -P $MAX_PROCS -I {} bash -c 'job "{}"'
