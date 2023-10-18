function main() {
    let one = 1;
    function inner() {
        // this access can't happen
        one++;
        if (one == 2) {
            echo "[ FAIL ] test_2_let.ss";
        }
    }
    inner();
}

main();
