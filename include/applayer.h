#ifndef INCLUDE_APPLAYER_H_
#define INCLUDE_APPLAYER_H_

#include <stdint.h>

/*######OPCODE##########*/
typedef uint8_t OpCode;
#define READDATA     ((uint8_t)0x00) 		//invia misurazione al centro di controllo 					###	Centro Controllo -> Nodo Sensore
#define CONFIGSENSOR ((uint8_t)0x01) 		//configurazione sensore 									###	Centro Controllo -> Nodo Sensore
#define DATA         ((uint8_t)0x02)			//dato di misurazione										###	Nodo Sensore -> Centro Controllo
#define CANJOIN      ((uint8_t)0x03)			//join nodo, fornisce la chiave personale del nodo e l'id	### Centro Controllo <- Nodo centrale
#define CANJOINREPLY ((uint8_t)0x04)			//replyjoin risponde al nodo con la chiave condivisa			### Centro Controllo -> Nodo Centrale
#define JOIN         ((uint8_t)0x05)
/*#####################*/




typedef struct payload{
	uint8_t id; 		//id sensore
	int16_t val;		//valore
	uint8_t alarm;		//allarme
	int32_t ht;			//high th.shold
	int32_t lt;			//low  th.shold
	int16_t period;		//periodo
	int8_t  priority;	//priorit������
} Payload;



typedef struct netPackage{
	OpCode 	code;
	Payload	payload;
} NetPackage;

//160 bit application package for the net

//###################################################
//Pacchetti Centro controllo <-> Nodo Centrale
//##################################################

#define READDATA_DIM     (sizeof(ReadDataPacketType))
#define DATA_DIM         (sizeof(DataPacketType) + 1)
#define CONFSENS_DIM     (sizeof(ConfigSensorType))
#define CANJOIN_DIM      (sizeof(CanJoinPacketType) + 1)
#define CANJOINREPLY_DIM (sizeof(CanJoinReplyPacketType))
#define JOIN_DIM         (sizeof(JoinPacketType) + 1)

typedef struct __attribute__((aligned(1),packed)) {
  uint16_t nodeAddress;
  uint8_t  sensorID;
} ReadDataPacketType;

typedef struct __attribute__((aligned(1),packed)) {
  uint16_t nodeAddress;
  uint8_t  sensorID;
  int32_t  timestamp;
  int32_t  value;
  uint8_t  alarm;
} DataPacketType;

typedef struct __attribute__((aligned(1),packed)) {
  uint16_t nodeAddress;
  uint8_t  sensorID;
  uint8_t  alarm;
  int32_t  highThreshold;
  int32_t  lowThreshold;
  int16_t  period;
  int8_t   priority;
} ConfigSensorType;

typedef struct __attribute__((aligned(1),packed)) {
  uint8_t id[12];
} NodeIDType;

typedef struct __attribute__((aligned(1),packed)) {
  uint64_t sk0;
  uint64_t sk1;
} SecretKeyType;

typedef struct __attribute__((aligned(1),packed)) {
  NodeIDType nodeID;
} CanJoinPacketType;

typedef struct __attribute__((aligned(1),packed)) {
  NodeIDType nodeID;
  SecretKeyType secretKey;
  uint16_t nodeAddress;
} CanJoinReplyPacketType;

typedef struct  __attribute__((aligned(1),packed)) {
  NodeIDType nodeID;
} JoinPacketType;

typedef struct __attribute__((aligned(1),packed)) {
  uint8_t code;
  union __attribute__((aligned(1),packed)) {
    ReadDataPacketType     readDataPacket;
    DataPacketType         dataPacket;
    ConfigSensorType       configSensor;
    CanJoinPacketType      canJoinPacket;
    CanJoinReplyPacketType canJoinReplyPacket;
    JoinPacketType         joinPacket;
  } Tpack;
  uint16_t length;
} NodeMessage;

void setCanJoinCallback(void (*callback)(NodeIDType, SecretKeyType, uint16_t nodeAddress));
void callCanJoinCallback(NodeIDType nodeID, SecretKeyType secretKey, uint16_t nodeAddress);
void canJoin(NodeIDType nodeID);
void join(NodeIDType nodeID);

#endif

