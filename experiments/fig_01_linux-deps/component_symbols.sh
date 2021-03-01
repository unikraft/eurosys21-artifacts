components=("fs" "ipc" "mm" "net" "security" "block")


#### FInd all the function calls in the module
MYPWD=${PWD}
for i in "${components[@]}"; do 
	echo "Extracting symbols for $i"
	cd linux/$i && cscope -R -L -2 ".*" | grep -v "extern" | grep -v "/usr/" | awk -F ' ' '{print $2}' | sort | uniq  > $MYPWD/$i.symbols
	cd $MYPWD
	#rm -rf cscope.*
done

cd $MYPWD

#### We now find the definition of each call, later during processing we will plot the data.
for i in "${components[@]}"; do 
	echo "Writing calees for each function from $i"

	echo "" > $i.deps;
	while read in; do 
		#cscope -d -L3 $in | awk -F '.c' '{print $1}' | uniq | grep linux >> $i.deps;
		if [[ $(cscope -d -L1 $in | uniq | grep linux | wc -l) -le 1 ]]; then
			cscope -d -L1 $in | uniq | grep linux >> $i.deps;
		fi
	done < $i.symbols
done



components=("sched" "time" "irq" "locking")

#### FInd all the function calls in the module -> same as before, added a kernel/ to path
for i in "${components[@]}"; do 
	echo "Extracting symbols for $i"
	cd linux/kernel/$i && cscope -R -L -2 ".*" | awk -F ' ' '{print $2}' | grep -v "extern" | grep -v "/usr/"  > $MYPWD/$i.symbols
	cd $MYPWD
	#rm -rf cscope.*
done

cd $MYPWD

for i in "${components[@]}"; do 
	echo "Writing calees for each function from $i"

	echo "" > $i.deps;
	while read in; do 
		#cscope -d -L3 $in | awk -F '.c' '{print $1}' | uniq | grep linux >> $i.deps;
		if [[ $(cscope -d -L1 $in | uniq | grep linux | wc -l) -le 1 ]]; then
			cscope -d -L1 $in | uniq | grep linux >> $i.deps;
		fi
	done < $i.symbols
done
