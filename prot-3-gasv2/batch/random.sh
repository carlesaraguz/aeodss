OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/dbg_goal/dbg_goal_1bl.yml -l ../data/2019_05_19_072002_dbg_goal_1bl_0/system.yml  --random -d ../data/2019_05_19_072002_dbg_goal_1bl_rand/ > job_logs/dbg_goal_1bl_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/dbg_goal/dbg_goal_2as.yml -l ../data/2019_05_18_205551_dbg_goal_2as_1/system.yml  --random -d ../data/2019_05_18_205551_dbg_goal_2as_rand/ > job_logs/dbg_goal_2as_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/dbg_goal/dbg_goal_2bs.yml -l ../data/2019_05_18_232845_dbg_goal_2bs_0/system.yml  --random -d ../data/2019_05_18_232845_dbg_goal_2bs_rand/ > job_logs/dbg_goal_2bs_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/dbg_goal/dbg_goal_2cs.yml -l ../data/2019_05_19_015716_dbg_goal_2cs_0/system.yml  --random -d ../data/2019_05_19_015716_dbg_goal_2cs_rand/ > job_logs/dbg_goal_2cs_rand.log 2>&1 &
echo -n "Waiting for the jobs to complete..."
wait $(jobs -p)
echo " done."
