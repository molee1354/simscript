function main() {
    var one = 1;
    function inner() {
        one++;
        if (one != 2) {
            echo "[ FAIL ] test_2_var.ss";
        }
    }
    inner();
}

main();
