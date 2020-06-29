#include "mbed.h"
#include "bbcar.h"

Ticker servo_ticker;
PwmOut pin9(D9), pin8(D8);
DigitalInOut pin10(D10);
BBCar car(pin9, pin8, servo_ticker);
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11); // D11:rx(black), D12:tx(orange)
//Serial pc(USBTX,USBRX); //tx,rx
Serial uart(D1,D0); //tx,rx

DigitalOut red(LED1);
DigitalOut green(LED2);
DigitalOut blue(LED3);
// led table:
// red = turn left
// green = turn right
// yellow = go straight
// white = mission done

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
Thread thread1;

//void GetAccData(Arguments *in, Reply *out);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
void GetData(void);
void goPing(int dis);
void backPing(int dis);
void turnLeft(int dir);
void turnRight(int dir);

void TurnRight1();
void TurnRight();
void TurnLeft();
void TurnLeft1();

void mission2(void);
void send_thread();
void recieve_thread(){
   while(1) {
      if(uart.readable()){
            char recv = uart.getc();
            pc.putc(recv);
            pc.printf("\r\n");
      }
   }
}

int op;

int main() {

    int i = 0;

    pc.baud(9600);
    char xbee_reply[4];
    
    red = 1; blue = 1; green = 1;
    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x240\r\n");
    reply_messange(xbee_reply, "setting MY : 240");

    xbee.printf("ATDL 0x140\r\n");
    reply_messange(xbee_reply, "setting DL : 140");

    xbee.printf("ATID 0x1\r\n");
    reply_messange(xbee_reply, "setting PAN ID : 0x1");

    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");

    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");

    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");

    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();

    // start setup queue and send back info to Xbee
    pc.printf("start\r\n");
    red = 1; blue = 0; green = 1;
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    op = 5;
    queue.call_every(1000, GetData);

    // setup camera
    uart.baud(9600);
    thread1.start(recieve_thread);
    red = 1; blue = 1; green = 1;

    // start for BBcar
    parallax_ping  ping1(pin10);

    // Step 1: go to Mission 1

    goPing(15);          // go stright to wall
    //turnLeft(1);        // turn left
    turnRight(-1);
    goPing(50);         // go straight till 50 cm from wall

    // Step 2: Mission 1
    
    turnLeft(-1);       // start back to garage
    wait(0.5);
    backPing(55);       // backward
    goPing(40);         // go straight till 40 cm from wall
    turnLeft(1);        // turn left
    goPing(15);         // go straight
    turnRight(1);       // turn right
    // ready to take picture & classification
    send_thread();      // mission 1 ()
    goPing(10);         // go straight to leave the mission
    turnRight(1);       // turn right
    goPing(40);         // go straight
    turnRight(1);       // turn right
    goPing(10);         // go straight

    // Step 3: Mission 2

    turnRight(1);       // turn right to go to mission 2 area
    goPing(90);         // go straight
    turnRight(1);       // turn right
    goPing(10);         // go straight
    mission2();         // mission 2
    backPing(60);       // backward to leave mission 2
    turnRight(-1);      // left wheel back
    goPing(10);         // go to top
    turnRight(1);       // turn right
    car.goStraight(100);         // leave the maze
    wait(5);
    car.stop();
    //  turn function
    //  factor = 1 or 0
    //  it will go straight
    /*
    -----
    car.turn(150, 0.03); // right forward
    wait(1.5); // about # 1/4 loop
    car.stop();

    car.turn(37, 0.1);
    // wait_ms(2800);
    wait_ms(2800);
    -----
    car.turn(-100, 0.1); // right backward
    wait(1.512); // about #  1/4 loop
    car.stop();
    wait(3);

    car.turn(-31.5, 0.1);
    // wait_ms(2650); 
    wait_ms(2500); 
    car.stop();
    ----
    
    car.turn(135, -0.1); // left forward
    wait(1.55); // about #  1/4 loop
    car.stop();
    wait(3);

    car.turn(31.5, -0.1);
    wait_ms(2900);  // 2750
    car.stop();

    ------
    car.turn(-100, -0.05); // left backward
    wait(1.501); // about #   loop
    car.stop();
    wait(3);

    car.turn(-37, -0.1);
    // wait_ms(2800);
    wait_ms(2850);
    -------
    */
}

