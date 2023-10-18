#!/bin/sh

PROG="./simscript"
TESTS="./tests/*.ss"
COUNT=$(ls ${TESTS} | wc -l)
PASSED=0
N=1
for TEST in ./tests/*.ss; do
    CURRENT=$(echo "${TEST}" | grep -Po "tests/test_[0-9]+_\K[a-zA-Z]+")
    printf "\ttesting %-12s [%2d/%2d]\t......\t" ${CURRENT} ${N} ${COUNT}
    OUTPUT=$(timeout 3s ${PROG} ${TEST} 2> /dev/null | grep -Eo "FAIL")
    if [[ -n ${OUTPUT} ]]; then
        printf "\033[0;33mFAIL\033[0m\n"
    else
        printf "\033[0;32mPASS\033[0m\n"
        ((PASSED++))
    fi
    ((N++))
done

echo "Passed ${PASSED}/${COUNT}"
