#!/bin/bash 


rm -f lab2_add.csv 
#add none
#
t_num=(2, 4, 8)
i_num=(100, 1000, 10000, 100000)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_add --threads=$t --iterations=$i >>lab2_add.csv 
	done
done 	

#add yield none
#lab2_add-1.png

t_num=(2, 4, 8, 12)
i_num=(10, 20, 40, 80, 100, 1000, 10000, 100000)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_add --threads=$t --iterations=$i --yield>>lab2_add.csv 
	done
done 	

#lab2_add-2.png
i_num=(100, 1000, 10000, 100000)
for t in {2..8};
do 
	for i in ${i_num[@]};
	do
		./lab2_add --threads=$t --iterations=$i --yield>>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i >>lab2_add.csv
	done
done

i_num=(1, 10, 100, 1000, 10000, 100000)
for i in ${i_num[@]};
do
	./lab2_add --threads=1 --iterations=$i >>lab2_add.csv
done

i_num=(1, 10, 100, 1000, 10000, 100000)
t_num=(1, 2, 4, 8, 12)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_add --threads=$t --iterations=$i >>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i --sync=s >>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i --sync=c >>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i --sync=m >>lab2_add.csv 

		./lab2_add --threads=$t --iterations=$i --yield>>lab2_add.csv
		./lab2_add --threads=$t --iterations=$i --sync=s --yield>>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i --sync=c --yield>>lab2_add.csv 
		./lab2_add --threads=$t --iterations=$i --sync=m --yield>>lab2_add.csv 
	done
done 	


rm -f lab2_list.csv 

i_num=(10, 100, 1000, 10000, 20000)

for i in ${i_num[@]};
do
	./lab2_list --threads=1 --iterations=$i >>lab2_list.csv 
done


t_num=(2, 4, 8, 12)
i_num=(1, 10, 100, 1000)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_list --threads=$t --iterations=$i >>lab2_list.csv
	done
done 	

t_num=(2, 4, 8, 12)
i_num=(1, 2, 4, 8, 16, 32)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_list --threads=$t --iterations=$i >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --yield=i >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --yield=d >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --yield=il >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --yield=dl >>lab2_list.csv

	done
done 	

t_num=(2, 4, 8, 12)
i_num=(1, 2, 4, 8, 16, 32)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_list --threads=$t --iterations=$i --sync=m >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=m --yield=i >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=m --yield=d >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=m --yield=il >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=m --yield=dl >>lab2_list.csv

		./lab2_list --threads=$t --iterations=$i --sync=s >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=s --yield=i >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=s --yield=d >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=s --yield=il >>lab2_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=s --yield=dl >>lab2_list.csv
	done
done 	

t_num=(1, 2, 4, 8, 12, 16, 24)
for t in ${t_num[@]};
do
	./lab2_list --threads=$t --iterations=1000 --sync=m>>lab2_list.csv
	./lab2_list --threads=$t --iterations=1000 --sync=s>>lab2_list.csv
done

