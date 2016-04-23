// L3D 16^3 Cube UDP Listener script, hacked onto an older version of the Cube 16^3 Demo code

#include <math.h>
#include "application.h"

// If we lose connection for some reason, like a crappy router or packet issue, reset
bool hasReceivedData = false;
int _timestamp = 0;
int _timeoutInSeconds = 5;

 // Streaming stuff - the streaming magic happens around line 656, in the loop
// Switch listening mode to off to go back to demo mode
// You can find the IP address and port of the Particle by getting the variables IPAddress and port via HTTP
char data[4096];
UDP Udp;
bool listeningMode = true;
char localIP[24];
char macAddress[20];
int port;
int lastUpdated=0;
int packetCounter = 0;
int setPort(String _port);
#define PACKET_SIZE 4096


// Important init code

#define PIXEL_PIN D0
#define totalPIXEL 4096
#define stripPIXEL totalPIXEL/8    //THERE ARE FOUR PINS TO DRIVE EACH STRIPS
#define PIXEL_TYPE WS2812B
#define SIZE 16

// Demo code

#define PI 3.14159
 
#define DEMO_ROUTINES 5
#define FIREWORKS 0
#define FASTPLASMA 1
#define ROMAN_CANDLE 2
#define FFT_JOY 3
#define CIRCLES 4
 
#define PLASMA 5
#define DISPLAY_TEST 6
 
 
 
/**********************************
 * flip variables *
 * ********************************/
 //accelerometer pinout
#define X A0
#define Y A1
#define Z A6
#define NUM_SAMPLES 100
int accelerometer[3];
unsigned long totals[3];
int runningAverage[3];
boolean whack[3];
boolean whacked=false;
#define WHACK_X 20
#define WHACK_Y 20
#define WHACK_Z 20
 
bool autoCycle=true;    //start on autocycle by default
 
/*******************************
 * fade variables *
 * ****************************/
bool fading=false;
int fadeValue=255;
int fadeSpeed=2;
 
/*  datatype definitions
*/
/**   An RGB color. */
struct Color
{
  unsigned char red, green, blue;
 
  Color(int r, int g, int b) : red(r), green(g), blue(b) {}
  Color() : red(0), green(0), blue(0) {}
};
 
/********************************
 * zplasma variables *
 * *****************************/
float phase = 0.0;
long fastPhase=0;
float phaseIncrement = 0.035; // Controls the speed of the moving points. Higher == faster
int fastPhaseIncrement = 1; // Controls the speed of the moving points. Higher == faster
int fastColorStretch = 4; // will divide rather than multiply
float colorStretch = 0.23; // Higher numbers will produce tighter color bands
float plasmaBrightness = 0.2;
Color plasmaColor;
 
 
 
/**   A point in 3D space.  */
struct Point
{
  float x;
  float y;
  float z;
  Point() : x(0), y(0), z(0) {}
  Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};
 
