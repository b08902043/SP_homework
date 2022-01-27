#!/bin/bash

echo_all_combination()
{
n_player=${1}
for ((i=1; i<=${n_player}; i++)); do             
  for ((j=$((i+1)); j<=${n_player}; j++)); do
    for ((k=$((j+1)); k<=${n_player}; k++)); do
      for ((l=$((k+1)); l<=${n_player}; l++)); do
        for ((m=$((l+1)); m<=${n_player}; m++)); do
          for ((n=$((m+1)); n<=${n_player}; n++)); do
            for ((o=$((n+1)); o<=${n_player}; o++)); do
              for ((p=$((o+1)); p<=${n_player}; p++)); do
                echo ${i} ${j} ${k} ${l} ${m} ${n} ${o} ${p}
                echo s
              done
            done
          done
        done
      done
    done
  done
done
}
#g++ host.c -o host

rm fifo_tmp*

while getopts m:n:l: flag
do 
    case "${flag}" in
        m) n_host=${OPTARG} ;;
        n) n_player=${OPTARG} ;;
        l) lucky_number=${OPTARG} ;;
    esac
done
#count the combination of players
player_combination_count=1

for((i=${n_player};i > 8;i --))
    do
    player_combination_count=$((player_combination_count * i))
    curr=$((curr-1))
    done
dividor=$((n_player-8))
for((i = ${dividor};i > 0;i --))
    do 
    player_combination_count=$((player_combination_count / i))
    done

# find how many host we need
if [ ${n_host} -le ${player_combination_count} ]; then
    min_host=${n_host}
else    
    min_host=${player_combination_count}
fi

# make fifos for hosts
for((i = 0;i <= ${min_host};i ++))
do
    mkfifo fifo_${i}.tmp
done
# connect  file descriptors with fifos
exec 3<>fifo_0.tmp
exec 4<>fifo_1.tmp
exec 5<>fifo_2.tmp
exec 6<>fifo_3.tmp
exec 7<>fifo_4.tmp
exec 8<>fifo_5.tmp
exec 9<>fifo_6.tmp
exec 10<>fifo_7.tmp
exec 11<>fifo_8.tmp
exec 12<>fifo_9.tmp
exec 13<>fifo_10.tmp
#find all combinations
all_combination=$(echo_all_combination ${n_player})
combinations=()
for ((i=1; i<=${player_combination_count}; i++)); do
    combinations[${i}]=$(echo ${all_combination} | cut -d"s" -f ${i})
  done

#set hosts
for((i = 1;i <= ${min_host};i ++))
do
    ./host "-m" ${i} "-d" 0 "-l" ${lucky_number} &
done
#init the player score
player_list=()
for((i=1; i<=${n_player}; i++)); do
    player_list[${i}]=0
  done

#send combination to host
for((i=1; i<=${n_host}; i++))
do 
    echo ${combinations[${i}]}>fifo_${i}.tmp
done
split=()
#read from fifo and send new combination to available host
for ((i=$((${n_host}+1)); i<=${player_combination_count}; i++)); do
    read avail_host<&3
    for((j=1; j<=8; j++)); do
        read word<fifo_0.tmp
        #split player and score
        count=0
        for curr in $word
        do
            split[${count}]=${curr}
            count=$((${count}+1))
        done
        player_list[${split[0]}]=$((${player_list[${split[0]}]}+${split[1]}))
      done   
    echo ${combinations[${i}]}>fifo_${avail_host}.tmp
  done
#read remaind combinations
for((i=1;i<=${min_host};i++)); do
    read avail_host<fifo_0.tmp
    for((j=1; j<=8; j++)); do
        read word<fifo_0.tmp
        count=0
        for curr in $word
        do
            split[${count}]=${curr}
            count=$((${count}+1))
        done
        player_list[${split[0]}]=$((${player_list[${split[0]}]}+${split[1]}))
      done
  done
#send end message
for((i=1;i<=10;i++)); do
    echo "-1 -1 -1 -1 -1 -1 -1 -1\n">fifo_${i}.tmp
  done

wait
#init ranklist
ranklist=()
for((i=0;i <= ${n_player};i ++));
do
    ranklist[${i}]=0
done

#find the rank

finished=1
rank=1
while [ "${finished}" != "0" ]
do
    largest=0
    count=0
    for ((i=1;i <= ${n_player};i ++));
    do
        if [ ${player_list[${i}]} -gt ${largest} ];
        then
            largest=${player_list[${i}]}
        fi
    done
    for ((i=1;i <= ${n_player};i ++));
    do
        if [ ${player_list[${i}]} -eq ${largest} ] && [ ${ranklist[${i}]} -eq "0" ];
        then
            ranklist[${i}]=${rank}
            player_list[${i}]=0
            count=$((${count}+1))
        fi
    done
    rank=$((${rank}+${count}))
    finished=${largest}
done
#list players rank
for((i=1; i<=${n_player}; i++)); do
    #echo ${i} ${player_list[${i}]}
    echo ${i} ${ranklist[${i}]}
  done
rm fifo*


