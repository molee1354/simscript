#!/bin/sh

PROG="./simscript"
TESTS="./tests/*.ss"
TEMP="stderr.temp"
COUNT=$(ls ${TESTS} | wc -l)
((COUNT--))
PASSED=0
N=1

if [[ -e ${PROG} ]]; then
    echo Using ${PROG}
else
    echo Cannot find ${PROG}
    exit
fi

printf "Should performance benchmarks be run? [y/N] : "
read SKIP_BENCHMARK

touch ${TEMP}
for TEST in ./tests/*.ss; do
    CURRENT=$(echo "${TEST}" | grep -Po "tests/test_[0-9]+_\K[a-zA-Z]+")
    if [[ ${CURRENT} == "benchmark" ]]; then
        if [[ ${SKIP_BENCHMARK} != "y" ]]; then
            continue
        fi
        rm ${TEMP}
        printf "\n\tRunning benchmark...\n"
        ${PROG} ${TEST}
        continue
    fi
    printf "\ttesting %-12s [%2d/%2d]\t......\t" ${CURRENT} ${N} ${COUNT}
    
    # Run the command with a timeout
    OUTPUT=$(timeout 3s ${PROG} ${TEST} 2> ${TEMP})
    EXIT_STATUS=$?

    if [ $EXIT_STATUS -eq 124 ]; then
        printf "\033[0;33mFAIL\033[0m\n"
        ((N++))
        continue
    fi

    RESULT=$(grep -Eo "FAIL" <<< "$OUTPUT")
    RESULT+=$(cat ${TEMP} | grep -Eo "ERROR|Undefined|Error")
    if [[ -n ${RESULT} ]]; then
        if [[ ${CURRENT} == "let" || ${CURRENT} == "const" ]]; then
            printf "\033[0;32mPASS\033[0m\n"
            ((PASSED++))
        else
            printf "\033[0;33mFAIL\033[0m\n"
        fi
    else
        printf "\033[0;32mPASS\033[0m\n"
        ((PASSED++))
    fi

    ((N++))
done

if [[ -e ${TEMP} ]]; then
    rm ${TEMP}
fi
echo -e "\nPassed ${PASSED}/${COUNT}"