/**   An integer point in 3D space.  */
struct Point_i
{
  int x;
  int y;
  int z;
  Point_i() : x(0), y(0), z(0) {}
  Point_i(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};
 
struct Rocket
{
        float x,y,z;
        float xVel, yVel, zVel;
        float gravity;
        Color col;
        Rocket():x((SIZE-1)/2), y(0), z((SIZE-1)/2), col(Color(255,0,0)){}
};
 
 
#define NUM_ROCKETS 50
Rocket rockets[NUM_ROCKETS];
float offset=0;
 
int y=0;
int yinc=1;
int maxBrightness=50;
 
int demo=FIREWORKS;
 
#define MICROPHONE A7
#define GAIN_CONTROL D5
 
/*********************************
 * FFTJoy variables *
 * *******************************/
#define M 5
float real[(int)pow(2,M)];
float imaginary[(int)pow(2,M)];
float maxValue=0;
float sample;
 
 
/******************************
 * fireworks variables *
 * ****************************/
int centerX, centerY, centerZ;
int launchX, launchZ;
int red, green, blue;
float radius=0;
float speed;
bool showRocket;
bool exploded;
bool dead;
float xInc, yInc, zInc;
float rocketX, rocketY, rocketZ;
float launchTime;
int maxSize;
int fireworkRes;
Color rocketColor, fireworkColor;
 
uint8_t PIXEL_RGB[totalPIXEL*3];  //EACH PIXELS HAS 3 BYTES DATA
 
// function prototypes
int displayTestIteration=0;
 
void displayTest();
 
void initFireworks();
void initCircles();
void initRockets();
void initAccelerometer();
void setVoxel(int x, int y, int z, const Color& col);

void setVoxel(const Point& p, const Color& col);
Color getVoxel(int x, int y, int z);
Color getVoxel(const Point& p);
void line(const Point& p1, const Point& p2, const Color& col);
void line(int x1, int y1, int z1, int x2, int y2, int z2, const Color& col);
void sphere(const Point& center, float radius, const Color& col);
void sphere(const Point& center, float radius, const Color& col, int res);
void sphere(float x, float y, float z, float radius, const Color& col);
void sphere(float x, float y, float z, float radius, const Color& col, int res);
void background(const Color& col);
Color colorMap(float val, float minVal, float maxVal);
Color lerpColor(const Color& a, const Color& b, int val, int minVal, int maxVal);
Point add(const Point& a, const Point& b);
void fft_joy();
short FFT(short int dir,int m,float *x,float *y);
void fade();
void mirror();
void updateFireworks();
void circles();
void show();
void setFadeSpeed();
void prepRocket();
float distance(float x, float y, float z, float x1, float y1, float z1);
 
 
int frame;
 
Point poly1[4];
float poly1Angles[4];
float poly1AnglesInc[4];
 
Point poly2[4];
float poly2Angles[4];
float poly2AnglesInc[4];
 
Point poly3[4];
float poly3Angles[4];
float poly3AnglesInc[4];
 
float poly1Color, poly2Color, poly3Color;
float poly1ColorInc=0.001, poly2ColorInc=0.0005, poly3ColorInc=0.0007;
 
Point center;
unsigned int lastDemo=0;
#define DEMO_TIME 30000
int timeout=0;
 
bool onlinePressed=false;
bool lastOnline=true;
#define BUTTON D6 //press this button to connect to the internet
#define MODE D4
 
void setup()
{
 _timestamp = Time.now() + 5;
  
    randomSeed(analogRead(A0));
          pinMode(GAIN_CONTROL, OUTPUT);
          digitalWrite(GAIN_CONTROL, LOW);
    initFireworks();
    initCircles();
    initRockets();
    initAccelerometer();
    pinMode(D0,OUTPUT);  //PB7
    pinMode(D1,OUTPUT); //PB6
    pinMode(D2,OUTPUT);  //BP5
    pinMode(D3,OUTPUT);  //PB4
    pinMode(A2,OUTPUT);  //PA4
    pinMode(A3,OUTPUT); //PA5
    pinMode(A4,OUTPUT);  //BA6
    pinMode(A5,OUTPUT);  //PA7
 
 Udp.setBuffer(4096);
    port=2222;
  Udp.begin (port);
  Serial.begin(115200);
  Serial.println("initializing...");

  
 
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
    
      updateNetworkInfo();
  initSparkVariables();
  
        background(Color(255,255,255));

       background(Color(0,0,0));

    
}

//initializes the shared variables and functions that are accessible through the spark API
//this makes the core's local IP address, MAC address and port accessible to streaming programs
//that have the access token for the core owner's account
//The function setPort lets a streaming program set the port on which the core will listen for streaming packets
void initSparkVariables()
{
    Particle.variable("IPAddress", localIP, STRING);
    Particle.variable("MACAddress", macAddress, STRING);
    Particle.variable("port", &port, INT);
    Particle.function("setPort", setPort);
}

//updates the local IP address and mac address and stores them in global strings
//if those strings are Spark variables (using the Spark.variable() function), they'll be accessible to the wider world
//This is all kinds of helpful anytime you're working on a project that uses network communicqtion (streaming, etc)

void updateNetworkInfo()
{
    IPAddress myIp = WiFi.localIP();
    sprintf(localIP, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
    byte macAddr[6];
    WiFi.macAddress(macAddr);
    sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x",macAddr[5],macAddr[4],macAddr[3],macAddr[2],macAddr[1],macAddr[0]);

    //print it out the serial port
    Serial.print("local IP Address: ");
    Serial.println(localIP);
    Serial.print("MAC address:  ");
    Serial.println(macAddress);
}

int setPort(String _port)
{
    port=_port.toInt();
    Udp.begin(port);
    return port;
}
 
//initializes the running average values for the accelerometer
//I just set them to the first reading of the accelerometer on boot -- this is an imperfect method,
//but it gets the rolling average very close it its eventual value
//I chose to base it off of each cube's individual ADC reading, rather than hardcode the values from my sample cube
void initAccelerometer()
{
        runningAverage[0]=analogRead(X);
        runningAverage[1]=analogRead(Y);
        runningAverage[2]=analogRead(Z);
}
 
 
/********************************
 * zplasma functions *
 * *****************************/
 
void zPlasma()
{
 
    phase += phaseIncrement;
    // The two points move along Lissajious curves, see: http://en.wikipedia.org/wiki/Lissajous_curve
    // We want values that fit the LED grid: x values between 0..15, y values between 0..15, z values between 0..15
    // The sin() function returns values in the range of -1.0..1.0, so scale these to our desired ranges.
    // The phase value is multiplied by various constants; I chose these semi-randomly, to produce a nice motion.
    Point p1 = { (sin(phase*1.000)+1.0) * 8.0, (sin(phase*1.310)+1.0) * 8.0,  (sin(phase*1.380)+1.0) * 8.0};
    Point p2 = { (sin(phase*1.770)+1.0) * 8.0, (sin(phase*2.865)+1.0) * 8.0,  (sin(phase*1.410)+1.0) * 8.0};
    Point p3 = { (sin(phase*0.250)+1.0) * 8.0, (sin(phase*0.750)+1.0) * 8.0,  (sin(phase*0.380)+1.0) * 8.0};
 
    byte row, col, dep;
 
    // For each row
    for(row=0; row<16; row++) {
        float row_f = float(row); // Optimization: Keep a floating point value of the row number, instead of recasting it repeatedly.
 
        // For each column
        for(col=0; col<16; col++) {
            float col_f = float(col); // Optimization.
 
            // For each depth
            for(dep=0; dep<16; dep++) {
                float dep_f = float(dep); // Optimization.
 
                // Calculate the distance between this LED, and p1.
                Point dist1 = { col_f - p1.x, row_f - p1.y,  dep_f - p1.z }; // The vector from p1 to this LED.
                float distance1 = sqrt( dist1.x*dist1.x + dist1.y*dist1.y + dist1.z*dist1.z);
 
                // Calculate the distance between this LED, and p2.
                Point dist2 = { col_f - p2.x, row_f - p2.y,  dep_f - p2.z}; // The vector from p2 to this LED.
                float distance2 = sqrt( dist2.x*dist2.x + dist2.y*dist2.y + dist2.z*dist2.z);
 
                // Calculate the distance between this LED, and p3.
                Point dist3 = { col_f - p3.x, row_f - p3.y,  dep_f - p3.z}; // The vector from p3 to this LED.
                float distance3 = sqrt( dist3.x*dist3.x + dist3.y*dist3.y + dist3.z*dist3.z);
 
                // Warp the distance with a sin() function. As the distance value increases, the LEDs will get light,dark,light,dark,etc...
                // You can use a cos() for slightly different shading, or experiment with other functions.
                float color_1 = distance1; // range: 0.0...1.0
                float color_2 = distance2;
                float color_3 = distance3;
                float color_4 = (sin( distance1 * distance2 * colorStretch )) + 2.0 * 0.5;
                // Square the color_f value to weight it towards 0. The image will be darker and have higher contrast.
                color_1 *= color_1 * color_4;
                color_2 *= color_2 * color_4;
                color_3 *= color_3 * color_4;
                color_4 *= color_4;
                // Scale the color up to 0..7 . Max brightness is 7.
                //strip.setPixelColor(col + (8 * row), strip.Color(color_4, 0, 0) );
                plasmaColor.red=color_1*plasmaBrightness;
                plasmaColor.green=color_2*plasmaBrightness;
                plasmaColor.blue=color_3*plasmaBrightness;
 
                setVoxel(row,col,dep,plasmaColor);
            }
        }
    }
}
 
// all inputs need to be multiplied by 572
inline int ifsin( int x )
{
  // convert float to integer (32768 units times 2*pi / 360 degrees )
  //int x= angle * 572;
 
  static const int qN = 13, qA= 12, qP= 15, qR= 2*qN-qP, qS= qN+qP+1-qA;
  //x *= 166886; // -- conversion to radians
  x= x<<(30-qN); // 32768 steps per circle (360degrees, 2pi radians)
  // shift to full s32 range (Q13->Q30)
  if( (x^(x<<1)) < 0)     // test for quadrant 1 or 2
  x= (1<<31) - x;
  x= x>>(30-qN);
 
  // convert result to -1024..+1024
  return (x * ( (3<<qP) - (x*x>>qR) ) >> qS) >> 2;
}
 
/********************************
 * fastPlasma functions *
 * *****************************/
 
inline unsigned int isqrt(unsigned long a) {
     unsigned int rem = 0;
     int root = 0;
     int i;
 
     for (i = 0; i < 16; i++) {
         root <<= 1;
         rem <<= 2;
         rem += a >> 30;
         a <<= 2;
 
         if (root < rem) {
             root++;
             rem -= root;
             root++;
         }
     }
 
     return (unsigned short) (root >> 1);
 }
 
void fastPlasma()
{
 
    fastPhase += fastPhaseIncrement;
    // The two points move along Lissajious curves, see: http://en.wikipedia.org/wiki/Lissajous_curve
    // We want values that fit the LED grid: x values between 0..15, y values between 0..15, z values between 0..15
    // The sin() function returns values in the range of -1.0..1.0, so scale these to our desired ranges.
    // The phase value is multiplied by various constants; I chose these semi-randomly, to produce a nice motion.
    Point_i p1 = { (ifsin(fastPhase*572)+1024)/128, (ifsin(fastPhase*749)+1024) /128,  (ifsin(fastPhase*789)+1024) /128};
    Point_i p2 = { (ifsin(fastPhase*1144)+1024)/128, (ifsin(fastPhase*1639)+1024) /128,  (ifsin(fastPhase*807)+1024) /128};
    Point_i p3 = { (ifsin(fastPhase*143)+1024)/128, (ifsin(fastPhase*429)+1024) /128,  (ifsin(fastPhase*217)+1024) /128};
 
    byte row, col, dep;
 
    for(row=0; row<16; row++) {
        for(col=0; col<16; col++) {
            for(dep=0; dep<16; dep++) {
 
                // Calculate the distance between this LED, and p1.
                Point_i dist1 = { col - p1.x, row - p1.y,  dep - p1.z }; // The vector from p1 to this LED.
                int distance1 = isqrt( dist1.x*dist1.x + dist1.y*dist1.y + dist1.z*dist1.z);
 
                // Calculate the distance between this LED, and p2.
                Point_i dist2 = { col - p2.x, row - p2.y,  dep - p2.z}; // The vector from p2 to this LED.
                int distance2 = isqrt( dist2.x*dist2.x + dist2.y*dist2.y + dist2.z*dist2.z);
 
                // Calculate the distance between this LED, and p3.
                Point_i dist3 = { col - p3.x, row - p3.y,  dep - p3.z}; // The vector from p3 to this LED.
                int distance3 = isqrt( dist3.x*dist3.x + dist3.y*dist3.y + dist3.z*dist3.z);
 
                // Warp the distance with a sin() function. As the distance value increases, the LEDs will get light,dark,light,dark,etc...
                // You can use a cos() for slightly different shading, or experiment with other functions.
                int color_1  = distance1; // range: 0...16
                int color_2 = distance2;
                int color_3 = distance3;
                int color_4 = (ifsin( distance1 * distance2 * 143 ) + 1024) /128;
                // Square the color_f value to weigh it towards 0. The image will be darker and have higher contrast.
                color_1 *= color_1 * color_4;
                color_2 *= color_2 * color_4;
                color_3 *= color_3 * color_4;
 
                // Scale the color up to 0..7 . Max brightness is 7.
                //strip.setPixelColor(col + (8 * row), strip.Color(color_4, 0, 0) );
                plasmaColor.red=color_1/290;
                plasmaColor.green=color_2/290;
                plasmaColor.blue=color_3/290;
 
                setVoxel(row,col,dep,plasmaColor);
            }
        }
    }
}
 
 
 
void displayTest()
{
  displayTestIteration++;
  Color testColor;
  testColor.red=displayTestIteration%7;
  testColor.green=(displayTestIteration+2)%7;
  testColor.blue=(displayTestIteration+4)%7;
 
  for(int x=0; x<16; x++)
    for(int y=0; y<16; y++)
      for(int z=0; z<16; z++)
        setVoxel(x,y,z,testColor);
 
  delay(50);
}
 
 
 
void updateAccelerometer()
{
        accelerometer[0]=analogRead(X);
        accelerometer[1]=analogRead(Y);
        accelerometer[2]=analogRead(Z);
        for(int i=0;i<3;i++)
        {
                totals[i]+=accelerometer[i];
                //sweet running average algorithm:  average[i+1]=average[i]+(sample[i]-average[i])/NUM_SAMPLES
                //I average over 100 samples, or ~2.5 seconds
                runningAverage[i]=runningAverage[i]+((accelerometer[i]-runningAverage[i])/NUM_SAMPLES);
                whack[i]=false;
        }
        if(abs(accelerometer[0]-runningAverage[0])>WHACK_X)
                whack[0]=true;
        if(abs(accelerometer[1]-runningAverage[1])>WHACK_Y)
                whack[1]=true;
        if(abs(accelerometer[2]-runningAverage[2])>WHACK_Z)
                whack[2]=true;
        whacked=whack[0] | whack[1] | whack[2];
}
 
void initRockets()
{
    for(int i=0;i<NUM_ROCKETS;i++)
    {
                rockets[i].gravity=-0.01;
                rockets[i].y=0;
                rockets[i].x=center.x;
                rockets[i].z=center.z;
                rockets[i].col=Color(255,0,0);
                rockets[i].xVel=-.5;//(random(10)/10)-0.5;
                rockets[i].yVel=0.25;//random(10)/10;
                rockets[i].zVel=0.25;//(random(10)/10)-0.5;
     }
}
 
void initCircles()
{
    center=Point((SIZE-1)/2,(SIZE-1)/2,(SIZE-1)/2);
    for(int i=0;i<4;i++)
    {
        poly1Angles[i]=random(6.28);
        poly2Angles[i]=random(6.28);
        poly3Angles[i]=random(6.28);
    }
    poly1AnglesInc[0]=random(10)/100 - 0.05;
    poly1AnglesInc[1]=random(20)/100 - 0.1;
    poly1AnglesInc[2]=random(10)/100 - 0.05;
    poly1AnglesInc[3]=random(20)/100 - 0.1;
 
    poly2AnglesInc[0]=random(20)/100 - 0.1;
    poly2AnglesInc[1]=random(20)/100 - 0.1;
    poly2AnglesInc[2]=random(20)/100 - 0.1;
    poly2AnglesInc[3]=random(20)/100 - 0.1;
 
    poly3AnglesInc[0]=random(20)/100 - 0.1;
    poly3AnglesInc[1]=random(20)/100 - 0.1;
    poly3AnglesInc[2]=random(20)/100 - 0.1;
    poly3AnglesInc[3]=random(20)/100 - 0.1;
}
 
 
void romanCandle()
{
  background(Color(0,0,0));
    for(int i=0;i<pow(2,M);i++)
    {
        real[i]=analogRead(MICROPHONE)-993;  //adapted for the 0.8v bias point of the big cube
        delayMicroseconds(212);
        imaginary[i]=0;
    }
    FFT(1, M, real, imaginary);
    for(int i=0;i<pow(2,M);i++)
    {
        imaginary[i]=sqrt(pow(imaginary[i],2)+pow(real[i],2));
        if(imaginary[i]>maxValue)
            maxValue=imaginary[i];
    }
    if(maxValue>100)
        maxValue--;
    int rocketsToFire=0;
    for(int i=0;i<pow(2,M)/2;i++)
    {
        imaginary[i]=SIZE*imaginary[i]/maxValue;
        if(imaginary[i]>SIZE/2)
                rocketsToFire++;
    }
 
            for(int i=0;i<NUM_ROCKETS;i++)
            {
                rockets[i].yVel+=rockets[i].gravity;
                rockets[i].x+=rockets[i].xVel;
                rockets[i].y+=rockets[i].yVel;
                rockets[i].z+=rockets[i].zVel;
                if(rockets[i].col.green>5)
                        rockets[i].col.green-=5;
                if(rockets[i].col.blue>5)
                        rockets[i].col.blue-=5;
                /*
                Serial.print(rockets[i].col.red);
                Serial.print(" ");
                Serial.print(rockets[i].col.green);
                Serial.print(" ");
                Serial.println(rockets[i].col.blue);
                setVoxel(rockets[i].x,rockets[i].y,rockets[i].z,rockets[i].col);
                */
 
                if(rocketsToFire>0)
                if(rockets[i].y<0)
                {
                rocketsToFire--;
                rockets[i].gravity=-0.01;
                rockets[i].y=0;
                rockets[i].x=center.x;
                rockets[i].z=center.z;
                rockets[i].col=Color(random(100),random(100),random(100));
                rockets[i].xVel=(float)random(10)/10;//((float)random(10)/10)-0.5;
                rockets[i].yVel=(float)random(10)/10;
                rockets[i].zVel=((float)random(5)/10);
                }
            }
            for(int i=0;i<(NUM_ROCKETS%2==0?NUM_ROCKETS:NUM_ROCKETS-1);i+=2)
                line(rockets[i].x*cos(offset), rockets[i].y, rockets[i].z*sin(offset),rockets[i+1].x, rockets[i+1].y, rockets[i+1].z,rockets[i].col);
            offset+=0.1;
            mirror();
}
 
void mirror()
{
        for(int x=center.x;x<SIZE;x++)
                for(int y=0;y<SIZE;y++)
                        for(int z=center.z;z<SIZE;z++)
                        {
                                setVoxel(center.x-(x-center.x),y,z, getVoxel(x,y,z));
                                setVoxel(center.x-(x-center.x),y,center.z-(z-center.z), getVoxel(x,y,z));
                                setVoxel(x,y,center.z-(z-center.z), getVoxel(x,y,z));
                        }
}
 
void loop()
{


// This is where the listening magic happens. 

 if(listeningMode == true){

int32_t bytesrecv = Udp.parsePacket();
if(millis()-lastUpdated>60000)  //update the network settings every minute
{
    updateNetworkInfo();
    lastUpdated=millis();
}

if (bytesrecv>0) {
    if(hasReceivedData == false){
        hasReceivedData = true;
        }
    
    Udp.read(data,bytesrecv);
        _timestamp = Time.now();

          background(Color(0,0,0));

    for(int x=0;x<SIZE;x++){
        for(int y=0;y<SIZE;y++){
            for(int z=0;z<16;z++)
            {
                int index = z*256 + y*16 + x;
//                color pixelColor={ data[index]&0xE0, (data[index]&0x1C)<<3, (data[index]&0x03)<<6};   //colors as encoded
//                color pixelColor={ (data[index]&0xE0)>>1, (data[index]&0x1C)<<2, (data[index]&0x03)<<5};   //colors with max brightness set to 128
                Color pixelColor={ (data[index]&0xE0)>>2, (data[index]&0x1C)<<1, (data[index]&0x03)<<4};   //colors with max brightness set to 64
                setVoxel(x,y,z,pixelColor);
            }
        }
    }
    
Udp.flush();
}


}

else {

  if(fading)
        fade();
    else
    {
        switch(demo%DEMO_ROUTINES)
        {
           case(FIREWORKS):
                updateFireworks();
                break;
 
            case(CIRCLES):
                circles();
                break;
 
            case(ROMAN_CANDLE):
                romanCandle();
                break;
 
            case(FFT_JOY):
                fft_joy();
                break;
 
            case(PLASMA):
                zPlasma();
                break;
 
            case(FASTPLASMA):
                fastPlasma();
                break;
 
            case(DISPLAY_TEST):
                displayTest();
                break;
 
            default:
                break;
        }
        frame++;
    }

}
 if(listeningMode == true){
    // if(packetCounter > 7){
                show();

// packetCounter = 0;
// }

   
}

// Reset timer
    if(hasReceivedData == true){
        if (Time.now() - _timestamp > _timeoutInSeconds) {
Serial.println("Timed out");
Serial.println (Time.now() - _timestamp);
 background(Color(0,0,0));
        }
    }

else{
 show();
   if(autoCycle)
      if(millis()-lastDemo>DEMO_TIME)
      {
                fading=true;
                demo++;
                lastDemo=millis();
      }
 
    if(fading)
    {
        fadeValue-=fadeSpeed;
        //if we're done fading)
        if(fadeValue<=0)
        {
            fading=false;
            fadeValue=255;
        }
        else
            fade();
    }
 
   //uncomment to enable whack-to-change
 
  updateAccelerometer();
 
   if((whacked)&&((millis()-timeout)>250))
   {
        autoCycle=false;
        fading=true;
        demo++;
        timeout=millis();
        }

 }
 
}
 
void fade()
{
    setFadeSpeed();
    Color voxelColor;
        for(int x=0;x<SIZE;x++)
            for(int y=0;y<SIZE;y++)
                for(int z=0;z<SIZE;z++)
                    {
                        voxelColor=getVoxel(x,y,z);
                        if(voxelColor.red>0)
                            voxelColor.red--;
                        if(voxelColor.green>0)
                             voxelColor.green--;
                        if(voxelColor.blue>0)
                            voxelColor.blue--;
                        setVoxel(x,y,z, voxelColor);
                    }
}
 
 
 
/********************************************
 *   FFT JOY functions
 * *****************************************/
void fft_joy(){
    for(int i=0;i<pow(2,M);i++)
    {
        real[i]=analogRead(MICROPHONE)-993;  //adapted for the 0.8v bias point of the big cube
        delayMicroseconds(212);
        imaginary[i]=0;
    }
    FFT(1, M, real, imaginary);
    for(int i=0;i<pow(2,M);i++)
    {
        imaginary[i]=sqrt(pow(imaginary[i],2)+pow(real[i],2));
        if(imaginary[i]>maxValue)
            maxValue=imaginary[i];
    }
    if(maxValue>100)
        maxValue--;
    for(int i=0;i<pow(2,M)/2;i++)
    {
        imaginary[i]=SIZE*imaginary[i]/maxValue;
        int y;
        for(y=0;y<=imaginary[i];y++)
            setVoxel(i,y,SIZE-1,colorMap(y,0,SIZE));
        for(;y<SIZE;y++)
            setVoxel(i,y,SIZE-1,Color(0,0,0));
    }
    for(int z=0;z<SIZE-1;z++)
        for(int x=0;x<SIZE;x++)
            for(int y=0;y<SIZE;y++)
            {
                Color col=getVoxel(x,y,z+1);
                setVoxel(x,y,z,col);
            }
 
    sample++;
    if(sample>=pow(2,M))
        sample-=pow(2,M);
}
 
short FFT(short int dir,int m,float *x,float *y)
{
   int n,i,i1,j,k,i2,l,l1,l2;
   float c1,c2,tx,ty,t1,t2,u1,u2,z;
 
   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++)
      n *= 2;
 
   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }
 
