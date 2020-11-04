#include <SoftwareSerial.h>
#include <Keypad.h>

int trig=7;//7
int echo=2;//2

//easterEG
int ledPin = 13;
//led for visualization (use 13 for built-in led)

int speakerPin = 13;
//speaker connected to one of the PWM ports

#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

//tastatura
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'},
  {'A','B','C','D'}
};
byte rowPins[ROWS] = {6, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 9, A0,A1}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//tastatura

#define control 12 // pin that controls the MOSFET

//communication
SoftwareSerial mySerial(11,10);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  // IRF520 MOSFET 
  pinMode(control,OUTPUT);// define control pin as output
  //ultra
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  //easterEG
  pinMode(ledPin, OUTPUT);
  // sets the ledPin to be an output
  pinMode(speakerPin, OUTPUT);  
  //sets the speakerPin to be an output
}

static int val_in_mililitri=0;
char key;

void loop() {
  //ultrasunete
  double val;
  double ddd;

    digitalWrite(trig,LOW);
    delayMicroseconds(2);
    digitalWrite(trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(trig,LOW);
    //FUNCTIE PT MASURAREA DURATEI IMPULSULUI
    val=pulseIn(echo,HIGH);
    ddd=(val*0.034)/2;
    Serial.println();Serial.println(ddd);Serial.println();

     if(ddd>15)
      mySerial.write("a");
      
    if(ddd>13.5 && ddd<15)
      mySerial.write("b");
      
    if(ddd>12 && ddd<13.5)
      mySerial.write("c");
      
    if(ddd>10.5 && ddd<12)
      mySerial.write("d");
      
    if(ddd>9 && ddd<10.5)
      mySerial.write("e");
      
    if(ddd>7.5 && ddd<9)
      mySerial.write("f");
      
    if(ddd>4.5 && ddd<7.5)
      mySerial.write("g");
      
    if(ddd>3 && ddd<4.5)
      mySerial.write("h");
      
    if(ddd<3)
      mySerial.write("i");
    
   while(!mySerial.available())
      return;
    
   key = mySerial.read();
   Serial.print(key);
   
    
  if(key == 'l')
  {
    if(ddd>15)
    {
      Serial.println();Serial.println("0  %");Serial.println();
      citireaSiDozare(23);
    }
    if(ddd>13.5 && ddd<15)
    {
      Serial.println();Serial.print("15");Serial.print(" %");Serial.println();
      citireaSiDozare(23);
    }
    if(ddd>12 && ddd<13.5)
    {
      Serial.println();Serial.print("25");Serial.print(" %");Serial.println();
      citireaSiDozare(23);
    }
    if(ddd>10.5 && ddd<12)
    {
      Serial.println();Serial.print("40");Serial.print(" %");Serial.println();
      citireaSiDozare(21);
    }
    if(ddd>9 && ddd<10.5)
    {
      Serial.println();Serial.print("50");Serial.print(" %");Serial.println();
      citireaSiDozare(20);
    }
    if(ddd>7.5 && ddd<9)
    {
      Serial.println();Serial.print("65");Serial.print(" %");Serial.println();
      citireaSiDozare(19);
    }
    if(ddd>4.5 && ddd<7.5)
    {
      Serial.println();Serial.print("75");Serial.print(" %");Serial.println();
      citireaSiDozare(18);
    }
    if(ddd>3 && ddd<4.5)
    {
      Serial.println();Serial.print("90");Serial.print(" %");Serial.println();
      citireaSiDozare(18);
    }
    if(ddd<3)
    {
      Serial.println();Serial.print("100");Serial.print(" %");Serial.println();
      citireaSiDozare(18);
    }
  }
}
void citireaSiDozare(int numarulInmultitor)
{
   //se citeste valoarea de mililitri de la tastatura
   val_in_mililitri=GetNumber();
   //se afiseaza valoarea de mililitri de la tastatura
   Serial.print(val_in_mililitri);Serial.println(" ml");
   //se transforma in secunde
   int secunde=(val_in_mililitri*numarulInmultitor)/250;
   //se afiseaza cate secunde o sa mearga pompa de apa
   Serial.print(secunde);Serial.println("sec");
   //transmit mililitri catre cealalta placuta
   char ml[3];
   itoa(val_in_mililitri,ml,0);
   //Serial.write(ml,3);
   //easterEG
   if(val_in_mililitri==10973)
   {
    march();
   }
   //golire toatala
   if(val_in_mililitri==3000)
   {
    digitalWrite(control,HIGH); // turn the MOSFET Switch ON
   }
   //verificarea
   if(val_in_mililitri<500)
   {
     digitalWrite(control,HIGH); // turn the MOSFET Switch ON
     delay(secunde*1000);
     digitalWrite(control,LOW); // Turn the MOSFET Switch OFF
   }
}
int tareMult;
int GetNumber()
{
   int number = 0;
   char key = keypad.getKey();
   if(key=='A'){
     mySerial.write('A');
     tone(speakerPin, 50,100);
   }
   while(key!='A')
   {
     switch (key)
     {
       case NO_KEY:
       break;
  
       case '0': case '1': case '2': case '3': case '4':case '5': case '6': case '7': case '8': case '9':
        tone(speakerPin, 50,100);  
        number = number * 10 + (key - '0');
       break;
     }
    key = keypad.getKey();
   }

   return number;
}
//easterEG
void beep (unsigned char speakerPin, int frequencyInHertz, long timeInMilliseconds)
{ 
    digitalWrite(ledPin, HIGH);  
    //use led to visualize the notes being played
    
    int x;   
    long delayAmount = (long)(1000000/frequencyInHertz);
    long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
    for (x=0;x<loopTime;x++)   
    {    
        digitalWrite(speakerPin,HIGH);
        delayMicroseconds(delayAmount);
        digitalWrite(speakerPin,LOW);
        delayMicroseconds(delayAmount);
    }    
    
    digitalWrite(ledPin, LOW);
    //set led back to low
    
    delay(20);
    //a little delay to make all notes sound separate
}    
     
void march()
{    
    //for the sheet music see:
    //http://www.musicnotes.com/sheetmusic/mtd.asp?ppn=MN0016254
    //this is just a translation of said sheet music to frequencies / time in ms
    //used 500 ms for a quart note
    
    beep(speakerPin, a, 500); 
    beep(speakerPin, a, 500);     
    beep(speakerPin, a, 500); 
    beep(speakerPin, f, 350); 
    beep(speakerPin, cH, 150);
    
    beep(speakerPin, a, 500);
    beep(speakerPin, f, 350);
    beep(speakerPin, cH, 150);
    beep(speakerPin, a, 1000);
    //first bit
    
    beep(speakerPin, eH, 500);
    beep(speakerPin, eH, 500);
    beep(speakerPin, eH, 500);    
    beep(speakerPin, fH, 350); 
    beep(speakerPin, cH, 150);
    
    beep(speakerPin, gS, 500);
    beep(speakerPin, f, 350);
    beep(speakerPin, cH, 150);
    beep(speakerPin, a, 1000);
    //second bit...
    
    beep(speakerPin, aH, 500);
    beep(speakerPin, a, 350); 
    beep(speakerPin, a, 150);
    beep(speakerPin, aH, 500);
    beep(speakerPin, gSH, 250); 
    beep(speakerPin, gH, 250);
    
    beep(speakerPin, fSH, 125);
    beep(speakerPin, fH, 125);    
    beep(speakerPin, fSH, 250);
    delay(250);
    beep(speakerPin, aS, 250);    
    beep(speakerPin, dSH, 500);  
    beep(speakerPin, dH, 250);  
    beep(speakerPin, cSH, 250);  
    //start of the interesting bit
    
    beep(speakerPin, cH, 125);  
    beep(speakerPin, b, 125);  
    beep(speakerPin, cH, 250);      
    delay(250);
    beep(speakerPin, f, 125);  
    beep(speakerPin, gS, 500);  
    beep(speakerPin, f, 375);  
    beep(speakerPin, a, 125); 
    
    beep(speakerPin, cH, 500); 
    beep(speakerPin, a, 375);  
    beep(speakerPin, cH, 125); 
    beep(speakerPin, eH, 1000); 
    //more interesting stuff (this doesn't quite get it right somehow)
    
    beep(speakerPin, aH, 500);
    beep(speakerPin, a, 350); 
    beep(speakerPin, a, 150);
    beep(speakerPin, aH, 500);
    beep(speakerPin, gSH, 250); 
    beep(speakerPin, gH, 250);
    
    beep(speakerPin, fSH, 125);
    beep(speakerPin, fH, 125);    
    beep(speakerPin, fSH, 250);
    delay(250);
    beep(speakerPin, aS, 250);    
    beep(speakerPin, dSH, 500);  
    beep(speakerPin, dH, 250);  
    beep(speakerPin, cSH, 250);  
    //repeat... repeat
    
    beep(speakerPin, cH, 125);  
    beep(speakerPin, b, 125);  
    beep(speakerPin, cH, 250);      
    delay(250);
    beep(speakerPin, f, 250);  
    beep(speakerPin, gS, 500);  
    beep(speakerPin, f, 375);  
    beep(speakerPin, cH, 125); 
           
    beep(speakerPin, a, 500);            
    beep(speakerPin, f, 375);            
    beep(speakerPin, c, 125);            
    beep(speakerPin, a, 1000);       
    //and we're done \ó/    
}
