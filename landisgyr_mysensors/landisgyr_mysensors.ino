/*	Landis & Gyr E350

	Read electric meter with IR serial and send to MySensors gateway

	Buchs Sullivan
*/

//#define MY_DEBUG 

// Optional define Node ID
#define MY_NODE_ID 27

// Define radio for MySensors
#define MY_RADIO_NRF24
#define MY_SOFTSPI
#define MY_RF24_CE_PIN 3
#define MY_RF24_CS_PIN 4
#define MY_SOFT_SPI_SCK_PIN 5
#define MY_SOFT_SPI_MOSI_PIN 6
#define MY_SOFT_SPI_MISO_PIN 7
#define MY_RF24_IRQ_PIN 8

// Define USB serial speed
#define SERIALSPEED    9600

// Define IR serial speed and config
#define IRSERIALSPEED 300
#define IRSERIALCONF  SERIAL_7E1

String inputString = "";  // a String to hold incoming data
String content = "";			// Content from ir serial

bool stringComplete = false;	// whether the string is complete
bool poll = false;				    // POLL flag when sending message to electric meter
bool ack = false;				      // ACK flag when sending message to electric meter
bool clearserial = false;		  // Clearing serial if needed

uint32_t currentTime = 0;			    // Actual time
uint32_t sendFrequency = 600000;	// frequency send to MySensors
uint32_t lastSend = 0;				    // Last send time
uint32_t readFrequency = 60000;		// frequency read electric meter 
uint32_t lastread = 0;				    // Last read electric meter time

const byte maxData = 30;      // Maximum data size
char response;					      // 
char label[maxData + 1];	    // 
char value[maxData + 1];	    // 
char unit[maxData + 1];		    // 
char message[] = "";			    // 
char * ElectricMeterResponse;	// 
 
size_t nbOfCharacter;			// Character number of the received message

String VFF;   // Error code (always first in the list)
String V181;  // Active energy import rate 1 (X = 1…6)
String V182;  // Active energy import rate 2 (X = 1…6)
String V881;  // kvarh rate 1
String V882;  // kvarh rate 2
String V180;  // Total active energy import
String V880;  // for the Reactive Energy Q4 Billing Totals
String VC70;  // Power Fail Count
String V327;  // VRMS Phase L1 (L12 in ZFFxxx meters)
String V527;  // VRMS Phase L2 (not for 3-wire ZFFxxx meters)
String V727;  // VRMS Phase L3 (L32 in ZFFxxx meters)
String V317;  // IRMS Phase L1
String V517;  // IRMS Phase L2 (not for 3-wire ZFFxxx meters)
String V717;  // IRMS Phase L3
String V337;  // Power factor phase L1 (4-wire ZMF1xx meters only)
String V537;  // Power factor phase L2 (4-wire ZMF1xx meters only)
String V737;  // Power factor phase L3 (4-wire ZMF1xx meters only)
String V137;  // Power Factor Phase Summation
String V8281; // Terminal cover removal counter
String V8282; // DC Field Count

//array error message type
enum : byte {
	ANALYSE_OK,
	ERREUR_FORMAT,
	ERREUR_DONNEE_TROP_LONGUE
} erreur;



#include <MySensors.h> // include MySensors library

// Define child sensor
MyMessage VFFMsg(0, S_INFO);
MyMessage V181Msg(1, V_KWH);
MyMessage V182Msg(2, V_KWH);
MyMessage V881Msg(3, V_VAR);
MyMessage V882Msg(4, V_VAR);
MyMessage V180Msg(5, V_KWH);
MyMessage V880Msg(6, V_VAR);
MyMessage VC70Msg(7, V_VAR1);
MyMessage V327Msg(8, V_VA);
MyMessage V527Msg(9, V_VA);
MyMessage V727Msg(10, V_VA);
MyMessage V317Msg(11, V_VA);
MyMessage V517Msg(12, V_VA);
MyMessage V717Msg(13, V_VA);
MyMessage V337Msg(14, V_POWER_FACTOR);
MyMessage V537Msg(15, V_POWER_FACTOR);
MyMessage V737Msg(16, V_POWER_FACTOR);
MyMessage V137Msg(17, V_POWER_FACTOR);
MyMessage V8281Msg(18, V_VAR2);
MyMessage V8282Msg(19, V_VAR3);


