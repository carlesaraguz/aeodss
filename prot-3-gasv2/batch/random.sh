echo -n "Running 5b_rand... "
../bin/prot-3 --simple-log -g0 -f ../batch/set/energy/energy_5b.yml -l ../data/2019_05_28_044002_energy_5b_0/system.yml  --random -d ../data/2019_05_28_044002_energy_5b_rand/ > job_logs/energy_5b_rand.log 2>&1
echo "done."
echo -n "Running 5c_rand... "
../bin/prot-3 --simple-log -g0 -f ../batch/set/energy/energy_5c.yml -l ../data/2019_05_28_150856_energy_5c_1/system.yml  --random -d ../data/2019_05_28_150856_energy_5c_rand/ > job_logs/energy_5c_rand.log 2>&1
echo " done."
