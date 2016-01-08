#!/bin/bash
N_OFFICES=$1
N_CENTERS=$2
N_SEGMENTS=$3
REQ_CAPACITY=0
TOTAL_CAPACITY=0

echo "/*********************************************"
echo " * OPL 12.4 Data"
echo " * Author: jbosch"
echo " * Creation Date: Dec 12, 2015 at 3:25:31 PM"
echo " *********************************************/"
echo ""
echo " nOffices = $N_OFFICES;"
echo " nBackupCenters = $N_CENTERS;"
echo " nSegments = $N_SEGMENTS;"
echo ""
echo " // Demand for each office (in PB)"
echo -n " demand = [ "

RANDOM=1 #Set the seed of random
for ((O=1; O <= $N_OFFICES; O++))
do
    DEMAND=$(( ( RANDOM % 10 ) + 1 ))
    REQ_CAPACITY=$(( $REQ_CAPACITY + $DEMAND ))
    echo -n "$DEMAND "
done

echo "];"
echo "" 
echo " // Whether office o is conected with each center c"
echo -n " united = [ "

for ((O=1; O <= $N_OFFICES; O++))
do
    echo -n "[ "
    for ((C=1; C <= $N_CENTERS; C++))
    do
        echo -n "1 "
    done
    echo -n "] "
done

echo "];"
echo ""
echo " // Capacity of each backup center (in PB)"
echo -n " capacity = [ " #4 4 30 4 30];

RANDOM=1 #Set the seed of random
MIN_CAPACITY=$(( ( $REQ_CAPACITY / $N_CENTERS + 1 ) * 2 ))
#MIN_CAPACITY=6
MAX_CAPACITY=$(( $REQ_CAPACITY / $N_OFFICES + 1 ))
for ((C=1; C <= $N_CENTERS; C++))
do
    CAPACITY=$(( ( RANDOM % $MAX_CAPACITY ) + $MIN_CAPACITY ))
    TOTAL_CAPACITY=$(( $TOTAL_CAPACITY + $CAPACITY ))
    echo -n "$CAPACITY "
done

echo "];"
echo ""
echo " // Fixed cost of using each backup center (in thousands of euros)"
echo -n " fixedCost = [ " #3 2 30 15 50];

RANDOM=1
for ((C=1; C <= $N_CENTERS; C++))
do
    COST=$(( ( RANDOM % 50 ) + 10 ))
    echo -n "$COST "
done

echo "];"
echo ""
echo " // Cost of one PB in each segment"
echo -n " costPerPB = [ " #3 2 1];

for ((COST=${N_SEGMENTS}; COST >= 1; COST--));
do
    echo -n "$COST "
done

echo "];"
echo ""
echo " // Minimum number of PB to apply the segment price"
echo -n " minimumPB = [ 1 " #1 10 20];

SEG_STEP=$(( $REQ_CAPACITY / ( $N_SEGMENTS * 2 ) + 1 ))
for ((S=2; S <= $N_SEGMENTS; S++));
do
    MIN=$(( ( $S - 1 ) * $SEG_STEP ))
    echo -n "$MIN "
done

echo "];"
