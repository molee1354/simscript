class Coffee {
    public init(type) {
        this.type = type;
    }
    public brew() {
        echo "Making some " + this.type + "!";
    }
}

function main() {
    var inst = Coffee("Americano");
    inst.brew();
}

main();
