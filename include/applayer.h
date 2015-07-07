
/*######OPCODE##########*/
typedef uint8_t OpCode;
#define READDATA 		((uint8_t)0x00) 		//invia misurazione al centro di controllo 					###	Centro Controllo -> Nodo Sensore
#define CONFIGSENSOR 	((uint8_t)0x01) 		//configurazione sensore 									###	Centro Controllo -> Nodo Sensore
#define DATA 			((uint8_t)0x02)			//dato di misurazione										###	Nodo Sensore -> Centro Controllo
#define JOIN			((uint8_t)0x03)			//join nodo, fornisce la chiave personale del nodo e l'id	### Centro Controllo <- Nodo centrale
#define CANJOIN			((uint8_t)0x04)			//canjoin risponde al nodo con la chiave condivisa			### Centro Controllo -> Nodo Centrale
/*#####################*/

#define NODOCENTRALE

typedef struct payload{
	uint8_t id; 		//id sensore
	int16_t val;		//valore
	uint8_t alarm;		//allarme
	int64_t timestamp;
	int32_t ht;			//high th.shold
	int32_t lt;			//low  th.shold
	int16_t period;		//periodo
	int8_t  priority;	//priorità
}Payload;

//128 bit application package

typedef struct aPPpackage{
	OpCode 	code;
	Payload	payload;
} APPpackage;



#ifdef 	NODOCENTRALE
//###################################################
//Pacchetti Centro controllo <-> Nodo Centrale
//##################################################

//struttura dati in ricezione al centro di controllo
typedef struct nodepack{
	uint16_t address;
	APPpackage pack;
} Nodepack;

//messaggio sicurezza join <->
typedef struct secpack{
	uint64_t id0;	//è necessario?
	uint32_t id1;
	uint64_t secret0;
	uint64_t secret1;
} Secpack;

//messaggio richiesta dati dal sensore ->
typedef struct getdata{
	uint16_t address;
	uint8_t sensor;
} Getdata;

#endif

//id a 12 byte
//indirizzi a 16 bit