   /* Compute the FFT */
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }
 
   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }
 
   return(0);
}
 
void circles()
{
   background(Color(0,0,0));
   poly1[0]=Point(center.x+(SIZE/2)*cos(poly1Angles[0]),0,center.z+(SIZE/2)*sin(poly1Angles[0]));
   poly1[1]=Point(0,center.y+(SIZE/2)*cos(poly1Angles[1]),center.z+(SIZE/2)*sin(poly1Angles[1]));
   poly1[2]=Point(center.x+(SIZE/2)*cos(poly1Angles[2]),SIZE-1,center.z+(SIZE/2)*sin(poly1Angles[2]));
   poly1[3]=Point(SIZE-1,center.y+(SIZE/2)*cos(poly1Angles[3]),center.z+(SIZE/2)*sin(poly1Angles[3]));
 
   poly2[0]=Point(center.x+(SIZE/2)*cos(poly2Angles[0]),center.y+(SIZE/2)*sin(poly2Angles[0]),0);
   poly2[1]=Point(SIZE-1,center.y+(SIZE/2)*cos(poly2Angles[1]),center.z+(SIZE/2)*sin(poly2Angles[1]));
   poly2[2]=Point(center.x+(SIZE/2)*cos(poly2Angles[2]),center.y+(SIZE/2)*sin(poly2Angles[2]),SIZE-1);
   poly2[3]=Point(0,center.y+(SIZE/2)*cos(poly2Angles[3]),center.z+(SIZE/2)*sin(poly2Angles[3]));
 
   poly3[0]=Point(center.x+(SIZE/2)*cos(poly3Angles[0]),0, center.z+(SIZE/2)*sin(poly3Angles[0]));
   poly3[1]=Point(center.x+(SIZE/2)*cos(poly3Angles[1]),center.y+(SIZE/2)*cos(poly3Angles[1]),0);
   poly3[2]=Point(center.x+(SIZE/2)*cos(poly3Angles[2]),SIZE-1, center.z+(SIZE/2)*sin(poly3Angles[2]));
   poly3[3]=Point(center.x+(SIZE/2)*cos(poly3Angles[3]),center.y+(SIZE/2)*cos(poly3Angles[3]),SIZE-1);
 
   for(int i=0;i<4;i++)
   {
        poly1Angles[i]+=poly1AnglesInc[i];
        poly2Angles[i]+=poly2AnglesInc[i];
        poly3Angles[i]+=poly3AnglesInc[i];
        float sin1=(float)255*sin(poly1Color);
        float sin2=(float)255*sin(poly2Color);
        float sin3=(float)255*sin(poly3Color);
        line(poly1[i], poly1[(i+1)%4], Color(abs(sin1),0,0));
        line(poly2[i], poly2[(i+1)%4], Color(0,0,abs(sin2)));
        line(poly3[i], poly3[(i+1)%4], Color(0,abs(sin3),0));
   }
   poly1Color+=poly1ColorInc;
   poly2Color+=poly2ColorInc;
   poly3Color+=poly3ColorInc;
}
 
