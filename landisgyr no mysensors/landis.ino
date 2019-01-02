/* Serial setup */
#define SERIALSPEED		9600

/* Ir setup */
#define IRSERIALSPEED	300
#define IRSERIALCONF	SERIAL_7E1

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

bool poll = false;
bool ack = false;
bool clearserial = false;

String content = "";
char response;
const byte maxData = 30;
char etiquette[maxData + 1];
char valeur[maxData + 1];
char unite[maxData + 1];
char message[] = "";
char * debutPtr;

String VFF = "00";   // Error code (always first in the list)
String V181 = "11.096";  // Active energy import rate 1 (X = 1…6)
String V182 = "19.999";  // Active energy import rate 2 (X = 1…6)
String V881 = "7.226";  // kvarh rate 1
String V882 = "5.419";  // kvarh rate 2
String V180 = "31.096";  // Total active energy import
String V880 = "12.645";  // for the Reactive Energy Q4 Billing Totals
String VC70 = "6";  // Power Fail Count
String V327 = "236";  // VRMS Phase L1 (L12 in ZFFxxx meters)
String V527 = "235";  // VRMS Phase L2 (not for 3-wire ZFFxxx meters)
String V727 = "237";  // VRMS Phase L3 (L32 in ZFFxxx meters)
String V317 = "0.139";  // IRMS Phase L1
String V517 = "1.60";  // IRMS Phase L2 (not for 3-wire ZFFxxx meters)
String V717 = "0.457";  // IRMS Phase L3
String V337 = "0.16";  // Power factor phase L1 (4-wire ZMF1xx meters only)
String V537 = "0.90";  // Power factor phase L2 (4-wire ZMF1xx meters only)
String V737 = "0.65";  // Power factor phase L3 (4-wire ZMF1xx meters only)
String V137 = "0.83";  // Power Factor Phase Summation
String V8281 = "1"; // Terminal cover removal counter
String V8282 = "0"; // DC Field Count

enum : byte { ANALYSE_OK, ERREUR_FORMAT, ERREUR_DONNEE_TROP_LONGUE } erreur;

size_t nbCaracteres;


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
	// print the string when a newline arrives:
	if (stringComplete) {
		Serial.print("receive : ");
		Serial.print(inputString);
		// clear the string:
		inputString = "";
		stringComplete = false;
	}

	if (poll == false) {
		Serial1.begin(IRSERIALSPEED, IRSERIALCONF);
		delay(1000);
		Serial.println("send : /?!<CR><LF>\r\n");
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
		Serial.println(F("send : <ACK>000<CR><LF>\r\n"));
		Serial1.write(0x06); // "<ACK>"
		Serial1.write(0x30); // "0"
		Serial1.write(0x30); // "0" 300 "1" 600
		Serial1.write(0x30); // "0"
		Serial1.write(0x8D); // "\r" CR
		Serial1.write(0x0A); // "\n" LF
		ack = false;
	}

	//delay(5000);
}

