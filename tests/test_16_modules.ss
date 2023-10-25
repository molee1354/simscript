module mod1 = "test_10_class.ss";
module mod2 = "test_11_inherit.ss";

function main() {
    echo "from main";
    local var coffee = mod1.Coffee("Latte");
    coffee.brew();

    local var myPet = mod2.Dog("Marley");
    myPet.bark();
}

main();
