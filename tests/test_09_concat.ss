function main() {
    local const first = "Kh";
    local const second = "an";
    local var number = 3;

    var name = first + second + number;

    if ("Khan3" != name) {
        echo "[ FAIL ] test_9_concat.ss";
    }
}

main();