// IR Serial event
void serialEvent1() {
	while (Serial1.available()) {
		// get the new byte:
		char inChar = (char)Serial1.read();
		/*
		Serial.print(F("DEBUG : int("));
		Serial.print((int)inChar);
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

		}

		//while (Serial1.available()) {Serial1.read();}

		if (inputString == "/LGZ4ZMF100AC.M29\r\n") {
			ack = true;
		}


		if (clearserial == true) {
			while (Serial1.available() > 0) { char trash = Serial1.read(); }
			content = inputString;
			inputString = "";
			decoding();

			clearserial = false;
		}
	}
}


void decoding() {
	//convert to decode
	int str_len = content.length() + 1;
	char debutPtr[str_len];
	content.toCharArray(debutPtr, str_len);

	// decode and arrange
	decodeirserial(debutPtr);

	// Clear received buffer
	content = "";
}


void decodeirserial(char *debutPtr) {

	while (*debutPtr) {

		if (strcmp(debutPtr, "!\r\ne") == 0) break; // FIN DU MESSAGE

		// -------------------------------------------------------
		// ON TROUVE LES SEPARATEURS ET ON FAIT QUELQUES VERIFS
		// -------------------------------------------------------

		char * parentheseOuvrante = strchr(debutPtr, '(');
		char * parentheseFermante = strchr(debutPtr, ')');
		char * etoile = strchr(debutPtr, '*');
		char * finDeLigne = strchr(debutPtr, '\n');

		String decodage = debutPtr;
		decodage.trim();
		Serial.print(F("decodage de : "));
		Serial.println(decodage);

		if ((parentheseOuvrante == NULL) ||
			(parentheseFermante == NULL) ||
			(finDeLigne == NULL) ||
			(parentheseOuvrante > parentheseFermante))
		{
			Serial.print("ERREUR_FORMAT reçu : ");
			Serial.println(debutPtr);

			break;
		}

		nbCaracteres = parentheseOuvrante - debutPtr;

		if (nbCaracteres > maxData) {
			erreur = ERREUR_DONNEE_TROP_LONGUE;
			Serial.println("ERREUR_DONNEE_TROP_LONGUE");
			break;
		}

		strncpy(etiquette, debutPtr, nbCaracteres);
		etiquette[nbCaracteres] = '\0';

		// -------------------------------------------------------
		// ON SAUTE LA PARENTHESE OUVRANTE, ON LIT LES DONNEES
		// -------------------------------------------------------

		debutPtr = parentheseOuvrante + 1;

		// LES DONNES SE TERMINENT SOIT PAR UNE PARENTHESE FERMANTE, SOIT PAR UNE *

		if ((etoile == NULL) || (etoile > finDeLigne)) {
			// On est dans le cas où il n'y as pas d'unités
			nbCaracteres = parentheseFermante - debutPtr;

			if (nbCaracteres > maxData) {
				erreur = ERREUR_DONNEE_TROP_LONGUE;
				Serial.println("ERREUR_DONNEE_TROP_LONGUE");
				break;
			}

			strncpy(valeur, debutPtr, nbCaracteres);
			valeur[nbCaracteres] = '\0';

			unite[0] = '\0'; // pas d'unités

		}
		else {
			// il y a des unités
			nbCaracteres = etoile - debutPtr;

			if (nbCaracteres > maxData) {
				erreur = ERREUR_DONNEE_TROP_LONGUE;
				Serial.println("ERREUR_DONNEE_TROP_LONGUE");
				break;
			}

			strncpy(valeur, debutPtr, nbCaracteres);
			valeur[nbCaracteres] = '\0';

			// on saute l'étoile
			debutPtr = etoile + 1;

			nbCaracteres = parentheseFermante - debutPtr;

			if (nbCaracteres > maxData) {
				erreur = ERREUR_DONNEE_TROP_LONGUE;
				Serial.println("ERREUR_DONNEE_TROP_LONGUE");
				break;
			}

			strncpy(unite, debutPtr, nbCaracteres);
			unite[nbCaracteres] = '\0';
		}

		// On passe au champs suivant
		debutPtr = finDeLigne + 1;
		Serial.print(F("ETIQUETTE = \"")); Serial.print(etiquette);



		if ((String)etiquette == "F.F") { VFF = (String)valeur; }
		if ((String)etiquette == "1.8.1") { V181 = (String)valeur; }
		if ((String)etiquette == "1.8.2") { V182 = (String)valeur; }
		if ((String)etiquette == "8.8.1") { V881 = (String)valeur; }
		if ((String)etiquette == "8.8.2") { V882 = (String)valeur; }
		if ((String)etiquette == "1.8.0") { V180 = (String)valeur; }
		if ((String)etiquette == "8.8.0") { V880 = (String)valeur; }
		if ((String)etiquette == "C.7.0") { VC70 = (String)valeur; }
		if ((String)etiquette == "32.7") { V327 = (String)valeur; }
		if ((String)etiquette == "52.7") { V527 = (String)valeur; }
		if ((String)etiquette == "72.7") { V727 = (String)valeur; }
		if ((String)etiquette == "31.7") { V317 = (String)valeur; }
		if ((String)etiquette == "51.7") { V517 = (String)valeur; }
		if ((String)etiquette == "71.7") { V717 = (String)valeur; }
		if ((String)etiquette == "33.7") { V337 = (String)valeur; }
		if ((String)etiquette == "53.7") { V537 = (String)valeur; }
		if ((String)etiquette == "73.7") { V737 = (String)valeur; }
		if ((String)etiquette == "13.7") { V137 = (String)valeur; }
		if ((String)etiquette == "82.8.1") { V8281 = (String)valeur; }
		if ((String)etiquette == "82.8.2") {
			V8282 = (String)valeur;


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

		}

		Serial.print(F("\", VALEUR = \"")); Serial.print(valeur);

		if (unite[0] != '\0') { // si on avait des unités
			Serial.print(F("\", UNITE = \"")); Serial.print(unite);
		}
	} // FIN DU WHILE
}