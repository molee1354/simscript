module mod1 = "test_10_class.ss";
module mod2 = "test_11_inherit.ss";

function main() {
    echo "from main";
    var coffee = mod1.Coffee("Latte");
    coffee.brew();

    var myPet = mod2.Dog("Marley");
    myPet.bark();
}

main();
