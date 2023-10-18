function for_loop() {
    for (var i = 0; i < 10;i++){
        if (i%2 == 0) {
            continue;
        }
        echo i;
    }
}

function while_loop() {
    var n = 0;
    while (n < 10) {
        if (n%2 == 0) {
            n++;
            continue;
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
