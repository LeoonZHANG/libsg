#!/bin/bash
function extract_duplicate_object()
{
    ar -t $1 | sort | uniq -c | grep -v '^ *1 ' | \
    while read line
    do
        read num obj <<< $line
        if [ "$num" ]
        then
            i=1
            while [ $i -le $num ]
            do
                ar -xN $i $archive $obj
                mv $obj $i"_"$obj
                i=$(($i + 1))
            done
        fi
    done
}

for archive in $@
do
    mkdir -p combines/$(basename $archive)
    pushd combines/$(basename $archive)
    ar -x $archive
    extract_duplicate_object $archive
    for obj in *.o
    do
        mv $obj $(basename $archive)"-"$obj
    done
    popd
done
rm libsg.a
ar -rs libsg.a combines/*/*.o
rm -rf combines
