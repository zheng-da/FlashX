#!/bin/sh

set_affinity_node ()
{
	start_cpu=$1
	start_irq=$2
	for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
	do
		cpu_num=$(($start_cpu + $i * 4))
#		cpu_num=$(($cpu_num % 32))
		#cpu_num=$(($start_cpu))
		irq_num=$(($start_irq + $i))
		echo $cpu_num > /proc/irq/$irq_num/smp_affinity_list
		echo "set $irq_num to $cpu_num"
		cat /proc/irq/$irq_num/smp_affinity_list
	done
}

if [ -f /etc/init.d/irq_balancer ] ; then
    /etc/init.d/irq_balancer stop
elif [ -f /etc/init.d/irqbalance ]; then
    /etc/init.d/irqbalance stop
else
    exit 1;
fi;

irq0=`grep mpt2sas0-msix0 /proc/interrupts | awk '{print $1}' | awk -F : '{print $1}'`
irq1=`grep mpt2sas1-msix0 /proc/interrupts | awk '{print $1}' | awk -F : '{print $1}'`
irq2=`grep mpt2sas2-msix0 /proc/interrupts | awk '{print $1}' | awk -F : '{print $1}'`
echo "irq0: $irq0"
echo "irq1: $irq1"
echo "irq2: $irq2"

echo "set for controller 0"
set_affinity_node 1 $irq0
echo "set for controller 1"
set_affinity_node 1 $irq1
echo "set for controller 2"
set_affinity_node 2 $irq2
