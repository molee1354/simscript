class Zoo {
  init() {
    this.aardvark = 1;
    this.baboon   = 1;
    this.cat      = 1;
    this.donkey   = 1;
    this.elephant = 1;
    this.fox      = 1;
  }
  ant()    { return this.aardvark; }
  banana() { return this.baboon; }
  tuna()   { return this.cat; }
  hay()    { return this.donkey; }
  grass()  { return this.elephant; }
  mouse()  { return this.fox; }
}

function test(iter) {
    local var sum = 0;
    local var zoo = Zoo();
    local var start = clock();
    while (sum < 100000000) {
        sum = sum + zoo.ant()
            + zoo.banana()
            + zoo.tuna()
            + zoo.hay()
            + zoo.grass()
            + zoo.mouse();
    }
    local var elapsed_time = clock() - start;
    echo "..." + iter + " : " + elapsed_time;
    return elapsed_time;
}

const iterations = 5;
var sum = 0;
var average;
for (var i=0; i < iterations; i = 1 + i) {
    sum = sum + test(i);
}
average = sum/iterations;
echo "Average Time : " + average;
