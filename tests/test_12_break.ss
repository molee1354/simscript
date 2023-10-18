function for_loop() {
    var n = 0;
    for (var i = 0;;i++){
        if (n >= 10) {
            break;
        }
        echo n;
        n++;
    }
}

function while_loop() {
    var n = 0;
    while (true) {
        if (n >= 10)  {
            break;
        }
        echo n;
        n++;
    }
}

function main() {
    for_loop();
    while_loop();
}

main();
