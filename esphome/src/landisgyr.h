#include "esphome.h"

class landisgyrComponent : public PollingComponent, public Sensor public Component, public uart::UARTDevice {

	protected:

		// Analyse variables
		const byte maxData = 30;
		char etiquette[30];
		char valeur[30];
		char unite[30];
		char message[300];
		char * myPointer;
		char * myPointerCrc;
		
		enum : byte { ANALYSE_OK, ERREUR_FORMAT, ERREUR_DONNEE_TROP_LONGUE } erreur;

		size_t nbCaracteres;
		
		int str_len_decode = 0;

		// Define flag
		bool unitFlag = false;
		bool firmwareVersionFlag = false;
		bool ackFlag = false;
		bool etxFlag = false;
		bool checksum = false;

		// buffer for crc, serial, decoding
		String serialBuffer = "";
		String meterIdBuffer = "";

		char response;
		char receivedChecksum;
		int str_len_crc = 0;

		// Constant
		const char NUL    = 0; // null
		const char SOH    = 1; // start of heading
		const char STX    = 2; // start of text
		const char ETX    = 3; // end of text
		const char EOT    = 4; // end of transmission
		const char ENQ    = 5; // enquiry
		const char ACK    = 6; // acknowledge
		const char BEL    = 7; // bell
		const char BS     = 8; // backspace
		const char TAB    = 9; // horizontal tab
		const char LF     = 10; // NL line feed
		const char VT     = 11; // vertical tab
		const char FF     = 12; // NP form feed
		const char CR     = 13; // carriage return
		const char SO     = 14; // shift out
		const char SI     = 15; // shift in
		const char DLE    = 16; // data link escape
		const char DC1    = 17; // device control 1
		const char DC2    = 18; // device control 2
		const char DC3    = 19; // device control 3
		const char DC4    = 20; // device control 4
		const char NAK    = 21; // negative acknowledge
		const char SYN    = 22; // synchronius idle
		const char ETB    = 23; // end of trans. block
		const char CAN    = 24; // cancel
		const char EM     = 25; // end of medium
		const char SUB    = 26; // substitute
		const char ESC    = 27; // escape
		const char FS     = 28; // file separator
		const char GS     = 29; // group separator
		const char RS     = 30; // record separator
		const char US     = 31; // unit separator
		const char POINT  = 46; // . point
		const char SLASH  = 47; // / slash
		const char HELP   = 63; // ? help

	public:

		// constructor
		landisgyrComponent( UARTComponent *parent) : PollingComponent(15000), UARTDevice(parent) {}

		// Define sensor's

		// Special
		Sensor *errorCode = new Sensor(); 							// F.F		Error code
		
		Sensor *customerIdentification = new Sensor(); 				// 0.0 		Customer identification
		Sensor *firmwareVersion = new Sensor(); 					// 0.2.0 	Firmware Version

		Sensor *meterId = new Sensor(); 							// C.1.0 	Meter ID
		Sensor *manufacturingId = new Sensor(); 					// C.1.1 	Manufacturing ID
		Sensor *statusFlag = new Sensor(); 							// C.5.0 	Status Flag
		Sensor *eventPowerDownCounter = new Sensor(); 				// C.7.0 	Event power down - counter

		// Security
		Sensor *terminalCoverRemovalCounter = new Sensor(); 		// 82.8.1  	Terminal cover removal counter
		Sensor *dcFieldCount = new Sensor(); 						// 82.8.2  	DC Field Count

		// Positive active energy
		Sensor *positiveActiveEnergyTot = new Sensor();				// 1.8.0 	Positive active energy (A+) total [kWh]
		Sensor *positiveActiveEnergy1 = new Sensor();				// 1.8.1 	Positive active energy (A+) in tariff T1 [kWh]
		Sensor *positiveActiveEnergy2 = new Sensor();				// 1.8.2 	Positive active energy (A+) in tariff T2 [kWh]

		// Negative active energy
		Sensor *negativeActiveEnergyTot = new Sensor();				// 2.8.0 	Negative active energy (A+) total [kWh]
		Sensor *negativeActiveEnergy1 = new Sensor();				// 2.8.1 	Negative active energy (A+) in tariff T1 [kWh]
		Sensor *negativeActiveEnergy2 = new Sensor();				// 2.8.2 	Negative active energy (A+) in tariff T2 [kWh]

