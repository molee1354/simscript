class Animal {
    public init(type) {
        this.type = type;
    }
    public speak() {
        echo "I am a " + this.type;
    }
}

class Dog extends Animal {
    public init(name) {
        this.name = name;
        super.init("Dog");
    }
    public bark() {
        this.speak();
        echo "My name is " + this.name;
    }
}

function main() {
    local const myDog = Dog("Khan");
    myDog.bark();
}

main();
