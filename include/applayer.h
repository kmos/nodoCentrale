
/*######OPCODE##########*/
typedef uint8_t OpCode;
#define READDATA 		((uint8_t)0x00) 		//invia misurazione al centro di controllo 		###	Centro Controllo -> Nodo Sensore
#define CONFIGSENSOR 	((uint8_t)0x01) 		//configurazione sensore 						###	Centro Controllo -> Nodo Sensore
#define DATA 			((uint8_t)0x02)			//dato di misurazione							###	Nodo Sensore -> Centro Controllo
/*#####################*/


typedef struct payload{
	uint8_t id; 		//id sensore
	int16_t val;		//valore
	uint8_t alarm;		//allarme
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

//Pacchetto Centro controllo <-> Nodo Centrale
typedef struct Nodepack{
	uint64_t id0;//è necessario?
	uint32_t id1;
	uint16_t address;
	uint64_t secret0;
	uint64_t secret1;
	APPpackage pack;
};


//id a 12 byte
//indirizzi a 16 bit
