function main() {
    var nums = [0, 1, 2, 3, 4, 5];
    const letters = ["a", "b", "c", "d", "e"];
    const nulls = [null, null, null];
    
    const matrix = [
        [1,2,3],
        [4,5,6],
        [7,8,9]
    ];
    echo matrix[1][1];

    echo nums;
    echo letters;
    echo nulls;
    nums[2]++;
    if (nums[2] != 3) {
        echo "[ FAIL ] test_17_array.ss";
    }

    if (letters[2] != "c") {
        echo "[ FAIL ] test_17_array.ss";
    }

    for (var i = 0; i < 5; i++) {
        echo "num -> " + nums[i];
        echo "letter -> " + letters[i];
    }

    echo "autovivification";
    nums[10] = "BAM!";
    echo nums;
}

main();