void background(const Color& col)
{
    for(int x=0;x<SIZE;x++)
        for(int y=0;y<SIZE;y++)
            for(int z=0;z<SIZE;z++)
                setVoxel(x,y,z,col);
}
 
void setVoxel(int x, int y, int z, const Color& col)
{
    if((x>=0)&&(x<SIZE))
        if((y>=0)&&(y<SIZE))
            if((z>=0)&&(z<SIZE))
            {
                int index=z*256+x*16+y;


// if(listeningMode == true) {
//               index+=packetCounter*512;

// }
                
                PIXEL_RGB[index*3]=col.green;
                PIXEL_RGB[index*3+1]=col.red;
                PIXEL_RGB[index*3+2]=col.blue;
            }
}
 
void setVoxel(const Point& p, const Color& col)
{
    setVoxel(p.x, p.y, p.z, col);
}
 
void line(const Point& p1, const Point& p2, const Color& col)
{
    line(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, col);
}
 
void line(int x1, int y1, int z1, int x2, int y2, int z2, const Color& col)
{
  Point currentPoint;
  currentPoint.x=x1;
  currentPoint.y=y1;
  currentPoint.z=z1;
 
  int dx = x2 - x1;
  int dy = y2 - y1;
  int dz = z2 - z1;
  int x_inc = (dx < 0) ? -1 : 1;
  int l = abs(dx);
  int y_inc = (dy < 0) ? -1 : 1;
  int m = abs(dy);
  int z_inc = (dz < 0) ? -1 : 1;
  int n = abs(dz);
  int dx2 = l << 1;
  int dy2 = m << 1;
  int dz2 = n << 1;
 
  if((l >= m) && (l >= n)) {
    int err_1 = dy2 - l;
    int err_2 = dz2 - l;
 
    for(int i = 0; i < l; i++) {
      setVoxel(currentPoint, col);
 
      if(err_1 > 0) {
        currentPoint.y += y_inc;
        err_1 -= dx2;
      }
 
      if(err_2 > 0) {
        currentPoint.z += z_inc;
        err_2 -= dx2;
      }
 
      err_1 += dy2;
      err_2 += dz2;
      currentPoint.x += x_inc;
    }
  } else if((m >= l) && (m >= n)) {
    int err_1 = dx2 - m;
    int err_2 = dz2 - m;
 
    for(int i = 0; i < m; i++) {
      setVoxel(currentPoint, col);
 
      if(err_1 > 0) {
        currentPoint.x += x_inc;
        err_1 -= dy2;
      }
 
      if(err_2 > 0) {
        currentPoint.z += z_inc;
        err_2 -= dy2;
      }
 
      err_1 += dx2;
      err_2 += dz2;
      currentPoint.y += y_inc;
    }
  } else {
    int err_1 = dy2 - n;
    int err_2 = dx2 - n;
 
    for(int i = 0; i < n; i++) {
      setVoxel(currentPoint, col);
 
      if(err_1 > 0) {
        currentPoint.y += y_inc;
        err_1 -= dz2;
      }
 
      if(err_2 > 0) {
        currentPoint.x += x_inc;
        err_2 -= dz2;
      }
 
      err_1 += dy2;
      err_2 += dx2;
      currentPoint.z += z_inc;
    }
  }
 
  setVoxel(currentPoint, col);
}
 
 
Point add(const Point& a, const Point& b)
{
        return Point(a.x+b.x, a.y+b.y, a.z+b.z);
}
 
