/* Serial setup */
#define SERIALSPEED		9600

/* Ir setup */
#define IRSERIALSPEED	300
//#define IRSERIALCONF  SERIAL_8N1 - SoftwareSerial doesn't have 7E1 mode, drop MSB instead
#include <SoftwareSerial.h>
#define IRrxPin 6 // IR rx
#define IRrxInverted 0 // IR rx non-inverted - IR receiver is active LOW
#define IRtxPin 7 // IR tx inverted - IR led is active HIGH
#define IRtxInverted 1 // IR tx inverted - IR led is active HIGH
#define NOrxPin 8 // NOT used
#define NOtxPin 9 // NOT used


String UnitRecognition = "/            .     "; // change with your meter signature response !
//  check unit signature "/xxxxxxxxxxxx.yyy\r\n"
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

bool poll = false;
bool ack = false;
bool clearserial = false;

// set up a new serial port
SoftwareSerial mySerialTX =  SoftwareSerial(NOrxPin, IRtxPin, IRtxInverted);
SoftwareSerial mySerialRX =  SoftwareSerial(IRrxPin, NOtxPin, IRrxInverted);

void setup() {
	// initialize serial:
  pinMode(IRrxPin, INPUT);
  pinMode(IRtxPin, OUTPUT);
  pinMode(NOrxPin, INPUT);
  pinMode(NOtxPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(SERIALSPEED);
  mySerialTX.begin(IRSERIALSPEED);
  mySerialRX.begin(IRSERIALSPEED);
	// reserve 200 bytes for the inputString:
	inputString.reserve(200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
}

void loop() {
	// print the string when a newline arrives:
	if (mySerialRX.available()) {
	  IRserialEvent();
  }
	if (stringComplete) {
//		Serial.print("receive : ");
		Serial.print(inputString);
		// clear the string:
		inputString = "";
		stringComplete = false;
	}

	if (poll == false) {
		mySerialRX.begin(IRSERIALSPEED);
		delay(100);
//		Serial.println("send : /?!<CR><LF>\r\n");
    mySerialTX.listen();
		mySerialTX.write(0xAF); // "/"  /
		mySerialTX.write(0x3F); // "?"  ?
		mySerialTX.write(0x21); // "!"  !
		mySerialTX.write(0x8D); // "\r" CR
		mySerialTX.write(0x0A); // "\n" LF
		poll = true;
    mySerialRX.listen();
	}
	if (ack == true) {
		mySerialRX.begin(IRSERIALSPEED);
		delay(100);
//		Serial.println(F("send : <ACK>000<CR><LF>\r\n"));
    mySerialTX.listen();
		mySerialTX.write(0x06); // "<ACK>"
		mySerialTX.write(0x30); // "0"
		mySerialTX.write(0x30); // "0" 300 "1" 600
		mySerialTX.write(0x30); // "0"
		mySerialTX.write(0x8D); // "\r" CR
		mySerialTX.write(0x0A); // "\n" LF
		ack = false;
    mySerialRX.listen();
	}
//delay(5000);
}

// IR Serial event
void IRserialEvent() {
	while (mySerialRX.available()) {
		// get the new byte:
    mySerialRX.listen();
		char inChar = (char)mySerialRX.read();
		inChar = inChar & 0x7F; // drop MSB = convert 8N1 to 7E1 mode
//		
//		Serial.print(F("DEBUG : int("));
//		Serial.print((int)inChar);
//		Serial.print(F(") ASCII : "));
//		Serial.println(inChar);
//		
		// FIND <STX>
		if (inChar == 2) {
			inChar = *"";
		}
		// FIND ! and remove
    else if (inChar == 33) {
      inChar = *"";
    }
		else {
			inputString = inputString + inChar;
		}

		// FIND <ETX>
		if (inChar == 3) {
			clearserial = true;
			inChar = *"";
			poll = false;
			mySerialRX.flush();
			mySerialRX.end();
      delay(47200); // magic number to have a push interval of 1 minute
		}

		// if the incoming character is a newline, set a flag so the main loop can
		// do something about it:
		if (inChar == '\n') {
			stringComplete = true;
		}

//  if (inputString == UnitRecognition) {
    if (inputString.length() == 19) {
  		if ((inputString[0] == UnitRecognition[0]) && (inputString[13] == UnitRecognition[13])) { // check unit signature "/xxxxxxxxxxxx.yyy" 
  			ack = true;
  		}  
		}

		if (clearserial == true) {
			while (mySerialRX.available() > 0) { char trash = mySerialRX.read(); }
			inputString = "";
			clearserial = false;
		}
	}
}

