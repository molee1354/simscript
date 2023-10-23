CFILE="src/main.c"
README="README.md"
VER_GREPSTR="[0-9]+\.[0-9]+\.[[:alnum:]]*"

function set-version() {
    NEW_VERSION=$1
    CUR_VERSION=$(cat ${CFILE} | grep -oP "#define.*?VERSION.*?\"\K${VER_GREPSTR}")

    MAJ=$(echo ${NEW_VERSION} | grep -oP "^[0-9]+")
    MIN=$(echo ${NEW_VERSION} | grep -oP "^[0-9]+\.\K[0-9]+")
    PAT=$(echo ${NEW_VERSION} | grep -oP "^[0-9]+\.[0-9]+\.\K[[:alnum:]]*$")
    CAT_VERSION="${MAJ}${MIN}${PAT}"

    printf "Changing version from v%s to v%s [Y/n] : " ${CUR_VERSION} ${NEW_VERSION}
    read CONFIRM
    echo ""
    if [[ ${CONFIRM} != "n" ]]; then
        perl -pi -e "s/#define.*?VERSION.*?\"\K${VER_GREPSTR}/${NEW_VERSION}/g" ${CFILE}
        perl -pi -e "BEGIN { \$count = 0 } if (\$count < 3 && s/v\K${VER_GREPSTR}/${NEW_VERSION}/) { \$count++ }" ${README}
        perl -pi -e "s/simscript-v\K[[:alnum:]]*/${CAT_VERSION}/g" ${README}
    else
        return
    fi
}
function timestamp() {
    TIMESTAMP="\""
    TIMESTAMP+=$(date "+%b %d %Y, %H:%M")
    TIMESTAMP+="\""

    echo "Set timestamp to ${TIMESTAMP}"
    sed -i -E "s/\"[A-Z][a-z]+\s[0-9]{2}\s[0-9]{4},\s[0-9]{2}:[0-9]{2}\"/${TIMESTAMP}/g" src/main.c
}

function refresh() {
    make clean
    timestamp
    make windows
    make clean
    timestamp
    make
    yes | make test
}

function push() {
    CUR_VERSION=$(cat ${CFILE} | grep -oP "#define.*?VERSION.*?\"\K${VER_GREPSTR}")
    refresh

    git status
    echo -e "\nPush to remote? v${CUR_VERSION}"
    printf "(this will add/commit ALL your changes) [Y/n] "
    read CONFIRM
    echo ""
    if [[ ${CONFIRM} != "n" ]]; then
        git add .
        printf "Add a commit msg (nothing for default) "
        read COMMIT_MSG
        echo ""
        if [[ -n ${COMMIT_MSG} ]]; then
            git commit -m "${COMMIT_MSG}"
        else
            git commit -m "Simscript v${CUR_VERSION}"
        fi
        git push
    else
        return
    fi
}

function commit() {
    CFILE="src/main.c"
    CUR_VERSION=$(cat ${CFILE} | grep -oP "#define.*?VERSION.*?\"\K${VER_GREPSTR}")
    STATUS=$(git status | grep -o "nothing to commit")
    if [[ -n ${STATUS} ]]; then
        echo "Nothing to commit..."
        return
    fi
    git status
    printf "\nCommit all changes? [Y/n]" ${CUR_VERSION}
    read CONFIRM
    echo ""
    if [[ ${CONFIRM} != "n" ]]; then
        git add .
        echo "Add a commit msg below (nothing for default) "
        read COMMIT_MSG
        echo ""
        if [[ -n ${COMMIT_MSG} ]]; then
            git commit -m "${COMMIT_MSG}"
        else
            git commit -m "Simscript v${CUR_VERSION}"
        fi
    else
        return
    fi
}
