#ifndef SIAMFILEFORMAT_H
#define SIAMFILEFORMAT_H

#include "stdint.h"
//-----------------------------------------------------------------------------
#pragma pack(push,1)
typedef struct
{
	uint32_t Time;			// время измерения от начала дня, мс
	int32_t Press;			// давление, 0.001 атм
	int32_t Temp;			// температура, 0.001 °С
	uint16_t Vbat;			// напряжение батареи,
	uint16_t crc;			// CRC16
} OMEGA_DATA_V1_1;
//-----------------------------------------------------------------------------
typedef struct
{
	uint16_t Shop;
	uint16_t Field;
	uint8_t Cluster[6];
	uint8_t Well[6];
	uint16_t PlaceId;
	int32_t Depth;
} FILES_RESEARCH_ID_V1_0;
//-----------------------------------------------------------------------------
typedef struct
{
	uint32_t FileType;
	char FileDescription[40];
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t NotUsed;
	FILES_RESEARCH_ID_V1_0 Id;
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
	uint32_t FileType;					//тип файла
	char FileDescription[40];			//описание файла
	RESEARCH_ID_V2_0 Id;				//идентификаторы исследования (add)
	uint16_t Reflections;				//число отражений
	uint16_t Level;						//уровень без поправки на скорость звука (для скорости 341.333 м/с), м
	int16_t Pressure;					//затрубное давление, 0.1 атм
	uint16_t Table;						//номер таблицы скоростей
	uint16_t Speed;						//скорость звука, 0.1 м/с
	int16_t BufPressure;				//буферное давление, 0.1 атм (new)
	int16_t LinePressure;				//линейное давление, 0.1 атм (new)
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
typedef struct
{
	uint32_t FileType;					//тип файла
	char FileDescription[40];			//описание файла
	RESEARCH_ID_V2_0 Id;				//идентификаторы исследования (add)
	uint16_t Rod;						//диаметр штока, 0.1 мм
	uint16_t Aperture;					//номер отверстия
	uint16_t MaxWeight;					//максимальная нагрузка, дискрет (изм)
	uint16_t MinWeight;					//минимальная нагрузка, дискрет (изм)
	uint16_t TopWeight;					//вес штанг вверху, дискрет (изм)
	uint16_t BotWeight;					//вес штанг внизу, дискрет (изм)
	uint16_t Travel;					//ход штока, дискрет
	uint16_t BeginPos;					//положение штока перед первым измерением, дискрет
	uint16_t TravelStep;				//величина дискреты перемещения, 0.1 мм
	uint16_t Period;					//период качаний, дискрет
	uint16_t TimeStep;					//величина дискреты времени, мс
	uint16_t Cycles;					//пропущено циклов
	uint16_t LoadStep;					//величина дискреты нагрузки, кг (new)
	int16_t Pressure;					//затрубное давление, 0.1 атм (new)
	int16_t BufPressure;				//буферное давление, 0.1 атм (new)
	int16_t LinePressure;				//линейное давление, 0.1 атм (new)
	uint16_t PumpType;					//тип привода станка-качалки (new)
	uint16_t Acc;						//напряжение аккумулятора датчика, 0.1В (new)
	int16_t Temp;						//температура датчика, 0.1С (new)
	uint16_t Data[1000];				//данные динамограммы
	uint16_t crc;						//crc16
} DYN_FILE_V2_0;

//-----------------------------------------------------------------------------
// edf infos
//-----------------------------------------------------------------------------
static const TypeInfo_t CommentsInf = { .Type = CString, .Name = "Comments" };
//-----------------------------------------------------------------------------
static const TypeInfo_t ResearchTimeInf =
{
	.Type = Struct, .Name = "Time", .Dims = { 0, NULL }, .Childs =
	{
		.Count = 6,
		.Item = (TypeInfo_t[])
		{
			{ UInt8, "Hour" },
			{ UInt8, "Min" },
			{ UInt8, "Sec" },
			{ UInt8, "Day" },
			{ UInt8, "Month" },
			{ UInt8, "Year" },
		}
	}
};
//-----------------------------------------------------------------------------
static const TypeInfo_t OmegaDataInf =
{
	.Type = Struct, .Name = "OMEGA_DATA_V1_1", .Dims = { 0, NULL }, .Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ UInt32, "Time" },
			{ Int32, "Press" },
			{ Int32, "Temp" },
			{ UInt16, "Vbat" },
		}
	}
};
//-----------------------------------------------------------------------------
static const TypeInfo_t DoubleValueInf =
{
	Struct, "DoubleValue", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ Double, "Value" },
			{ CString, "Unit" },
			{ CString, "Description" },
		}
	}
};
typedef struct DoubleValue
{
	char* Name;
	double Value;
	char* Unit;
	char* Description;
} DoubleValue_t;
//-----------------------------------------------------------------------------
static const TypeInfo_t Point2DInf =
{
	Struct, "Chart2D", { 0, NULL },
	.Childs =
	{
		.Count = 2,
		.Item = (TypeInfo_t[])
		{
			{ Single, "x" },
			{ Single, "y" },
		}
	}
};
struct Point2D
{
	float x;
	float y;
};

//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
#endif