		// Imported energy
		Sensor *importedInductiveReactiveEnergyTot = new Sensor();	// 5.8.0 	Imported inductive reactive energy in 1-st quadrant (Q1) total [kvarh]
		Sensor *importedInductiveReactiveEnergy1 = new Sensor();	// 5.8.1 	Imported inductive reactive energy in 1-st quadrant (Q1) in tariff T1 [kvarh]
		Sensor *importedInductiveReactiveEnergy2 = new Sensor();	// 5.8.2 	Imported inductive reactive energy in 1-st quadrant (Q1) in tariff T2 [kvarh]

		Sensor *importedCapacitiveReactiveEnergyTot = new Sensor();	// 6.8.0 	Imported capacitive reactive energy in 2-nd quadrant (Q2) total [kvarh]
		Sensor *importedCapacitiveReactiveEnergy1 = new Sensor();	// 6.8.1 	Imported capacitive reactive energy in 2-nd quadr. (Q2) in tariff T1 [kvarh]
		Sensor *importedCapacitiveReactiveEnergy2 = new Sensor();	// 6.8.2 	Imported capacitive reactive energy in 2-nd quadr. (Q2) in tariff T2 [kvarh]

		// exported energy
		Sensor *exportedInductiveReactiveEnergyTot = new Sensor();	// 7.8.0 	Exported inductive reactive energy in 3-rd quadrant (Q3) total [kvarh]
		Sensor *exportedInductiveReactiveEnergy1 = new Sensor();	// 7.8.1 	Exported inductive reactive energy in 3-rd quadrant (Q3) in tariff T1 [kvarh]
		Sensor *exportedInductiveReactiveEnergy2 = new Sensor();	// 7.8.2 	Exported inductive reactive energy in 3-rd quadrant (Q3) in tariff T2 [kvarh]
		Sensor *exportedCapacitiveReactiveEnergyTot = new Sensor();	// 8.8.0 	Exported capacitive reactive energy in 4-th quadrant (Q4) total [kvarh]
		Sensor *exportedCapacitiveReactiveEnergy1 = new Sensor();	// 8.8.1 	Exported capacitive reactive energy in 4-th quadr. (Q4) in tariff T1 [kvarh]
		Sensor *exportedCapacitiveReactiveEnergy2 = new Sensor();	// 8.8.2 	Exported capacitive reactive energy in 4-th quadr. (Q4) in tariff T2 [kvarh]

		// Voltages
		Sensor *instantaneousVoltageP1 = new Sensor();				// 32.7  	Instantaneous voltage (U) in phase L1 [V]
		Sensor *instantaneousVoltageP2 = new Sensor();				// 52.7  	Instantaneous voltage (U) in phase L2 [V]
		Sensor *instantaneousVoltageP3 = new Sensor();				// 72.7  	Instantaneous voltage (U) in phase L3 [V]

		// Current
		Sensor *instantaneousCurrentP1 = new Sensor();				// 31.7  	Instantaneous current (I) in phase L1 [A]
		Sensor *instantaneousCurrentP2 = new Sensor();				// 51.7  	Instantaneous current (I) in phase L2 [A]
		Sensor *instantaneousCurrentP3 = new Sensor();				// 71.7  	Instantaneous current (I) in phase L3 [A]
		
		// Power factor
		Sensor *instantaneousPowerFactor = new Sensor();			// 13.7  	Instantaneous power factor
		Sensor *instantaneousPowerFactorP1 = new Sensor();			// 33.7  	Instantaneous power factor in phase L1
		Sensor *instantaneousPowerFactorP2 = new Sensor();			// 53.7  	Instantaneous power factor in phase L2
		Sensor *instantaneousPowerFactorP3 = new Sensor();			// 73.7  	Instantaneous power factor in phase L3



	void setup() override {
	// nothing to do here
	}



