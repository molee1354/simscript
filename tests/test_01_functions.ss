function test_function(arg1, arg2) {
    echo "This is a test function.";
    echo "Arg 1 is : " + arg1;
    echo "Arg 2 is : " + arg2;
    return arg1 + arg2;
}

function main() {
    var arg1 = 1;
    var arg2 = 3;
    echo "Result is : " + test_function(arg1, arg2);
}

main();