// draws a hollow  centered around the 'center' PVector, with radius
// radius and color col
void sphere(const Point& center, float radius, const Color& col)
{
     float res = 30;
     for (float m = 0; m < res; m++)
         for (float n = 0; n < res; n++)
             setVoxel(center.x + radius * sin((float) PI * m / res) * cos((float) 2 * PI * n / res),
                      center.y + radius * sin((float) PI * m / res) * sin((float) 2 * PI * n / res),
                      center.z + radius * cos((float) PI * m / res),
                      col);
}
 
// draws a hollow  centered around the 'center' PVector, with radius
// radius and color col
void sphere(const Point& center, float radius, const Color& col, int res)
{
     for (float m = 0; m < res; m++)
         for (float n = 0; n < res; n++)
             setVoxel(center.x + radius * sin((float) PI * m / res) * cos((float) 2 * PI * n / res),
                      center.y + radius * sin((float) PI * m / res) * sin((float) 2 * PI * n / res),
                      center.z + radius * cos((float) PI * m / res),
                      col);
}
 
void sphere(float x, float y, float z, float radius, const Color& col)
{
     sphere(Point(x,y,z),radius, col);
}
 
void sphere(float x, float y, float z, float radius, const Color& col, int res)
{
     sphere(Point(x,y,z),radius, col, res);
}
 