	void loop() override {

		// While ir serial
		while (available()) {
			
			// Read serial buffer
			response = read();
			
			if (unitFlag == true ){

				// Reset flag for next round
				unitFlag = false;

				ESP_LOGD("HEX", "unitFlag == true");
				
				// Reset serial buffer
				serialBuffer = "";

			}

			if (response == POINT) {

				unitFlag = false;
				meterIdBuffer = serialBuffer;

				ESP_LOGD("CRC", "Meter ID : %s", String(meterIdBuffer));
			}

			if (response == SLASH){	unitFlag = true; }
			
			if (ackFlag == true) {

				// Reset flag for next round
				ackFlag = false;
				
				ESP_LOGD("HEX", "response == STX");
				// Reset serial buffer
				serialBuffer = "";
			}

			// Received <STX>
			if (response == STX) { 
				ackFlag = true; 
			}
			
			if (etxFlag == true) {
				
				// Reset flag for next round
				ackFlag = false;
				etxFlag = false;

				// Store received checksum
				receivedChecksum = response;
				
				// Checksum calculation
				str_len_crc = serialBuffer.length() + 1;
				char myPointerCrc[str_len_crc];
				serialBuffer.toCharArray(myPointerCrc, str_len_crc);

				checksum = crcCalculation((uint8_t *)myPointerCrc,sizeof(myPointerCrc));

				if (checksum == true) {

					// Decoding values
					str_len_decode = serialBuffer.length() + 1;
					char myPointer[str_len_decode];
					serialBuffer.toCharArray(myPointer, str_len_decode);

					decodeIrSerial(myPointer);

				} else {

					ESP_LOGD("ERR", "BAD CHECKSUM");
				}
			}

			// Received <ETX>
			if(response == ETX) { etxFlag = true; }

			// Add response to serial buffer
			serialBuffer = serialBuffer + response;

			// Debug
			ESP_LOGD("HEX", " 0x%02X", response);

		} 
	}

	// Calculate checksum
	bool crcCalculation(byte myArray[], int myArraySize)
	{

		byte crc = 0; // reset crc

		for (int i = 0; i < myArraySize; i++) {
			ESP_LOGD("CRC", " 0x%02X", myArray[i]);
			crc ^= myArray[i];
		}

		ESP_LOGD("CRC", "Received CRC 0x%02X calculated CRC 0x%02X", receivedChecksum, crc);

		if (receivedChecksum == crc) {
			ESP_LOGD("CRC", "CHECKSUM OK");
			return true;
		} else {
			ESP_LOGD("CRC", "CHECKSUM KO");
			return false;
		}

	}

