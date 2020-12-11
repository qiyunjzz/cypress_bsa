#!/bin/bash
#set -x
if [ $1 == "make" ];then
    app_path=`find ./ -type d -name "$2"` && echo $app_path &&cd $app_path/build && make && cp ./x86_64/$2 ~/work/bsa/bsa0107_28/out/ && cd -
    exit 0
elif [ $1 == "clean" ];then
    find ./ -name '*.o*' -exec rm -f {} \;
    exit 0
elif [ $1 == "cp_app" ];then
    app_path=`find ./ -type d -name "$2"` && echo $app_path && cp $app_path/build/x86_64/$2 ~/work/bsa/bsa0107_28/out/
    exit 0
fi
