#!/bin/bash
rm -f test.log
count = 0
for file in test_*; do 
    if [ -x "$file" ]; then
        let "count=count+1"
        echo "./$file -a"
        ./$file -a 
        if [[ $? -ne 0 ]]; then
            exit 1
        fi
    fi
done
echo -e "\nTotals"
echo -n "Passed :"; grep -o ':PASSED:' test.log | wc -l
echo -n "Skipped:"; grep -o ':SKIPPED:' test.log | wc -l
echo -n "Failed :"; grep -o ':FAILED:' test.log | wc -l
echo -e "\nTotal Test Suites:" $count
