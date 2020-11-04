#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>
//#include<Stream.h>

// Create instances
MFRC522 mfrc522(10, 9); // MFRC522 mfrc522(SS_PIN, RST_PIN)
LiquidCrystal_I2C lcd(0x27, 16, 4);
SoftwareSerial myserial(8,2);
    
// Set Pins for led's and wipe button
const int ir = 4; // senzor de proximitate
constexpr uint8_t greenLed = 7;
constexpr uint8_t yellowLed = 6;
constexpr uint8_t redLed = 5;
constexpr uint8_t wipeB = 3;     // Button pin for WipeMode
boolean match = false;          // initialize card match to false
boolean programMode = false;  // initialize programming mode to false
boolean replaceMaster = false;
uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
char c,aux;
char incoming ;
char procetanj_apa[3]="";


unsigned int integerValue=0;  // Max value is 65535
char incomingByte;

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  myserial.begin(9600);
  Serial.begin(9600);
  
  //Arduino Pin Configuration
  pinMode(ir, INPUT);//sensor IR
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(wipeB, INPUT);   // Enable pin's pull up resistor
  // Make sure led's are off
  digitalWrite(ir, LOW);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(wipeB,HIGH);
  //Protocol Configuration
  lcd.init();  // initialize the LCD
  lcd.backlight();
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
  //Wipe Code - If the Button (wipeB) Pressed while setup run (powered on) it wipes EEPROM


  
  // Check if admin card defined, if not let user choose a admin card
  // This also useful to just redefine the Admin Card
  // I write 143 to EEPROM address 1 to know when we have Admin
  if (EEPROM.read(1) != 143) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Admin Card Set");
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("Scan A Tag to ");
    lcd.setCursor(0, 1);
    lcd.print("Set as Admin");
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      // Visualize Admin Card need to be defined
      digitalWrite(yellowLed, HIGH);
      delay(200);
      digitalWrite(yellowLed, LOW);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned Tag's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Admin Card.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Admin set");
    delay(2000);
  }
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Admin Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to adminCard
  }
  ShowOnLCD();    // Print data on LCD
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}
///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    if (programMode) {
      cycleLeds();              // Program Mode cycles through Red Green Yellow waiting to read a new card
    }
    else {
      normalModeOn();     // Normal mode, yellow Power LED is on, all others are off
    }
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
  if (programMode) {
    if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Exit programming");
      lcd.setCursor(0, 1);
      lcd.print("mode!");
      lcd.setCursor(0, 2);
      lcd.print("Bye, ADMIN");
      delay(3000);
      ShowOnLCD();
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Already registered,");
        lcd.setCursor(0, 1);
        lcd.print("removal...");
        deleteID(readCard);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Add/Remove another");
        lcd.setCursor(9,1);
        lcd.print("or");
        lcd.setCursor(0, 2);
        lcd.print("ADMIN card to exit");
      }
      else {                    // If scanned card is not known add it
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("New Tag,");
        lcd.setCursor(0, 1);
        lcd.print("adding...");
        writeID(readCard);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Add/Remove another");
        lcd.setCursor(9,1);
        lcd.print("or");
        lcd.setCursor(0, 2);
        lcd.print("ADMIN card to exit");
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
      programMode = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hello, ADMIN!");
      lcd.setCursor(0, 1);
      lcd.print("You are in");
      lcd.setCursor(0, 2);
      lcd.print("programming mode!");
      uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that stores the number of ID's in EEPROM
      lcd.setCursor(0, 3);
      lcd.print("No. of USERS: ");
      lcd.print(count);
      delay(4000);

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Hold reset button");
      lcd.setCursor(0,1);
      lcd.print("to delete ALL data");
      delay(3000);
      
      lcd.clear();
      if (digitalRead(wipeB) == LOW) {  // when button pressed pin should get low, button connected to ground
        digitalWrite(redLed, HIGH); // Red Led stays on to inform user we are going to wipe
        lcd.setCursor(0, 0);
        lcd.print("Button Pressed");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("This will remove");
        lcd.setCursor(0, 1);
        lcd.print("all records");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("You have 10 ");
        lcd.setCursor(0, 1);
        lcd.print("secs to Cancel");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Unpres to cancel");
        lcd.setCursor(0, 1);
        lcd.print("Counting: ");
        bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
        if (buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
          lcd.print("Wiping EEPROM...");
          for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
            if (EEPROM.read(x) == 0) {              //If EEPROM address 0
              // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
            }
            else {
              EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
            }
          }
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wiping Done");
          // visualize a successful wipe
          digitalWrite(redLed, LOW);
          delay(200);
          digitalWrite(redLed, HIGH);
          delay(200);
          digitalWrite(redLed, LOW);
          delay(200);
          digitalWrite(redLed, HIGH);
          delay(200);
          digitalWrite(redLed, LOW);
        }
        else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wiping Cancelled"); // Show some feedback that the wipe button did not pressed for 10 seconds
          digitalWrite(redLed, LOW);
        }
      }
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ADD/REMOVE a card");
      lcd.setCursor(9, 1);
      lcd.print("or");
      lcd.setCursor(0, 2);
      lcd.print("ADMIN card to exit");
    }
    else {
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        granted();      
        ShowOnLCD();
      }
      else {      // If not, show that the Access is denied
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        denied();
        ShowOnLCD();
      }
    }
  }
}
/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted () {
  myserial.write("l");
  
  digitalWrite(yellowLed, LOW);   // Turn off blue LED
  digitalWrite(redLed, LOW);  // Turn off red LED
  digitalWrite(greenLed, HIGH);   // Turn on green LED
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello, User");
  lcd.print(findIDSLOT(readCard));   
  int z=9;
  while (digitalRead(ir) == HIGH){
    lcd.setCursor(0,1);
    digitalWrite(13,HIGH);
    lcd.print("---Missing glass----");
    if(z==0){
      goto final;
    }
    lcd.setCursor(0,2);
    lcd.print("Log off in: ");
    lcd.print(z);
    delay(1000);
    z--;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello, User");
  lcd.print(findIDSLOT(readCard));   
  digitalWrite(13,LOW);
  lcd.setCursor(0,1);
  lcd.print("Glass placed"); 
  //delay(300);
  
  lcd.setCursor(0,2);
  lcd.print("-Enter ml of water-");
  
  lcd.setCursor(0,3);
  for(int i=19;i>=0;i--){
     lcd.setCursor(i, 3);
     lcd.print('-');
     delay(30);
   }
  delay(6000);
  final:
  return;
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LOW);  // Make sure green LED is off
  digitalWrite(yellowLed, LOW);   // Make sure blue LED is off
  digitalWrite(redLed, HIGH);   // Turn on red LED
  delay(1000);
}
///////////////////////////////////////// Get Tag's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading Tags
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new Tag placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a Tag placed get Serial and continue
    return 0;
  }
  // There are Mifare Tags which have 4 byte or 7 byte UID care if you use 7 byte Tag
  // I think we should assume every Tag as they have 4 byte UID
  // Until we support 7 byte Tags
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
  }
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}
/////////////////////// Check if RFID Reader is correctly initialized or not /////////////////////
void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    lcd.setCursor(0, 0);
    lcd.print("Communication Failure");
    lcd.setCursor(0, 1);
    lcd.print("Check Connections");
 
    delay(2000);
    // Visualize system is halted
    digitalWrite(greenLed, LOW);  // Make sure green LED is off
    digitalWrite(yellowLed, LOW);   // Make sure yellow LED is off
    digitalWrite(redLed, HIGH);   // Turn on red LED
    while (true); // do not go further
  }
}
///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LOW);  // Make sure red LED is off
  digitalWrite(greenLed, HIGH);   // Make sure green LED is on
  digitalWrite(yellowLed, LOW);   // Make sure yellow LED is off
  delay(200);
  digitalWrite(redLed, LOW);  // Make sure red LED is off
  digitalWrite(greenLed, LOW);  // Make sure green LED is off
  digitalWrite(yellowLed, HIGH);  // Make sure yellow LED is on
  delay(200);
  digitalWrite(redLed, HIGH);   // Make sure red LED is on
  digitalWrite(greenLed, LOW);  // Make sure green LED is off
  digitalWrite(yellowLed, LOW);   // Make sure yellow LED is off
  delay(200);
}
//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  
  digitalWrite(redLed, LOW);  // Make sure Red LED is off
  digitalWrite(greenLed, LOW);  // Make sure Green LED is off
  digitalWrite(yellowLed, HIGH);  // Yellow LED ON and ready to read card
}
//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}
///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    BlinkLEDS(greenLed);
    lcd.setCursor(0, 2);
    lcd.print("It's added!");
    delay(1000);
  }
  else {
    BlinkLEDS(redLed);
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("wrong ID or bad EEPROM");
    delay(2000);
  }
}
///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    BlinkLEDS(redLed);      // If not
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("wrong ID or bad EEPROM");
    delay(2000);
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    BlinkLEDS(yellowLed);
    lcd.setCursor(0, 2);
    lcd.print("It's removed!");
    delay(1000);
  }
}
///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )      // Make sure there is something in the array first
    match = true;       // Assume they match at first
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] )     // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {      // Check to see if if match is still true
    return true;      // Return true
  }
  else  {
    return false;       // Return false
  }
}
///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
      break;          // Stop looking we found it
    }
  }
}
///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
      break;  // Stop looking we found it
    }
  }
  return false;
}
///////////////////////////////////////// Blink LED's For Indication   ///////////////////////////////////
void BlinkLEDS(int led) {
  digitalWrite(yellowLed, LOW);   // Make sure blue LED is off
  digitalWrite(redLed, LOW);  // Make sure red LED is off
  digitalWrite(greenLed, LOW);  // Make sure green LED is off
  delay(200);
  digitalWrite(led, HIGH);  // Make sure blue LED is on
  delay(200);
  digitalWrite(led, LOW);   // Make sure blue LED is off
  delay(200);
  digitalWrite(led, HIGH);  // Make sure blue LED is on
  delay(200);
  digitalWrite(led, LOW);   // Make sure blue LED is off
  delay(200);
  digitalWrite(led, HIGH);  // Make sure blue LED is on
  delay(200);
}
////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}
/////////////////// Counter to check in reset/wipe button is pressed or not   /////////////////////
bool monitorWipeButton(uint32_t interval) {
  unsigned long currentMillis = millis(); // grab current time
  while (millis() - currentMillis < interval)  {
    int timeSpent = (millis() - currentMillis) / 1000;
    Serial.println(timeSpent);
    lcd.setCursor(10, 1);
    lcd.print(timeSpent);
    // check on every half a second
    if (((uint32_t)millis() % 10) == 0) {
      if (digitalRead(wipeB) != LOW) {
        return false;
      }
    }
  }
  return true;
}