void show()
{
    uint8_t *ptrA,*ptrB,*ptrC,*ptrD,*ptrE,*ptrF,*ptrG,*ptrH;
    uint8_t mask;
    uint8_t c=0,a=0,b=0,j=0;
 
    GPIOA->BSRRH=0xE0;    //set A3~A5 to low
    GPIOB->BSRRH=0xF0;    //set D0~D4 to low
    GPIOC->BSRRH=0x04;     //set A2 to low
 
    ptrA=&PIXEL_RGB[0];
    ptrB=ptrA+stripPIXEL*3;
    ptrC=ptrB+stripPIXEL*3;
    ptrD=ptrC+stripPIXEL*3;
    ptrE=ptrD+stripPIXEL*3;
    ptrF=ptrE+stripPIXEL*3;
    ptrG=ptrF+stripPIXEL*3;
    ptrH=ptrG+stripPIXEL*3;
 
    delayMicroseconds(50);
    __disable_irq();
 
    uint16_t i=stripPIXEL*3;   //3 BYTES = 1 PIXEL
 
    while(i) { // While bytes left... (3 bytes = 1 pixel)
      i--;
      mask = 0x80; // reset the mask
      j=0;
        // reset the 8-bit counter
      do {
        a=0;
        b=0;
        c=0;
 
//========Set D0~D4, i.e. B7~B4=======
        if ((*ptrA)&mask) b|=0x10;// if masked bit is high
    //    else "nop";
        b<<=1;
        if ((*ptrB)&mask) b|=0x10;// if masked bit is high
    //    else "nop";
        b<<=1;
        if ((*ptrC)&mask) b|=0x10;// if masked bit is high
    //    else "nop";
        b<<=1;
        if ((*ptrD)&mask) b|=0x10;// if masked bit is high
   //     else "nop";
 
//=========Set A2, i.g. C2==========
        if ((*ptrE)&mask) c|=0x04;// if masked bit is high
   //     else "nop";
 
        GPIOA->BSRRL=0xE0;    //set A3~A5 to high
        GPIOB->BSRRL=0xF0;    //set D0~D4 to high
        GPIOC->BSRRL=0x04;    //set A2 to high
 
 
//=========Set A3~A5, i.e. A5~A7========
        if ((*ptrF)&mask) a|=0x80;// if masked bit is high
        // else "nop";
        a>>=1;
        if ((*ptrG)&mask) a|=0x80;// if masked bit is high
   //     else "nop";
        a>>=1;
        if ((*ptrH)&mask) a|=0x80;// if masked bit is high
 
        a=(~a)&0xE0;
        b=(~b)&0xF0;
        c=(~c)&0x04;
        GPIOA->BSRRH=a;
        GPIOB->BSRRH=b;
        GPIOC->BSRRH=c;
        mask>>=1;
         asm volatile(
            "mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t"
            ::: "r0", "cc", "memory");
        GPIOA->BSRRH=0xE0;    //set all to low
        GPIOB->BSRRH=0xF0;    //set all to low
        GPIOC->BSRRH=0x04;    //set all to low
          // WS2812 spec             700ns HIGH
          // Adafruit on Arduino    (meas. 812ns)
          // This lib on Spark Core (meas. 792ns)
          /*
        if(j<7) {
          asm volatile(
            "mov r0, r0" "\n\t"
            ::: "r0", "cc", "memory");
        }
        */
 
      } while ( ++j < 8 ); // ...one color on a pixel done
      ptrA++;
      ptrB++;
      ptrC++;
      ptrD++;
      ptrE++;
      ptrF++;
      ptrG++;
      ptrH++;
    } // end while(i) ... no more pixels
    __enable_irq();
}
 
 
 
