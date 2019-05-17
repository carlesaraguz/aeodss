OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_1a.yml -l ../data/2019_05_13_131619_energy_1a_0/system.yml  --random -d ../data/2019_05_13_131619_energy_1a_rand/ > job_logs/energy_1a_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_1b.yml -l ../data/2019_05_12_204749_energy_1b_0/system.yml  --random -d ../data/2019_05_12_204749_energy_1b_rand/ > job_logs/energy_1b_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_1c.yml -l ../data/2019_05_12_204749_energy_1c_0/system.yml  --random -d ../data/2019_05_12_204749_energy_1c_rand/ > job_logs/energy_1c_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_1d.yml -l ../data/2019_05_13_145303_energy_1d_01/system.yml --random -d ../data/2019_05_13_145303_energy_1d_rand/ > job_logs/energy_1d_rand.log 2>&1 &
echo -n "Waiting for the jobs to complete..."
wait $(jobs -rp)
echo " done."
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_2a.yml -l ../data/2019_05_14_202708_energy_2a_0/system.yml  --random -d ../data/2019_05_14_202708_energy_2a_rand/ > job_logs/energy_2a_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_2b.yml -l ../data/2019_05_15_004025_energy_2b_0/system.yml  --random -d ../data/2019_05_15_004025_energy_2b_rand/ > job_logs/energy_2b_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_2c.yml -l ../data/2019_05_15_053405_energy_2c_1/system.yml  --random -d ../data/2019_05_15_053405_energy_2c_rand/ > job_logs/energy_2c_rand.log 2>&1 &
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_3a.yml -l ../data/2019_05_15_183002_energy_3a_0/system.yml  --random -d ../data/2019_05_15_183002_energy_3a_rand/ > job_logs/energy_3a_rand.log 2>&1 &
echo -n "Waiting for the jobs to complete..."
wait $(jobs -rp)
echo " done."
OMP_NUM_THREADS=10 ../bin/prot-3 --simple-log -g0 -f ../batch/set/energy_3b.yml -l ../data/2019_05_15_220537_energy_3b_0/system.yml  --random -d ../data/2019_05_15_220537_energy_3b_rand/ > job_logs/energy_3b_rand.log 2>&1 &
echo "Random tests completed."
