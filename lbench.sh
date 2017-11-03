#!/bin/bash

PROGNAME=$(basename $0)

SYSFS_CPU="/sys/devices/system/cpu"
SYSFS_NODE="/sys/devices/system/node"

CPU_POSSIBLE_COUNT=$(ls -d ${SYSFS_CPU}/cpu[0-9]* | wc -l)
NODE_POSSIBLE_COUNT=$(ls -1d ${SYSFS_NODE}/node[0-9]* | wc -l)

echo ">>>$PROGNAME: Running one thread on different nodes"
for ((j=0;j < ${NODE_POSSIBLE_COUNT} ;j++)); do
	core=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | head -1`
	core=`basename $core | sed s/cpu//`
	for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
		echo ">>>$PROGNAME: Running between node $i and core $core"
		numactl --membind=$i --physcpubind=$core ./lbench 1 1
	done
done
echo

if [ $NODE_POSSIBLE_COUNT -ge 2 ]; then	
	echo ">>>$PROGNAME: Running two threads on different nodes"
	for ((j=0;j < ${NODE_POSSIBLE_COUNT} ;j++)); do
		core0=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n1`
		core0=`basename $core0 | sed s/cpu//`

		core1=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n2`
		core1=`basename $core1 | sed s/cpu//`

		for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
			echo ">>>$PROGNAME: Running between node $i and cores $core0, $core1"
			numactl --membind=$i --physcpubind=$core0,core$1 ./lbench 2 1
		done
	done
	echo
fi

if [ $NODE_POSSIBLE_COUNT -ge 4 ]; then	
	echo ">>>$PROGNAME: Running four threads on different nodes"
	for ((j=0;j < ${NODE_POSSIBLE_COUNT} ;j++)); do
		core0=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n1`
		core0=`basename $core0 | sed s/cpu//`

		core1=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n2`
		core1=`basename $core1 | sed s/cpu//`

		core2=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n3`
		core2=`basename $core2 | sed s/cpu//`

		core3=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n4`
		core3=`basename $core3 | sed s/cpu//`				

		for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
			echo ">>>$PROGNAME: Running between node $i and cores $core0, $core1, $core2, $core3"
			numactl --membind=$i --physcpubind=$core0,core$1,core$2,core$3 ./lbench 4 1
		done
	done
	echo
fi

if [ $NODE_POSSIBLE_COUNT -ge 8 ]; then	
	echo ">>>$PROGNAME: Running four threads on different nodes"
	for ((j=0;j < 4 ;j++)); do
		core0=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n1`
		core0=`basename $core0 | sed s/cpu//`

		core1=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n2`
		core1=`basename $core1 | sed s/cpu//`

		core2=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n3`
		core2=`basename $core2 | sed s/cpu//`

		core3=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n4`
		core3=`basename $core3 | sed s/cpu//`				

		for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
			echo ">>>$PROGNAME: Running between node $i and cores $core0, $core1, $core2, $core3"
			numactl --membind=$i --physcpubind=$core0,core$1,core$2,core$3 ./lbench 4 1
		done
	done
	echo
fi