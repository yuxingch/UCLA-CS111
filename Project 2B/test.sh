# for lab2b_1.png
# the total number of operations per second for each synchronization method
#	mutex
#./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_0_list.csv
#./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_0_list.csv
#	spin-lock
./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

# for lab2b_2.png
./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

# for lab2b_3.png
#Confirm the correctness of your new implementation
./lab2_list --threads=1  --iterations=1  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=1  --iterations=2  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=1  --iterations=4  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=1  --iterations=8  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=1  --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=2  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=4  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=8  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=2  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=4  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=8  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=12 --iterations=2  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=12 --iterations=4  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=12 --iterations=8  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=16 --iterations=2  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=16 --iterations=4  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=16 --iterations=8  --lists=4 --yield=id >> lab2b_list.csv
./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

#method=mutex
./lab2_list --threads=1  --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=4	 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=8	 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

#method=sync
./lab2_list --threads=1  --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=1	 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

#lab2_4.png
#./lab2_list --threads=1  --iterations=1000 --lists=1  --sync=m >> lab2_list.csv
#./lab2_list --threads=1	 --iterations=1000 --lists=1  --sync=m >> lab2_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=4  --sync=m >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=4  --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=4  --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=4  --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=4  --sync=m >> lab2b_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=8  --sync=m >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=8  --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=8  --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=8  --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=8  --sync=m >> lab2b_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

#./lab2_list --threads=1  --iterations=1000 --lists=1  --sync=s >> lab2_list.csv
#./lab2_list --threads=2  --iterations=1000 --lists=1  --sync=s >> lab2_list.csv
#./lab2_list --threads=4  --iterations=1000 --lists=1  --sync=s >> lab2_list.csv
#./lab2_list --threads=8  --iterations=1000 --lists=1  --sync=s >> lab2_list.csv
#./lab2_list --threads=12 --iterations=1000 --lists=1  --sync=s >> lab2_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=4  --sync=s >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=4  --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=4  --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=4  --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=4  --sync=s >> lab2b_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=8  --sync=s >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=8  --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=8  --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=8  --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=8  --sync=s >> lab2b_list.csv

./lab2_list --threads=1  --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