/** Map a value into a color.
  The set of colors fades from blue to green to red and back again.
 
  @param val Value to map into a color.
  @param minVal Minimum value that val will take.
  @param maxVal Maximum value that val will take.
 
  @return Color from value.
*/
Color colorMap(float val, float minVal, float maxVal)
{
  const float range = 1024;
  val = range * (val-minVal) / (maxVal-minVal);
 
  Color colors[6];
 
  colors[0].red = 0;
  colors[0].green = 0;
  colors[0].blue = maxBrightness;
 
  colors[1].red = 0;
  colors[1].green = maxBrightness;
  colors[1].blue = maxBrightness;
 
  colors[2].red = 0;
  colors[2].green = maxBrightness;
  colors[2].blue = 0;
 
  colors[3].red = maxBrightness;
  colors[3].green = maxBrightness;
  colors[3].blue = 0;
 
  colors[4].red = maxBrightness;
  colors[4].green = 0;
  colors[4].blue = 0;
 
  colors[5].red = maxBrightness;
  colors[5].green = 0;
  colors[5].blue = maxBrightness;
 
  if(val <= range/6)
    return lerpColor(colors[0], colors[1], val, 0, range/6);
  else if(val <= 2 * range / 6)
    return(lerpColor(colors[1], colors[2], val, range / 6, 2 * range / 6));
  else if(val <= 3 * range / 6)
    return(lerpColor(colors[2], colors[3], val, 2 * range / 6, 3*range / 6));
  else if(val <= 4 * range / 6)
    return(lerpColor(colors[3], colors[4], val, 3 * range / 6, 4*range / 6));
  else if(val <= 5 * range / 6)
    return(lerpColor(colors[4], colors[5], val, 4 * range / 6, 5*range / 6));
  else
    return(lerpColor(colors[5], colors[0], val, 5 * range / 6, range));
}
 
