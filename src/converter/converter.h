#include "_pch.h"

int BinToText(const char* src, const char* dst);
int DatToEdf(const char* src, const char* dst, char mode);
int EchoToEdf(const char* src, const char* dst, char mode);
int ChangeExt(char* file, const char* input, const char* output);
int IsExt(const char* file, const char* ext);

#pragma pack(push,1)
typedef struct
{
	uint32_t Time;			// время измерения от начала дня, мс
	int32_t Press;			// давление, 0.001 атм
	int32_t Temp;			// температура, 0.001 °С
	uint16_t Vbat;			// напряжение батареи,
	uint16_t crc;			// CRC16
} OMEGA_DATA_FILE_V1_1;

typedef struct
{
	uint16_t Shop;
	uint16_t Field;
	uint8_t Cluster[6];
	uint8_t Well[6];
	uint16_t PlaceId;
	int32_t Depth;
} FILES_RESEARCH_ID_V1_0;

typedef struct
{
	uint32_t FileType;
	char FileDescription[40];
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t NotUsed;
	FILES_RESEARCH_ID_V1_0 id;
	uint16_t RegType;
	uint16_t RegNum;
	uint16_t RegVer;
	uint16_t SensType;
	uint32_t SensNum;
	uint16_t SensVer;
	uint16_t crc;
} SPSK_FILE_V1_1;

//-----------------------------------------------------------------------------
typedef struct
{
	uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;
} TIME;
//-----------------------------------------------------------------------------
typedef struct
{
	uint16_t ResearchType;			//тип исследования
	uint16_t DeviceType;			//тип датчика
	uint32_t DeviceNum;				//номер датчика
	uint16_t Shop;					//номер цеха
	uint16_t Oper;					//номер оператора
	uint16_t Field;					//код месторождения
	char Cluster[6];				//номер куста
	char Well[6];					//номер скважины
	TIME Time;						//время начала исследования
	uint16_t RegType;				//тип регистратора (new)
	uint32_t RegNum;				//номер регистратора (new)
} RESEARCH_ID_V2_0;
//-----------------------------------------------------------------------------
typedef struct
{
	uint32_t FileType;
	char FileDescription[40];
	RESEARCH_ID_V2_0 id;
	uint16_t Reflections;				//число отражений
	uint16_t Level;						//уровень без поправки на скорость звука (для скорости 341.333 м/с), м
	int16_t Pressure;					//затрубное давление, 0.1 атм
	uint16_t Table;						//номер таблицы скоростей
	uint16_t Speed;						//скорость звука, 0.1 м/с
	int16_t BufPressure;				//буферное давление, 0.1 атм (new)
	int16_t LinPressure;				//линейное давление, 0.1 атм (new)
	uint16_t Current;					//ток, 0.1А (new)
	uint8_t IdleHour;					//время простоя, ч (new)
	uint8_t IdleMin;					//время простоя, мин (new)
	uint16_t Mode;						//режим исследования (new)
	uint16_t Acc;						//напряжение аккумулятора датчика, 0.1В (new)
	int16_t Temp;						//температура датчика, 0.1С (new)
	uint8_t Data[3000];					//данные эхограммы
	uint16_t crc;						//crc16
} ECHO_FILE_V2_0;
//-----------------------------------------------------------------------------

#pragma pack(pop)