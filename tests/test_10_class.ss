class Coffee {
    init(type) {
        this.type = type;
    }
    brew() {
        echo "Making some " + this.type + "!";
    }
}

function main() {
    var inst = Coffee("Americano");
    inst.brew();
}

main();
