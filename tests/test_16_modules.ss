module mod1 = "test_10_class.ss";
module "test_11_inherit.ss";

function main() {
    var coffee = mod1.Coffee("Latte");
    coffee.brew();
}

main();
