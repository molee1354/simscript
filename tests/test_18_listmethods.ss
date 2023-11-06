function main() {
    local var ll = [1,2];
    ll.append(3);
    echo ll;
    if (ll != [1,2,3]) {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.prepend(0);
    echo ll;
    if (ll != [0,1,2,3]) {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.insert(1,"BAM");
    echo ll;
    if (ll != [0, "BAM", 1, 2, 3]) {
        echo "FAIL test_18_listmethods.ss";
    }
    var targ = "BAM";
    echo targ + " is at " + ll.find(targ);
    if (ll.find(targ) != 1 ) {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.delete(1);
    echo ll;
    if (ll[1] == "BAM") {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.reverse();
    echo ll;
    if (ll != [3,2,1,0]) {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.push(4);
    echo ll;
    if (ll != [4,3,2,1,0]) {
        echo "FAIL test_18_listmethods.ss";
    }
    ll.enqueue(5);
    echo ll;
    if (ll != [5,4,3,2,1,0]) {
        echo "FAIL test_18_listmethods.ss";
    }
    echo "dequeue() " + ll.dequeue();
    echo ll;
    if (ll != [5,4,3,2,1]) {
        echo "FAIL test_18_listmethods.ss";
    }
    echo "pop() " + ll.pop();
    echo ll;
    if (ll != [4,3,2,1]) {
        echo "FAIL test_18_listmethods.ss";
    }
}

main();