void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Landis & Gyr E350 Energy Meter", "1.0");

	present(0, S_CUSTOM);
	present(1, S_POWER);
	present(2, S_POWER);
	present(3, S_POWER);
	present(4, S_POWER);
	present(5, S_POWER);
	present(6, S_POWER);
	present(7, S_CUSTOM);
	present(8, S_POWER);
	present(9, S_POWER);
	present(10, S_POWER);
	present(11, S_POWER);
	present(12, S_POWER);
	present(13, S_POWER);
	present(14, S_POWER);
	present(15, S_POWER);
	present(16, S_POWER);
	present(17, S_POWER);
	present(18, S_CUSTOM);
	present(19, S_CUSTOM);

}



void setup() {

	// initialize serial:
	Serial.begin(SERIALSPEED);
	Serial1.begin(IRSERIALSPEED, IRSERIALCONF);

	// reserve 200 bytes for the inputString:
	inputString.reserve(200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
}



void loop() {

	currentTime = millis(); // Update time

	// print the string when a newline arrives:
	if (stringComplete) {
		Serial.print(" RECEIVE : ");
		Serial.print(inputString);
		// clear the string:
		inputString = "";
		stringComplete = false;
	}

	if (currentTime - lastread > readFrequency) {

		if (poll == false) {
			Serial1.begin(IRSERIALSPEED, IRSERIALCONF);
			delay(1000);
			Serial.println("IR SEND : /?!<CR><LF>\r\n");
			Serial1.write(0xAF); // "/"  /
			Serial1.write(0x3F); // "?"  ?
			Serial1.write(0x21); // "!"  !
			Serial1.write(0x8D); // "\r" CR
			Serial1.write(0x0A); // "\n" LF

			poll = true;
		}

		if (ack == true) {
			Serial1.begin(IRSERIALSPEED, IRSERIALCONF);
			delay(1000);
			Serial.print(millis() / 1000);
			Serial.println(F("IR SEND : <ACK>000<CR><LF>\r\n"));
			Serial1.write(0x06); // "<ACK>"
			Serial1.write(0x30); // "0"
			Serial1.write(0x30); // "0" 300 "1" 600
			Serial1.write(0x30); // "0"
			Serial1.write(0x8D); // "\r" CR
			Serial1.write(0x0A); // "\n" LF
			ack = false;
		}
	}

	if (currentTime - lastSend > sendFrequency) {

		lastSend = currentTime; // update send frequency timer

		//send values to gateway
		send(VFFMsg.set(VFF.toFloat(), 0));
		send(V181Msg.set(V181.toFloat(), 3));
		send(V182Msg.set(V182.toFloat(), 3));
		send(V881Msg.set(V881.toFloat(), 3));
		send(V882Msg.set(V882.toFloat(), 3));
		send(V180Msg.set(V180.toFloat(), 3));
		send(V880Msg.set(V880.toFloat(), 3));
		send(VC70Msg.set(VC70.toFloat(), 0));
		send(V327Msg.set(V327.toFloat(), 3));
		send(V527Msg.set(V527.toFloat(), 3));
		send(V727Msg.set(V727.toFloat(), 3));
		send(V317Msg.set(V317.toFloat(), 3));
		send(V517Msg.set(V517.toFloat(), 3));
		send(V717Msg.set(V717.toFloat(), 3));
		send(V337Msg.set(V337.toFloat(), 3));
		send(V537Msg.set(V537.toFloat(), 3));
		send(V737Msg.set(V737.toFloat(), 3));
		send(V137Msg.set(V137.toFloat(), 3));
		send(V8281Msg.set(V8281.toFloat(), 0));
		send(V8282Msg.set(V8282.toFloat(), 0));
	}
}



// IR Serial event
void serialEvent1() {

	while (Serial1.available()) {

		// get the new byte:
		char inChar = (char)Serial1.read();

		/* // Uncomment for debugging
		Serial.print(F(") ASCII : "));
		Serial.println(inChar);
		*/

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
			Serial1.flush();
			Serial1.end();
		}

		// if the incoming character is a newline, set a flag so the main loop can
		// do something about it:
		if (inChar == '\n') {
			stringComplete = true;
			content = content + inputString;

		}

		if (inputString == "/LGZ4ZMF100AC.M29\r\n") {
			ack = true;
		}

		if (clearserial == true) {
			while (Serial1.available() > 0) { char trash = Serial1.read(); }

			inputString = "";

			decoding();
			clearserial = false;
		}
	}
}



