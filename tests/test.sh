#!/bin/sh

PROG="./simscript"
TESTS="./tests/*.ss"
TEMP="stderr.temp"
COUNT=$(ls ${TESTS} | wc -l)
PASSED=0
N=1

touch ${TEMP}
for TEST in ./tests/*.ss; do
    CURRENT=$(echo "${TEST}" | grep -Po "tests/test_[0-9]+_\K[a-zA-Z]+")
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
    RESULT+=$(cat ${TEMP} | grep -Eo "ERROR|Undefined")
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

rm ${TEMP}
echo "Passed ${PASSED}/${COUNT}"