/** Linear interpolation between colors.
 
  @param a, b The colors to interpolate between.
  @param val Position on the line between color a and color b.
  When equal to min the output is color a, and when equal to max the output is color b.
  @param minVal Minimum value that val will take.
  @param maxVal Maximum value that val will take.
 
  @return Color between colors a and b.
*/
Color lerpColor(const Color& a, const Color& b, int val, int minVal, int maxVal)
{
  int red = a.red + (b.red-a.red) * (val-minVal) / (maxVal-minVal);
  int green = a.green + (b.green-a.green) * (val-minVal) / (maxVal-minVal);
  int blue = a.blue + (b.blue-a.blue) * (val-minVal) / (maxVal-minVal);
 
  return Color(red, green, blue);
}
 
/***************************************
 * fireworks functions *
 * ***********************************/
 
 
void updateFireworks()
{
        background(Color(0,0,0));
//loop through all the pixels, calculate the distance to the center point, and turn the pixel on if it's at the right radius
            if(showRocket)
                sphere(rocketX,rocketY,rocketZ,radius,rocketColor);
            if(exploded)
                sphere(centerX,centerY,centerZ,radius,fireworkColor, fireworkRes);
 
        if(exploded)
            radius+=speed;  //the sphere gets bigger
        if(showRocket)
        {
            rocketX+=xInc;
            rocketY+=yInc;
            rocketZ+=zInc;
        }
        //if our sphere gets too large, restart the animation in another random spot
        if(radius>maxSize)
            prepRocket();
        if(abs(distance(centerX,centerY,centerZ,rocketX, rocketY, rocketZ)-radius)<2)
            {
                showRocket=false;
                exploded=true;
            }
}
 
float distance(float x, float y, float z, float x1, float y1, float z1)
{
    return(sqrt(pow(x-x1,2)+pow(y-y1,2)+pow(z-z1,2)));
}
 
void prepRocket()
{
        fireworkRes=10+rand()%20;
            radius=0.25;
//          centerX=0;
//          centerY=14;
//          centerZ=8;
            centerX=rand()%8;
            centerY=rand()%8;
            centerZ=rand()%8;
            fireworkColor.red=rand()%maxBrightness;
            fireworkColor.green=rand()%maxBrightness;
            fireworkColor.blue=rand()%maxBrightness;
            launchX=rand()%8;
            launchZ=rand()%8;
            rocketX=launchX;
            rocketY=0;
            rocketZ=launchZ;
            launchTime=10+rand()%15;
            xInc=(centerX-rocketX)/launchTime;
            yInc=(centerY-rocketY)/launchTime;
            zInc=(centerZ-rocketZ)/launchTime;
            showRocket=true;
            exploded=false;
            speed=0.35;
            maxSize=SIZE/2+random(SIZE/2);
            //speed=rand()%5;
            //speed*=0.1;
}
 
void initFireworks()
{
    rocketColor.red=255;
    rocketColor.green=150;
    rocketColor.blue=100;
    prepRocket();
}
 
Color getVoxel(int x, int y, int z)
{
    Color col=Color(0,0,0);
    if((x>=0)&&(x<SIZE))
        if((y>=0)&&(y<SIZE))
            if((z>=0)&&(z<SIZE))
            {
                int index=z*256+x*16+y;
                col.red=PIXEL_RGB[index*3+1];
                col.green=PIXEL_RGB[index*3];
                col.blue=PIXEL_RGB[index*3+2];
            }
    return col;
}
 
Color getVoxel(const Point& p)
{
        return getVoxel(p.x, p.y, p.z);
}
 
void setFadeSpeed()
{
    if(autoCycle)
        fadeSpeed=2;
    else
        fadeSpeed=20;
}
 
 void incrementDemo()
 {
     demo++;
     setFadeSpeed();
     fading=true;
     if(demo>=DEMO_ROUTINES)
        demo=0;
 }
 
  void decrementDemo()
 {
     demo--;
     setFadeSpeed();
     fading=true;
     if(demo<0)
        demo=DEMO_ROUTINES-1;
 }