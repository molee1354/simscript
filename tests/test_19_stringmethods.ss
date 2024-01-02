function main() {
    local var str = "string";
    if (6 != str.length() ) {
        echo "FAIL TEST_19_stringmethods.ss";
    }
    echo str[0];
    if ("s" != str[0])  {
        echo "FAIL TEST_19_stringmethods.ss";
    }
    // add string methods here
}

main();