////////////////////// Print Info on LCD   ///////////////////////////////////
void ShowOnLCD() {
  while(!myserial.available()){
    return;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  char s[20]="FUEL:";
  c=myserial.read();
    
    if(c=='a')
      strcat(s,"0%-------------");
    else if(c=='b')
      strcat(s,"15%------------");
    else if(c=='c')
      strcat(s,"25%------------");
    else if(c=='d')
      strcat(s,"40%------------");
    else if(c=='e')
      strcat(s,"50%------------");
    else if(c=='f')
      strcat(s,"65%------------");
    else if(c=='g')
      strcat(s,"75%------------");
    else if(c=='h')
      strcat(s,"90%------------");
    else if(c=='i'){
      strcat(s,"100%-----------");
    }
    else{
     strcat(s,"error-----------");
    }
    
      
    lcd.setCursor(0, 1);
    lcd.print("   WATER DISPENSER  ");
  
    lcd.setCursor(0, 2);
    lcd.print(" Please scan a card ");
    //char s[20]="--------------------";
    for(int i=0;i<=19;i++){
      lcd.setCursor(i,0);
      lcd.print(s[i]);
      delay(30);
     }
     
    char s2[20]="--------------------";
    for(int i=19;i>=0;i--){
      lcd.setCursor(i, 3);
      lcd.print(s2[i]);
      delay(30);
     }
}
