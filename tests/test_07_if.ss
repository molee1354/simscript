function check() {
    const var one = 1;
    const var zero = 0;
    const var str = "Khan";

    if ( one > zero ) {
        return 0;
    } else if ( one >= zero) {
        return 0;
    } else if ( zero < one ) {
        return 0;
    } else if ( zero <= one) {
        return 0;
    } else if ( zero == 0 ) {
        return 0;
    } else if ( one == 1) {
        return 0;
    } else if ( true ) {
        return 0;
    } else if ( str == "Khan") {
        return 0;
    } else {
        return 1;
    }
}

function main() {
    if (check() == 1) {
        echo "[ FAIL ] test_7_if.ss";
    }
}

main();
