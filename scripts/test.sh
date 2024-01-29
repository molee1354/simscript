#!/bin/bash

PROG="./simscript"
VERSIONFILE="src/main.c"
VERSION=$(cat ${VERSIONFILE} | grep -oP "#define.*?VERSION.*?\"\K[0-9]\.[0-9]\.[[:alnum:]]*")
TESTS="./tests/*.ss"
TEMP="stderr.temp"

COUNT=$(ls ${TESTS} | wc -l)
((COUNT--))
PASSED=0
N=1

function _fail() {
    printf "\033[0;31mFAIL\033[0m\n"
}

function _pass() {
    printf "\033[0;32mPASS\033[0m\n"
}

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

# Prompting for memcheck
printf "Should memory leak checks be skipped?\n \
        (the skip benchmarks settings will be preserved) [Y/n] : "
read SKIP_MEMCHECK

touch ${TEMP}

echo ""
# code functionality test
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
    printf "\ttesting  %-14s [%2d/%2d]\t......\t" ${CURRENT} ${N} ${COUNT}
    
    # Run the command with a timeout
    OUTPUT=$(timeout 3s ${PROG} ${TEST} 2> ${TEMP})
    EXIT_STATUS=$?

    # Timeout Error
    if [ $EXIT_STATUS -eq 124 ]; then
        _fail
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
                _fail
                continue
            fi
            _pass
            ((PASSED++))
        else
            _fail
        fi
    else
        # If no RESULT, then pass
        _pass
        ((PASSED++))
    fi
    ((N++))
done

# memory leaks test
if [[ ${SKIP_MEMCHECK} == "n" ]]; then
    M=1
    CLEAR=0
    MTEMP="stderr.temp"
    touch ${MTEMP}
    echo ""
    for TEST in ${TESTS}; do
        CURRENT=$(echo "${TEST}" | grep -Po "tests/test_[0-9]+_\K[a-zA-Z]+")
        # Benchmarks
        if [[ ${CURRENT} == "benchmark" ]]; then
            if [[ ${SKIP_BENCHMARK} != "n" ]]; then
                continue
            fi
            rm ${MTEMP}
            printf "\n\tRunning memcheck on benchmark...\n"
            valgrind ${PROG} ${TEST}
            continue
        fi
        #
        # Printing out test header
        printf "\tmemcheck %-12s [%2d/%2d]\t......\t" ${CURRENT} ${M} ${COUNT}

        # Run the command with a timeout
        OUTPUT=$(timeout 3s valgrind ${PROG} ${TEST} 2> ${MTEMP})
        EXIT_STATUS=$?

        # Timeout Error
        if [ $EXIT_STATUS -eq 124 ]; then
            _fail
            ((M++))
            continue
        fi

        # Parsing outputs for errors
        RESULT=$(cat ${MTEMP} | grep -E "All heap blocks were freed") # stderr
        if [[ -n ${RESULT} ]]; then
            _pass
            ((CLEAR++))
        else
            if [[ ${CURRENT} == "let" || ${CURRENT} == "const" ]]; then
                _pass
                ((CLEAR++))
            else
                _fail
            fi
        fi
        ((M++))
    done
fi
# Remove temp stderr redirects if exists
if [[ -e ${TEMP} ]]; then
    rm ${TEMP}
fi
if [[ -e ${MTEMP} ]]; then
    rm ${MTEMP}
fi

if [[ ${SKIP_MEMCHECK} == "n" ]]; then
    echo -e "\nPassed ${PASSED}/${COUNT} (${CLEAR}/${COUNT} leak check)"
else
    echo -e "\nPassed ${PASSED}/${COUNT}"
fi

if [[ ${PASSED} -eq ${COUNT} ]]; then
    echo -e "\nVersion v${VERSION} ready to release!"
fi