void GetData(void)
{
  char c[3];

  // output operation data
  pc.printf("%01d ", op);
  sprintf(c, "%01d", op);
  xbee.printf("%s", c);
}

void goPing(int dis)
{
    parallax_ping  ping1(pin10);

    car.goStraight(100);
    op = 1;
    red = 0; green = 0; blue = 1;
    while(1){
        if((float)ping1 < dis) {
            car.stop();
            op = 5;
            red = 1; green = 1; blue = 1;
            break;
        }
        wait(.01);
    }
    wait(0.5);
}

void backPing(int dis)
{
    parallax_ping  ping1(pin10);

    car.goStraight(-100);
    op = 2;
    red = 0; green = 0; blue = 1;
    while(1){
        if((float)ping1 > dis) {
            car.stop();
            op = 5;
            red = 1; green = 1; blue = 1;
            break;
        }
        wait(.01);
    }
    wait(0.5);
}

void turnLeft(int dir)
{
    red = 0; green = 1; blue = 1;
    op = 4;

    if (dir == 1) {
        //car.turn(150, 0.03); // right forward
        //wait(0.5); // about # 1/4 loop
        car.turn(37, 0.1);
        wait_ms(2800);
    }
    else {
        //car.turn(-100, 0.1); // right backward
        //wait(1.512); // about #  1/4 loop
        car.turn(-31.5, 0.1);
        wait_ms(2650); 
    }
    car.stop();
    red = 1; green = 1; blue = 1;
    op = 5;
    wait(0.3);
}

void turnRight(int dir)
{
    red = 1; green = 0; blue = 1;
    op = 3;

    if (dir == 1) {
        //car.turn(135, -0.1); // left forward
        //wait(1.7); // about #  1/4 loop
        car.turn(31.5, -0.1);
        wait_ms(2900);  // 2750
    }
    else {
        //car.turn(-100, -0.05); // left backward
        //wait(1.8); // about #   loop
        car.turn(-37, -0.1);
        wait_ms(2800);
    }
    car.stop();
    red = 1; green = 1; blue = 1;
    op = 5;
    wait(0.3);
}

void send_thread()
{
    red = 1; green = 1; blue = 0;
    op = 6;
    char s[21];
    sprintf(s,"image_classification");
    uart.puts(s);
    //pc.printf("send\r\n");
    wait(0.5);
    red = 1; green = 1; blue = 1;
    op = 5;
}

void mission2(void)
{
    parallax_ping  ping1(pin10);
    int i;

    op = 7;
    red = 1; green = 1; blue = 0;
    car.turn(135, -0.1); // left forward
    wait(0.75); // about #  1/4 loop
    car.stop();
    for (i=0; i<10; i++) {
        car.turn(-150, -0.1); // left backward
        wait(0.075); // about #   loop
        printf("%g ", (float)ping1);
    }
    for (i=0; i<10; i++) {
        car.turn(150, 0.1); // right forward
        wait(0.075); // about # 1/4 loop
        printf("%g ", (float)ping1);
    }
    red = 1; green = 1; blue = 1;
    op = 5;
}

void reply_messange(char *xbee_reply, char *messange){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
    pc.printf("%s\r\n", messange);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';

  }
}

void check_addr(char *xbee_reply, char *messenger){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  xbee_reply[3] = xbee.getc();
  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
  xbee_reply[0] = '\0';
  xbee_reply[1] = '\0';
  xbee_reply[2] = '\0';
  xbee_reply[3] = '\0';
}

void TurnLeft()
{
  // heads
  green = 0;
  car.turn(37, 0.1);
  // wait_ms(2800);
  wait_ms(2800);
  car.stop();
  green = 1;
}
void TurnLeft1()
{
  // heads
  green = 0;
  car.turn(-37, -0.1);
  // wait_ms(2800);
  wait_ms(2850);
  car.stop();
  green = 1;

}

void TurnRight()
{
  // heads 
  green = 0;
  car.turn(-31.5, 0.1);
  // wait_ms(2650); 
  wait_ms(2500); 
  car.stop();
  green = 1;
}
void TurnRight1()
{
  // heads 
  green = 0;
  car.turn(31.5, -0.1);
  wait_ms(2900);  // 2750
  car.stop();
  green = 1;
}