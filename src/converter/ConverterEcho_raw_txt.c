#include "_pch.h"
#include "converter.h"
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
#define SAMPLE_FREQ (170 * 6) // частота дискретизации АЦП;2000Гц - для локатора муфт, 1020Гц - для 3000м, 510Гц - для 6000м
#define WRITE_TIME 20		 // максимальное время записи в секундах
#define FIR_ORDER_MAX 1023 // максимальный порядок фильтра, резервируем для массива
#define MASSIV_LENGTH (SAMPLE_FREQ + WRITE_TIME * SAMPLE_FREQ + FIR_ORDER_MAX)
//-----------------------------------------------------------------------------
/// typedef
//-----------------------------------------------------------------------------
typedef struct
{
	float raw[MASSIV_LENGTH]; 	// отсчеты сигнала
	float alpha;			  	// коэффициент для вырезания постоянной составляющей
	float betta;			  	// коэффициент для экспоненциального сглаживания
	float gamma;			  	// коэффициент для удаления паразитного смещения сигнала между максимумами
	float tetta;			 	// возведение сигнала в степень для сжатия амплитуды
	uint16_t len;			 	// длинна массива
	uint16_t num;			 	// количество полученных отсчетов
	uint16_t sample;		 	// частота дискретизации
	uint8_t ku;				 	// коэффициент усиления сигнала предварительным усилителем
	uint8_t sh;				 	// признак включен ли шунт (оптопара) пьезо-датчика (ослабляет сигнал ~5раз) 1-шунт выключен, 0-включен
	float min;				 	// минимальное значение
	float max;				 	// максимальное значение
	float med;				 	// среднее значение
	float adc_signal_ratio;		// коэффициент использования ацп, как полно сигнал использует его разрядность
	float noise_amp;			// амплитуда шумов в остатках сигнала
	float noise_threshold;		// коэффициент для амплитуды шумов, ниже которого удаляем сигналы

	//добавка дополнительных переменных
	uint16_t errors;

} DEV_SIGNAL;
//-----------------------------------------------------------------------------
/// typedef
//-----------------------------------------------------------------------------
DEV_SIGNAL sig;

//-----------------------------------------------------------------------------
int EchoRawToEdf(const char* src, const char* edf, char mode)
{
	static EdfWriter_t dw;

	static int meas_cnt = 0;
	static int file_cnt = 0;
	static char buff[512];
	static char* ch;
	static size_t writed;

	FILE* f;
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
		sig.raw[meas_cnt] = (float)(atoi((char*)buff));
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
	EdfWriteInfo(&dw, &DateTimeInf, &writed);
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

	//-----------------------------------------------------------------------------
	EdfWriteInfo(&dw, &Point2DInf, &writed);
	struct Point2D p = { 0,0 };
	float xDiscrete = (1 / (float)SAMPLE_FREQ);
	int maxDepthMult = 1;
	float speed = 341.0;
	uint16_t adc_tmp;
	for (int i = 0; i < sig.num; i++)
	{
		adc_tmp = (uint16_t)sig.raw[i];
		p.x = xDiscrete * i * maxDepthMult * speed / 2.0;
		p.y = (float)(adc_tmp >> 4);
		EdfWriteDataBlock(&dw, &p, sizeof(struct Point2D));
	}
	//-----------------------------------------------------------------------------
	EdfWriteInfData(&dw, UInt8, "KU", &((uint8_t) { 4 }));
	EdfWriteInfData(&dw, UInt8, "SHUNT", &((uint8_t) { 1 }));
	EdfWriteInfData(&dw, UInt8, "AWT", &((uint8_t) { 0 }));
	EdfWriteInfData(&dw, UInt32, "DLIT(ms)", &((uint32_t) { 0 }));
	EdfWriteInfData(&dw, Single, "Urov(m)", &((float) { 309.45 }));
	EdfWriteInfData(&dw, Single, "Speed(m/s)", &((float) { speed }));
	EdfWriteInfData(&dw, Single, "Time(s)", &((float) { 0 }));
	EdfWriteInfData(&dw, UInt32, "Reflection", &((uint32_t) { 3 }));
	EdfWriteInfData(&dw, UInt16, "Sample", &((uint16_t) { 1020 }));
	EdfWriteInfData(&dw, Single, "Pressure(atm)", &((float) { -0.1 }));
	EdfWriteInfData(&dw, Int16, "Temperature(C)", &((int16_t) { 24 }));
	EdfWriteInfData(&dw, Single, "Vbat(V)", &((float) { 4.137 }));
	EdfWriteInfData(&dw, UInt16, "Errors", &((uint16_t) { 0 }));
	EdfWriteInfData(&dw, UInt8, "Avarkl", &((bool) { false }));

	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
