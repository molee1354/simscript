#!/bin/bash

PROG="./simscript"
VERSIONFILE="src/main.c"
VERSION=$(cat ${VERSIONFILE} | grep -oP "#define.*?VERSION.*?\"\K[0-9].[0-9].[0-9]")
TESTS="./tests/*.ss"
TEMP="stderr.temp"

COUNT=$(ls ${TESTS} | wc -l)
((COUNT--))
PASSED=0
N=1

# Checking for simscript binary
if [[ -e ${PROG} ]]; then
    echo Using ${PROG}
else
    echo Cannot find ${PROG}
    exit
fi

# Prompting for benchmarks
printf "Should performance benchmarks be skipped? [Y/n] : "
read SKIP_BENCHMARK

touch ${TEMP}

echo ""
for TEST in ${TESTS}; do
    CURRENT=$(echo "${TEST}" | grep -Po "tests/test_[0-9]+_\K[a-zA-Z]+")

    # Benchmarks
    if [[ ${CURRENT} == "benchmark" ]]; then
        if [[ ${SKIP_BENCHMARK} != "n" ]]; then
            continue
        fi
        rm ${TEMP}
        printf "\n\tRunning benchmark...\n"
        ${PROG} ${TEST}
        continue
    fi

    # Printing out test header
    printf "\ttesting %-12s [%2d/%2d]\t......\t" ${CURRENT} ${N} ${COUNT}
    
    # Run the command with a timeout
    OUTPUT=$(timeout 3s ${PROG} ${TEST} 2> ${TEMP})
    EXIT_STATUS=$?

    # Timeout Error
    if [ $EXIT_STATUS -eq 124 ]; then
        printf "\033[0;33mFAIL\033[0m\n"
        ((N++))
        continue
    fi

    # Parsing outputs for errors
    RESULT=$(grep -E "FAIL" <<< "$OUTPUT") # stdout
    RESULT+=$(cat ${TEMP} | grep -E "ERROR|Undefined|Error|Expect|dumped") # stderr

    if [[ -n ${RESULT} ]]; then
        # If RESULT exists, then it means there was an error
        if [[ ${CURRENT} == "let" || ${CURRENT} == "const" ]]; then
            # Ignore for "let" and "const" type tests. Add for more
            MSG=$(echo ${RESULT} | grep -E "FAIL")
            if [[ -n ${MSG} ]]; then
                printf "\033[0;33mFAIL\033[0m\n"
                continue
            fi
            printf "\033[0;32mPASS\033[0m\n"
            ((PASSED++))
        else
            printf "\033[0;33mFAIL\033[0m\n"
        fi
    else
        # If no RESULT, then pass
        printf "\033[0;32mPASS\033[0m\n"
        ((PASSED++))
    fi
    ((N++))
done

# Remove temp stderr redirects if exists
if [[ -e ${TEMP} ]]; then
    rm ${TEMP}
fi

echo -e "\nPassed ${PASSED}/${COUNT}"

if [[ ${PASSED} -eq ${COUNT} ]]; then
    echo "Version v${VERSION} ready to release!"
fi