void decoding() {

	//convert to decode
	//Serial.println(content); // Uncomment for debugging

	int str_len = content.length() + 1;
	char ElectricMeterResponse[str_len];
	content.toCharArray(ElectricMeterResponse, str_len);

	// decode and arrange
	decodeirserial(ElectricMeterResponse);

	// Clear received buffer
	content = "";
}



void decodeirserial(char *ElectricMeterResponse) {

	while (*ElectricMeterResponse) {

		if (strcmp(ElectricMeterResponse, "!\r\n") == 0) break; // END OF MESSAGE

		// -------------------------------------------------------
		// FIND SEPARATORS AND A FEW CHECKS
		// -------------------------------------------------------

		char * parentheseOpen = strchr(ElectricMeterResponse, '(');
		char * parentheseClose = strchr(ElectricMeterResponse, ')');
		char * star = strchr(ElectricMeterResponse, '*');
		char * endOfLine = strchr(ElectricMeterResponse, '\n');

		String decodage = ElectricMeterResponse;
		decodage.trim();

		//Serial.print(F("decodage of : ")); // Uncomment for debugging
		//Serial.println(decodage); // Uncomment for debugging

		if ((parentheseOpen == NULL) ||
			(parentheseClose == NULL) ||
			(endOfLine == NULL) ||
			(parentheseOpen > parentheseClose))
		{
			Serial.print("ERROR FORMAT : ");
			Serial.println(ElectricMeterResponse);

			break;
		}

		nbOfCharacter = parentheseOpen - ElectricMeterResponse;

		if (nbOfCharacter > maxData) {
			Serial.println("ERROR DATA TOO LONG");
			break;
		}

		strncpy(label, ElectricMeterResponse, nbOfCharacter);
		label[nbOfCharacter] = '\0';

		// -------------------------------------------------------
		// JUMP THE OPENING PARENTHESE AND READ THE DATA
		// -------------------------------------------------------

		ElectricMeterResponse = parentheseOpen + 1;

		// THE DATA END EITHER BY A FERMENTED PARENTHESE, OR BY A *

		if ((star == NULL) || (star > endOfLine)) {
			// We are in the case where there are no units
			nbOfCharacter = parentheseClose - ElectricMeterResponse;

			if (nbOfCharacter > maxData) {
				Serial.println("ERROR DATA TOO LONG");
				break;
			}

			strncpy(value, ElectricMeterResponse, nbOfCharacter);
			value[nbOfCharacter] = '\0';

			unit[0] = '\0'; // no unit

		}
		else {
			// unit
			nbOfCharacter = star - ElectricMeterResponse;

			if (nbOfCharacter > maxData) {
				Serial.println("ERROR DATA TOO LONG");
				break;
			}

			strncpy(value, ElectricMeterResponse, nbOfCharacter);
			value[nbOfCharacter] = '\0';

			// Jump *
			ElectricMeterResponse = star + 1;

			nbOfCharacter = parentheseClose - ElectricMeterResponse;

			if (nbOfCharacter > maxData) {
				Serial.println("ERROR DATA TOO LONG");
				break;
			}

			strncpy(unit, ElectricMeterResponse, nbOfCharacter);
			unit[nbOfCharacter] = '\0';
		}

		// We move to the next field
		ElectricMeterResponse = endOfLine + 1;

		//Serial.print(F(" label = \"")); Serial.print(label); // Uncomment for debugging

		if ((String)label == "F.F") { VFF = (String)value; }
		if ((String)label == "1.8.1") { V181 = (String)value; }
		if ((String)label == "1.8.2") { V182 = (String)value; }
		if ((String)label == "8.8.1") { V881 = (String)value; }
		if ((String)label == "8.8.2") { V882 = (String)value; }
		if ((String)label == "1.8.0") { V180 = (String)value; }
		if ((String)label == "8.8.0") { V880 = (String)value; }
		if ((String)label == "C.7.0") { VC70 = (String)value; }
		if ((String)label == "32.7") { V327 = (String)value; }
		if ((String)label == "52.7") { V527 = (String)value; }
		if ((String)label == "72.7") { V727 = (String)value; }
		if ((String)label == "31.7") { V317 = (String)value; }
		if ((String)label == "51.7") { V517 = (String)value; }
		if ((String)label == "71.7") { V717 = (String)value; }
		if ((String)label == "33.7") { V337 = (String)value; }
		if ((String)label == "53.7") { V537 = (String)value; }
		if ((String)label == "73.7") { V737 = (String)value; }
		if ((String)label == "13.7") { V137 = (String)value; }
		if ((String)label == "82.8.1") { V8281 = (String)value; }
		if ((String)label == "82.8.2") {
			V8282 = (String)value;

			/* Uncomment for debugging
			Serial.print("FF : ");
			Serial.println(VFF);
			Serial.print("V181 : ");
			Serial.println(V181);
			Serial.print("V182 : ");
			Serial.println(V182);
			Serial.print("V881 : ");
			Serial.println(V881);
			Serial.print("V882 : ");
			Serial.println(V882);
			Serial.print("V180 : ");
			Serial.println(V180);
			Serial.print("V880 : ");
			Serial.println(V880);
			Serial.print("VC70 : ");
			Serial.println(VC70);
			Serial.print("V327 : ");
			Serial.println(V327);
			Serial.print("V527 : ");
			Serial.println(V527);
			Serial.print("V727 : ");
			Serial.println(V727);
			Serial.print("V317 : ");
			Serial.println(V317);
			Serial.print("V517 : ");
			Serial.println(V517);
			Serial.print("V717 : ");
			Serial.println(V717);
			Serial.print("V337 : ");
			Serial.println(V337);
			Serial.print("V537 : ");
			Serial.println(V537);
			Serial.print("V737 : ");
			Serial.println(V737);
			Serial.print("V137 : ");
			Serial.println(V137);
			Serial.print("V8281 : ");
			Serial.println(V8281);
			Serial.print("V8282 : ");
			Serial.println(V8282);
			*/

			//send values to gateway
			send(VFFMsg.set(VFF.toFloat(), 0));
			send(V181Msg.set(V181.toFloat(), 3));
			send(V182Msg.set(V182.toFloat(), 3));
			send(V881Msg.set(V881.toFloat(), 3));
			send(V882Msg.set(V882.toFloat(), 3));
			send(V180Msg.set(V180.toFloat(), 3));
			send(V880Msg.set(V880.toFloat(), 3));
			send(VC70Msg.set(VC70.toFloat(), 0));
			send(V327Msg.set(V327.toFloat(), 3));
			send(V527Msg.set(V527.toFloat(), 3));
			send(V727Msg.set(V727.toFloat(), 3));
			send(V317Msg.set(V317.toFloat(), 3));
			send(V517Msg.set(V517.toFloat(), 3));
			send(V717Msg.set(V717.toFloat(), 3));
			send(V337Msg.set(V337.toFloat(), 3));
			send(V537Msg.set(V537.toFloat(), 3));
			send(V737Msg.set(V737.toFloat(), 3));
			send(V137Msg.set(V137.toFloat(), 3));
			send(V8281Msg.set(V8281.toFloat(), 0));
			send(V8282Msg.set(V8282.toFloat(), 0));

		}

		//Serial.print(F("\", value = \"")); Serial.print(value); // Uncomment for debugging

		if (unit[0] != '\0') { // si on avait des unités
		  //Serial.print(F("\", unit = \"")); Serial.print(unit); Serial.println(F("\"")); // Uncomment for debugging
		}
	} // End of while

	lastread = currentTime; // update send frequency timer
}
