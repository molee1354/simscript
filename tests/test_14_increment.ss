class Food {
    init(number) {
        this.number = number;
    }
    get_num() {
        return this.number;
    }
}

function main() {
    var food1 = Food(0);
    food1.number += 1;
    food1.number++;
    echo food1.number;

    if (food1.number != 2) {
        echo "[ FAIL ] test_14_increment.ss";
    }

    var food2 = Food(0);
    food2.number -= 1;
    food2.number--;
    echo food2.number;
    if (food2.number != -2) {
        echo "[ FAIL ] test_14_increment.ss";
    }
        
    var food3 = Food(1);
    food3.number *= 3;
    echo food3.number;
    if (food3.number != 3) {
        echo "[ FAIL ] test_14_increment.ss";
    }

    var food4 = Food(9);
    food4.number /= 3;
    echo food4.number;
    if (food4.number != 3) {
        echo "[ FAIL ] test_14_increment.ss";
    }
}

main();
