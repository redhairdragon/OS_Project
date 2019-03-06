rm -f lab2b_list.csv 

t_num=(1, 2, 4, 8, 12, 16, 24)

for t in ${t_num[@]};
do
	./lab2_list --threads=$t --iterations=1000 --sync=m >>lab2b_list.csv
	./lab2_list --threads=$t --iterations=1000 --sync=s >>lab2b_list.csv
done 	


echo "-------OK to see error---------"
t_num=(1, 4, 8, 12， 16)
i_num=(1, 2, 4, 8, 16)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 >>lab2b_list.csv
	done
done 	
echo "-------------end----------------"


i_num=(10, 20, 40, 80)
for t in ${t_num[@]};
do
	for i in ${i_num[@]};
	do
		./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 --sync=m >>lab2b_list.csv
		./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 --sync=s >>lab2b_list.csv
	done
done 	

t_num=(1, 4, 8, 12)
l_num=(4, 8, 16) 

for t in ${t_num[@]};
do
	for l in ${l_num[@]};
	do
		./lab2_list --threads=$t --iterations=1000 --sync=m --lists=$l >>lab2b_list.csv
		./lab2_list --threads=$t --iterations=1000 --sync=s --lists=$l >>lab2b_list.csv
	done
done 	