	void decodeIrSerial(char *myPointer) {

		while (*myPointer) {

			if (strcmp(myPointer, "!\r\n") == 0) break; // FIN DU MESSAGE
		
			// -------------------------------------------------------
			// ON TROUVE LES SEPARATEURS ET ON FAIT QUELQUES VERIFS
			// -------------------------------------------------------

			char * parentheseOuvrante = strchr(myPointer, '(');
			char * parentheseFermante = strchr(myPointer, ')');
			char * etoile = strchr(myPointer, '*');
			char * finDeLigne = strchr(myPointer, '\n');

			String decodage = myPointer;
			decodage.trim();
				

			if ((parentheseOuvrante == NULL) ||
				(parentheseFermante == NULL) ||
				(finDeLigne == NULL) ||
				(parentheseOuvrante > parentheseFermante))
			{

				break;
			}

			nbCaracteres = parentheseOuvrante - myPointer;

			if (nbCaracteres > maxData) {
				erreur = ERREUR_DONNEE_TROP_LONGUE;
				break;
			}

			strncpy(etiquette, myPointer, nbCaracteres);
			etiquette[nbCaracteres] = '\0';

			// -------------------------------------------------------
			// ON SAUTE LA PARENTHESE OUVRANTE, ON LIT LES DONNEES
			// -------------------------------------------------------

			myPointer = parentheseOuvrante + 1;

			// LES DONNES SE TERMINENT SOIT PAR UNE PARENTHESE FERMANTE, SOIT PAR UNE *

			if ((etoile == NULL) || (etoile > finDeLigne)) {
				// On est dans le cas où il n'y as pas d'unités
				nbCaracteres = parentheseFermante - myPointer;

				if (nbCaracteres > maxData) {
					erreur = ERREUR_DONNEE_TROP_LONGUE;
					break;
				}

				strncpy(valeur, myPointer, nbCaracteres);
				valeur[nbCaracteres] = '\0';

				unite[0] = '\0'; // pas d'unités

			}
			else {
				// il y a des unités
				nbCaracteres = etoile - myPointer;

				if (nbCaracteres > maxData) {
					erreur = ERREUR_DONNEE_TROP_LONGUE;
					break;
				}

				strncpy(valeur, myPointer, nbCaracteres);
				valeur[nbCaracteres] = '\0';

				// on saute l'étoile
				myPointer = etoile + 1;

				nbCaracteres = parentheseFermante - myPointer;

				if (nbCaracteres > maxData) {
					erreur = ERREUR_DONNEE_TROP_LONGUE;
					break;
				}

				strncpy(unite, myPointer, nbCaracteres);
				unite[nbCaracteres] = '\0';
			}

			// On passe au champs suivant
			myPointer = finDeLigne + 1;
						
			if ((String)etiquette == "F.F") 	{ errorCode->publish_state(atof(valeur)); }
			if ((String)etiquette == "0.0") 	{ customerIdentification->publish_state(atof(valeur)); }
			if ((String)etiquette == "0.2.0") 	{ firmwareVersion->publish_state(atof(valeur)); }
			if ((String)etiquette == "C.1.0") 	{ meterId->publish_state(atof(valeur)); }
			if ((String)etiquette == "C.1.1") 	{ manufacturingId->publish_state(atof(valeur)); }
			if ((String)etiquette == "C.5.0") 	{ statusFlag->publish_state(atof(valeur)); }
			if ((String)etiquette == "C.7.0") 	{ eventPowerDownCounter->publish_state(atof(valeur)); }
			if ((String)etiquette == "82.8.1") 	{ terminalCoverRemovalCounter->publish_state(atof(valeur)); }
			if ((String)etiquette == "82.8.2") 	{ dcFieldCount->publish_state(atof(valeur)); }
			if ((String)etiquette == "1.8.0") 	{ positiveActiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "1.8.1") 	{ positiveActiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "1.8.2") 	{ positiveActiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "2.8.0") 	{ negativeActiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "2.8.1") 	{ negativeActiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "2.8.2") 	{ negativeActiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "5.8.0") 	{ importedInductiveReactiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "5.8.1") 	{ importedInductiveReactiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "5.8.2") 	{ importedInductiveReactiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "6.8.0") 	{ importedCapacitiveReactiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "6.8.1") 	{ importedCapacitiveReactiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "6.8.2") 	{ importedCapacitiveReactiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "7.8.0") 	{ exportedInductiveReactiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "7.8.1") 	{ exportedInductiveReactiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "7.8.2") 	{ exportedInductiveReactiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "8.8.0") 	{ exportedCapacitiveReactiveEnergyTot->publish_state(atof(valeur)); }
			if ((String)etiquette == "8.8.1") 	{ exportedCapacitiveReactiveEnergy1->publish_state(atof(valeur)); }
			if ((String)etiquette == "8.8.2") 	{ exportedCapacitiveReactiveEnergy2->publish_state(atof(valeur)); }
			if ((String)etiquette == "32.7") 	{ instantaneousVoltageP1->publish_state(atof(valeur)); }
			if ((String)etiquette == "52.7") 	{ instantaneousVoltageP2->publish_state(atof(valeur)); }
			if ((String)etiquette == "72.7") 	{ instantaneousVoltageP3->publish_state(atof(valeur)); }
			if ((String)etiquette == "31.7") 	{ instantaneousCurrentP1->publish_state(atof(valeur)); }
			if ((String)etiquette == "51.7") 	{ instantaneousCurrentP2->publish_state(atof(valeur)); }
			if ((String)etiquette == "71.7") 	{ instantaneousCurrentP3->publish_state(atof(valeur)); }
			if ((String)etiquette == "13.7") 	{ instantaneousPowerFactorP1->publish_state(atof(valeur)); }
			if ((String)etiquette == "33.7") 	{ instantaneousPowerFactorP2->publish_state(atof(valeur)); }
			if ((String)etiquette == "53.7") 	{ instantaneousPowerFactorP3->publish_state(atof(valeur)); }
			if ((String)etiquette == "73.7") 	{ instantaneousPowerFactor->publish_state(atof(valeur)); }
		}
	}
};