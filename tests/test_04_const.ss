function main() {
    local const one = 1;
    one = 2;
    // should never reach here
    echo "[ FAIL ] test_4_const.ss";
}

main();
