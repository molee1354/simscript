class TestClass {
    public init(message) {
        echo "Test constructor";
        this.message = message;
        this.say_private_message();
    }

    public say_message() {
        echo "Message is: " + this.message;
    }

    private say_private_message() {
        echo "Private message";
    }
}

function main() {
    local var obj = TestClass("Hello Khan!");
    obj.say_message();
}

main();
