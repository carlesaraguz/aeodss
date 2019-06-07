#!/bin/bash

# Default values:
WORKERS=4
BATCH_CONF=batch.conf
MAX_CPUS=$(grep -c ^processor /proc/cpuinfo)
OMP_CPUS=$(($MAX_CPUS / $WORKERS))
ERR_COUNT=2

# Read command arguments:
POSITIONAL=()
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -j|--jobs)
        WORKERS="$2"
        if [ "$WORKERS" -gt 42 ]; then
            WORKERS=42
        fi
        OMP_CPUS=$(($MAX_CPUS / $WORKERS))
        shift # past argument
        shift # past value
        ;;
        -f|--file-config)
        BATCH_CONF="$2"
        shift # past argument
        shift # past value
        ;;
        -e|--error-count)
        ERR_COUNT=$(($2))
        shift # past argument
        shift # past value
        ;;
        -l|--limit-cpus)
        OMP_CPUS="$2"
        shift # past argument
        shift # past value
        ;;
        -m|--max-cpus)
        MAX_CPUS="$2"
        OMP_CPUS=$(($MAX_CPUS / $WORKERS))
        shift # past argument
        shift # past value
        ;;
        -h|--help)
        echo "-j | --jobs <numjobs>         Starts batch processing with <numjobs> concurrent processes."
        echo "-f | --file-config <filepath> Loads batch-configuration file from <filepath>."
        echo "-e | --error-count <numerrs>  Allows up to <numerrs> per simulation (and relaunch simulation until exhausted)."
        echo "-l | --limit-cpus <numcpus>   Limits the number of parallel processors to use in each process."
        echo "-m | --max-cpus <numcpus>     Defines a maximum number of CPU's to automatically balance parallel processors."
        echo "-h | --help                   Shows this help."
        exit
        shift # past argument
        ;;
        *)    # unknown option
        POSITIONAL+=("$1") # save it in an array for later
        shift # past argument
        ;;
    esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

sleep 1
echo -e "\nRunning batch simulations with $WORKERS workers. Concurrent threads per job: $OMP_CPUS."
echo "Reading batch configuration from '$BATCH_CONF'."
echo "Errors allowed after aborting case: '$ERR_COUNT'."

if [ ! -f "$BATCH_CONF" ]; then
    echo "Error: unable to find configuration file."
    exit
fi

mkdir -p job_logs
mkdir -p job_procinfo
rm job_procinfo/* 2> /dev/null

function job_meminfo {
    while [ 1 ]; do
        if [ -e /proc/$1/status ]; then
            mem_use=$(cat /proc/$1/status | grep VmPeak | awk '{print $2}')
            echo $mem_use > job_procinfo/$1.mem
            sleep 100
        else
            break
        fi
    done
}

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
    while [ ${count} -lt 2 ]; do
        # Loop until the folder name doesn't exist
        while [ -d $resdir ]; do
            dircount=$(($dircount + 1))
            resdir="$data_path$dirdate"
            resdir+="_$simulation_name"
            resdir+="_$dircount"
        done
        tstart=$(date +%s)

        if [ -f "$load_file" ]; then
            OMP_NUM_THREADS=$OMP_CPUS $bin --simple-log -g0 -f ../batch/$conf_file -l $load_file -d $resdir/ > $log_file 2>&1 &
            pid_sim=$!
            job_meminfo $pid_sim &
            wait $pid_sim
            exit_value=$?
            memkb=$(cat job_procinfo/$pid_sim.mem)
            memgb=$(echo "scale=1; $memkb/1024/1024" | bc -l)
            cp $log_file $resdir/ 2> /dev/null
        else
            OMP_NUM_THREADS=$OMP_CPUS $bin --simple-log -g0 -f ../batch/$conf_file -d $resdir/ > $log_file 2>&1 &
            pid_sim=$!
            job_meminfo $pid_sim &
            wait $pid_sim
            exit_value=$?
            memkb=$(cat job_procinfo/$pid_sim.mem)
            memgb=$(echo "scale=1; $memkb/1024/1024" | bc -l)
            cp $log_file $resdir/ 2> /dev/null
        fi
        tend=$(date +%s)
        tspan=$(($tend - $tstart))
        tspan_days=$(($tspan / 86400))
        tspan=$(($tspan - ($tspan_days * 86400) ))
        tspan_str=$(date -u -d @${tspan} +"%T")
        if [[ $exit_value != 0 ]]; then
            completed_date=$(date +"%F %T")
            printf "$completed_date %20s -- (%dd $tspan_str, %4.1f GB) [ FAIL ] : $resdir [C:$count, E:$exit_value]\n" $simulation_name $tspan_days $memgb | tee -a batch.log
            count=$(($count + 1))
        else
            break
        fi
    done
    completed_date=$(date +"%F %T")
    if [ ${count} -ge 2 ]; then
        # Failed!
        printf "$completed_date %20s -- (%dd $tspan_str, %4.1f GB) [ FAIL ] : $resdir [aborted]\n" $simulation_name $tspan_days $memgb | tee -a batch.log
    else
        # Succeeded:
        printf "$completed_date %20s -- (%dd $tspan_str, %4.1f GB) [ OK-$count ] : $resdir\n" $simulation_name $tspan_days $memgb | tee -a batch.log
    fi
    # Run the random version:
    count=0
    simulation_name+="_rand"
    log_file="job_logs/$simulation_name.log"
    randresdir="$data_path$dirdate"
    randresdir+="_$simulation_name"
    load_file="$resdir/system.yml"
    tstart=$(date +%s)
    OMP_NUM_THREADS=$OMP_CPUS $bin --simple-log -g0 -f ../batch/$conf_file -l $load_file --random -d $randresdir/ > $log_file 2>&1 &
    pid_sim=$!
    job_meminfo $pid_sim &
    wait $pid_sim
    exit_value=$?
    memkb=$(cat job_procinfo/$pid_sim.mem)
    memgb=$(echo "scale=1; $memkb/1024/1024" | bc -l)
    cp $log_file $randresdir/ 2>/dev/null
    tend=$(date +%s)
    tspan=$(($tend - $tstart))
    tspan_days=$(($tspan / 86400))
    tspan=$(($tspan - ($tspan_days * 86400) ))
    tspan_str=$(date -u -d @${tspan} +"%T")
    completed_date=$(date +"%F %T")
    if [ $exit_value != 0 ]; then
        printf "$completed_date %20s -- (%dd $tspan_str, %4.1f GB) [ FAIL ] : $randresdir [aborted, E:$exit_value]\n" $simulation_name $tspan_days $memgb | tee -a batch.log
    else
        printf "$completed_date %20s -- (%dd $tspan_str, %4.1f GB) [ OK-$count ] : $randresdir\n" $simulation_name $tspan_days $memgb | tee -a batch.log
    fi
}

export -f job           # Exports the function so that it can be used in xargs.
export -f job_meminfo   # Exports the function so that it can be used within job.

# NOTE: `xargs` has been used here to parallelize calls to the function job. This is the meaning of
# its arguments:
#   -n 1          --> For each line passed as input, executes one single command.
#   -P $PROC_NUM  --> Use up to $PROC_NUM concurrent threads.
#   -I {}         --> Delimiter.
#
# See:  http://coldattic.info/post/7/
#       https://stackoverflow.com/a/11003457/1876268

sed -i '/^$/d' batch.conf
cat $BATCH_CONF | xargs -n 1 -P $WORKERS -I {} bash -c 'job "{}"'
