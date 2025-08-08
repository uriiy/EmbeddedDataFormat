#include "_pch.h"

int BinToText(const char* src, const char* dst);
int DatToEdf(const char* src, const char* dst, char mode);
int ChangeExt(char* file, const char* input, const char* output);
int IsExt(const char* file, const char* ext);

#pragma pack(push,1)
typedef struct
{
	uint32_t Time; // время измерения от начала дня, мс
	int32_t Press; // давление, 0.001 атм
	int32_t Temp;	// температура, 0.001 °С
	uint16_t Vbat; // напряжение батареи,
	uint16_t crc;	// CRC16
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
#pragma pack(pop)