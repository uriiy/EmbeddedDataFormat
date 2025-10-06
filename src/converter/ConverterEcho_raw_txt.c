#include "_pch.h"
#include "converter.h"
#include "Charts.h"
#include "SiamFileFormat.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
#include "stdlib.h"
#include "stdbool.h"
#include "time.h"
//-----------------------------------------------------------------------------
/// ECHO_RAW_TXT
//-----------------------------------------------------------------------------
/// Define
//-----------------------------------------------------------------------------
#define FILEDESCRIPTION_ECHO "ECHO_FILE"
#define SAMPLE_FREQ (170 * 6) // частота дискретизации АЦП;2000Гц - для локатора муфт, 1020Гц - для 3000м, 510Гц - для 6000м
#define WRITE_TIME 20			// максимальное время записи в секундах
#define FIR_ORDER_MAX 1023		// максимальный порядок фильтра, резервируем для массива
#define MASSIV_LENGTH (SAMPLE_FREQ + WRITE_TIME * SAMPLE_FREQ + FIR_ORDER_MAX)
//-----------------------------------------------------------------------------
/// typedef
//-----------------------------------------------------------------------------
typedef struct
{
	float raw[MASSIV_LENGTH]; // отсчеты сигнала
	float alpha;				  // коэффициент для вырезания постоянной составляющей
	float betta;				  // коэффициент для экспоненциального сглаживания
	float gamma;				  // коэффициент для удаления паразитного смещения сигнала между максимумами
	float tetta;				  // возведение сигнала в степень для сжатия амплитуды
	uint16_t len;				  // длинна массива
	uint16_t num;				  // количество полученных отсчетов
	uint16_t sample;			  // частота дискретизации
	uint8_t ku;					  // коэффициент усиления сигнала предварительным усилителем
	uint8_t sh;					  // признак включен ли шунт (оптопара) пьезо-датчика (ослабляет сигнал ~5раз) 1-шунт выключен, 0-включен
	float min;					  // минимальное значение
	float max;					  // максимальное значение
	float med;					  // среднее значение
	float adc_signal_ratio;	  // коэффициент использования ацп, как полно сигнал использует его разрядность
	float noise_amp;			  // амплитуда шумов в остатках сигнала
	float noise_threshold;	  // коэффициент для амплитуды шумов, ниже которого удаляем сигналы

	// добавка дополнительных переменных
	uint16_t errors;

} DEV_SIGNAL;

#pragma pack(push, 1)
static const TypeInfo_t MeasReport =
	 {
		  .Type = Struct,
		  .Name = "MeasReport",
		  .Dims = {0, NULL},
		  .Childs = {
				.Count = 9,
				.Item = (TypeInfo_t[]){
					 {Double, "Urov(m)"},
					 {Single, "Speed(m/s)"},
					 {UInt16, "Table"},
					 {Double, "Discrete"},
					 {Single, "Time(s)"},
					 {UInt32, "Reflection"},
					 {Double, "Press(atm)"},
					 {Int16, "Temper(C)"},
					 {UInt16, "Err"},
				}}};

typedef struct
{
	double Urov;
	float Speed;
	uint16_t Table;
	double Discrete;
	float Time;
	uint32_t Reflection;
	double Pressure;
	int16_t Temperature;
	uint16_t Errors;
} MeasReport_t;

static const TypeInfo_t DeviceState =
	 {
		  .Type = Struct,
		  .Name = "DeviceState",
		  .Dims = {0, NULL},
		  .Childs = {
				.Count = 6,
				.Item = (TypeInfo_t[]){
					 {UInt8, "KU"},
					 {UInt8, "SHUNT"},
					 {UInt8, "AWT"},
					 {UInt32, "DLIT(ms)"},
					 {Single, "Vbat(V)"},
					 {UInt8, "Avarkl"},
				}}};

typedef struct
{
	uint8_t KU;
	uint8_t SHUNT;
	uint8_t AWT;
	uint32_t DLIT;
	float Vbat;
	uint8_t Avarkl;
} DeviceState_t;

#pragma pack(pop)
//-----------------------------------------------------------------------------
/// typedef
//-----------------------------------------------------------------------------
DEV_SIGNAL sig;

