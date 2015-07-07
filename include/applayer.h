
/*######OPCODE##########*/
typedef uint8_t OpCode;
#define READDATA 		((uint8_t)0x00) 		//invia misurazione al centro di controllo 		###	Centro Controllo -> Nodo Sensore
#define CONFIGSENSOR 	((uint8_t)0x01) 		//configurazione sensore 						###	Centro Controllo -> Nodo Sensore
#define DATA 			((uint8_t)0x02)			//dato di misurazione							###	Nodo Sensore -> Centro Controllo
/*#####################*/

typedef struct payload{
	uint8_t ID;
	int32_t valore;
	uint8_t alarm;
	int32_t High_Threashold;
	int32_t Low_Threashold;
	int16_t periodo;
	int8_t  priorità;
}Payload;

typedef struct aPPpackage{

	OpCode 	code;
	Payload	payload;
} APPpackage;


