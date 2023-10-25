class Animal {
    init(type) {
        this.type = type;
    }
    speak() {
        echo "I am a " + this.type;
    }
}

class Dog extends Animal {
    init(name) {
        this.name = name;
        super.init("Dog");
    }
    bark() {
        this.speak();
        echo "My name is " + this.name;
    }
}

function main() {
    local const myDog = Dog("Khan");
    myDog.bark();
}

main();