//-----------------------------------------------------------------------------
int EchoRawToEdf(const char *src, const char *edf, char mode)
{
	static EdfWriter_t dw;

	static int meas_cnt = 0;
	static int file_cnt = 0;
	static char buff[512];
	static char *ch;
	static size_t writed;

	FILE *f;
	int err = fopen_s(&f, src, "rb");
	if (err)
		return err;
	do
	{
		ch = buff;
		do
		{
			file_cnt = fread(ch, 1, 1, f);
			if (*ch == '\r')
			{
				*ch = '\0';
				break;
			}
			ch++;
		} while (file_cnt);
		sig.raw[meas_cnt] = (float)(atoi((char *)buff));
		meas_cnt++;
	} while (file_cnt);
	sig.num = meas_cnt - 1;
	fclose(f);
	//-----------------------------------------------------------------------------
	EdfOpen(&dw, edf, "wb");
	//-----------------------------------------------------------------------------
	EdfHeader_t h = MakeHeaderDefault();
	if (err = EdfWriteHeader(&dw, &h, &writed))
		return err;
	//-----------------------------------------------------------------------------
	EdfWriteInfDataString(&dw, 0, "FileDescription",
								 FILEDESCRIPTION_ECHO, sizeof(FILEDESCRIPTION_ECHO));
	//-----------------------------------------------------------------------------
	EdfWriteInfo(&dw, &(const TypeRec_t){DeviceInfoType, DEVICEINFO, "DevInfo"}, &writed);
	EdfWriteDataBlock(&dw, &(DeviceInfo_t){.HwId = 64, .HwModel = 1, .SwId = 64, .SwModel = 1, .SwRevision = 122, .HwNumber = 1234},
							sizeof(DeviceInfo_t));
	//-----------------------------------------------------------------------------
	EdfWriteInfo(&dw, &(const TypeRec_t){DateTimeType, BEGINDATETIME, "BeginDateTime"}, &writed);
	struct tm local_time;
	_getsystime(&local_time);
	DateTime_t date_time = {
		 .Year = local_time.tm_year + 1900,
		 .Month = local_time.tm_mon + 1,
		 .Day = local_time.tm_mday,
		 .Hour = local_time.tm_hour,
		 .Min = local_time.tm_min,
		 .Sec = local_time.tm_sec,
		 .mSec = 0,
		 .Tz = 0,
	};
	EdfWriteDataBlock(&dw, &date_time, sizeof(date_time));
	//-----------------------------------------------------------------------------
	EdfWriteInfo(&dw, &(const TypeRec_t){PositionType, POSITION, "Position"}, &writed);
	Position_t position = {
		 .Field = "12",
		 .Cluster = "34",
		 .Well = "56",
		 .Shop = "78",
	};
	EdfWriteDataBlock(&dw, &position, sizeof(Position_t));
	//-----------------------------------------------------------------------------
	EdfWriteInfo(&dw, &(const TypeRec_t){Point2DInf, 0, "EchoChart"}, &writed);
	struct PointXY p = {0, 0};
	float xDiscrete = (1 / (float)SAMPLE_FREQ);
	int maxDepthMult = 1;
	float speed = 341.0;
	uint16_t adc_tmp;
	for (int i = 0; i < sig.num; i++)
	{
		adc_tmp = (uint16_t)sig.raw[i];
		p.x = xDiscrete * i * maxDepthMult * speed / 2.0;
		p.y = (float)(adc_tmp >> 4);
		EdfWriteDataBlock(&dw, &p, sizeof(struct PointXY));
	}
	//-----------------------------------------------------------------------------
	EdfWriteInfData(&dw, 0, Double, "Discrete", &((double){xDiscrete})); // 1 / (float)SAMPLE_FREQ
	EdfWriteInfData(&dw, 0, UInt32, "Reflection", &((uint32_t){3}));		// hu->reflection
	EdfWriteInfData(&dw, 0, Double, "Urov(m)", &((double){309.45}));		// sdata->distance
	EdfWriteInfData(&dw, 0, Double, "Pressure(atm)", &((double){-0.1})); // DHReg.mCurrReg.Pressure
	EdfWriteInfData(&dw, 0, UInt16, "Table", &((uint16_t){0}));
	EdfWriteInfData(&dw, 0, Single, "Speed(m/s)", &((float){speed})); // sdata->speed

	// EdfWriteInfo(&dw, &(const TypeRec_t){MeasReport, 0, "MeasReport"}, &writed);
	// EdfWriteDataBlock(&dw, &(MeasReport_t){
	// 									.Urov = 309.45,
	// 									.Speed = speed,
	// 									.Table = 0,
	// 									.Discrete = xDiscrete,
	// 									.Time = 100,
	// 									.Reflection = 3,
	// 									.Pressure = -0.1,
	// 									.Temperature = 23,
	// 									.Errors = 0,
	// 							  },
	// 						sizeof(MeasReport_t));

	EdfWriteInfData(&dw, 0, Double, "BufPressure", &((double){0}));
	EdfWriteInfData(&dw, 0, Double, "LinePressure", &((double){0}));

	// EdfWriteInfo(&dw, &(const TypeRec_t){DeviceState, 0, "DeviceState"}, &writed);
	// EdfWriteDataBlock(&dw, &(DeviceState_t){
	// 									.KU = 4,
	// 									.SHUNT = 1,
	// 									.AWT = 0,
	// 									.DLIT = 0,
	// 									.Vbat = 4.137,
	// 									.Avarkl = false,
	// 							  },
	// 						sizeof(DeviceState_t));

	EdfWriteInfData(&dw, 0, UInt8, "KU", &((uint8_t){4}));				  // get_ku()
	EdfWriteInfData(&dw, 0, UInt8, "SHUNT", &((uint8_t){1}));			  // get_shunt()
	EdfWriteInfData(&dw, 0, UInt8, "AWT", &((uint8_t){0}));				  // DHReg.mParam_Survey.wypusk ? 0 : 1
	EdfWriteInfData(&dw, 0, UInt32, "DLIT(ms)", &((uint32_t){0}));		  // get_solenoid_1_shot()
	EdfWriteInfData(&dw, 0, Single, "Time(s)", &((float){0}));			  // sdata->time
	EdfWriteInfData(&dw, 0, Int16, "Temperature(C)", &((int16_t){24})); // DHReg.mCurrReg.Temperature
	EdfWriteInfData(&dw, 0, Single, "Vbat(V)", &((float){4.137}));		  // DHReg.mCurrReg.BatttVoltage / 1000.0
	EdfWriteInfData(&dw, 0, UInt16, "Errors", &((uint16_t){0}));		  // sig->errors
	EdfWriteInfData(&dw, 0, UInt8, "Avarkl", &((bool){false}));			  // get_avarkl()

	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------