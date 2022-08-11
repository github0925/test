#!/bin/bash
CRTDIR=$(pwd)

echo "running in ${CRTDIR}"

if [ ! -d $CRTDIR/board/ ];then
  echo -e "\\033[31mOpps,the board dir is not exist !! \033[0m"
  exit 1
fi

dir=$(ls -l $CRTDIR/board/ |awk '/^d/ {print $NF}')
# ospi1 only
for board in $dir
do
    # bf200  an g9_k1 is  not include in this build
    if [[ $(echo $board | grep "bf200") == "" ]] && [[ $(echo $board | grep "g9_k1") == "" ]]
    then
        make clean
        echo "make  unified_boot BOARD=$board CFG=t_loader"
        make  unified_boot BOARD=$board CFG=t_loader
        if [ $? -ne 0 ]; then
                echo -e "\\033[31mOpps, build $board fail !! \033[0m"
                exit 1
        fi
        make clean
        echo "make  bin BOARD=$board CFG=t_loader DIL2=1"
        make  bin BOARD=$board CFG=t_loader DIL2=1
        if [ $? -ne 0 ]; then
                echo -e "\\033[31mOpps, build $board fail !! \033[0m"
                exit 1
        fi
    fi
done
#emmmc1 only
for board in $dir
do
  # bf200  an g9  is  not include in this build
  if [[ $(echo $board | grep "g9") == "" ]] && [[ $(echo $board | grep "bf200") == "" ]] && [[ $(echo $board | grep "g9_k1") == "" ]]
    then
        make clean
        echo "make unified_boot BOARD=$board CFG=t_loader TGT=sec"
        make  unified_boot BOARD=$board CFG=t_loader TGT=sec
        if [ $? -ne 0 ]; then
                echo -e "\\033[31mOpps, build $board fail !! \033[0m"
                exit 1
        fi
        make clean
        echo "make bin PEER_LOAD=1 BOARD=$board CFG=t_loader DIL2=1"
        make  bin PEER_LOAD=1 BOARD=$board CFG=t_loader DIL2=1
        if [ $? -ne 0 ]; then
                echo -e "\\033[31mOpps, build $board fail !! \033[0m"
                exit 1
        fi
  fi
done

#bin_packer
echo "make -C tools/bin_packer"
make -C tools/bin_packer
if [ $? -ne 0 ]; then
        echo -e "\\033[31mOpps, build  bin_packer fail !!\033[0m"
        exit 1
fi


#ddr_seq_parser
echo "make -C tools/ddr_seq_parser"
make -C tools/ddr_seq_parser
if [ $? -ne 0 ]; then
        echo -e "\\033[31mOpps, ddr_seq_parser build fail !! \033[0m"
        exit 1
fi



#ddr_squeezer
if [ ! -e $CRTDIR/mk.sh ];then
  echo -e "\\033[31mOpps,the ddr_squeezer mk.sh is not exist !! \033[0m"
  exit 1
fi

echo ".mk.sh  all"
./mk.sh  all
if [ $? -ne 0 ]; then
    echo -e "\\033[31mOpps,build ddr_suqeezer fail !! \033[0m"
    exit 1
fi
echo "make CFG=bf200_irq_route bin"
make clean
make CFG=bf200_irq_route bin
if [ $? -ne 0 ]; then
    echo -e "\\033[31mOpps,build bf200 fail !! \033[0m"
    exit 1
fi

echo -e "\\033[32mAll build item pass.\n\n \033[0m"
exit 0
