function outer() {
    var x = 0;

    function middle() {
        function inner() {
            x++;
            if (x != 1) {
                echo "[ FAIL ] test_8_closure.ss";
                return 1;
            }
            return x;
        }
        return inner;
    }
    return middle;
}

function main() {
    var mid = outer();
    var in = mid();
    var val = in();

    if (val != 1) {
        echo "[ FAIL ] test_8_closure.ss";
    }
}

main();
