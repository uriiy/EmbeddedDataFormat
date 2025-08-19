#include "_pch.h"
#include "converter.h"
#include "SiamFileFormat.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
/// ECHO
//-----------------------------------------------------------------------------

static const double Discrete3000 = 0.00585938;
static const double Discrete6000 = 0.01171875;
//-----------------------------------------------------------------------------
static double ExtractDiscrete(uint16_t level)
{
	return (level & 0x4000) ? Discrete6000 : Discrete3000;
}
//-----------------------------------------------------------------------------
static double ExtractLevel(uint16_t level)
{
	return level & 0xBFFF;
}
//-----------------------------------------------------------------------------
static uint16_t ExtractReflections(uint16_t val)
{
	// ахтунг! параметр передаётся в двоично десятичном виде :-(
	const uint16_t mask = 0x000F;
	uint16_t dec = (uint16_t)((((val >> 4)) & mask) * 10);
	uint16_t sig = (uint16_t)(val & mask);
	int refect = dec + sig;
	return (refect > 99) ? (uint16_t)99 : (uint16_t)refect;
}
//-----------------------------------------------------------------------------
static double UnPow(double v, double p)
{
	if (0 > v)
		return pow(fabs(v), p) * (-1.0f);
	return pow(v, p);
}
//-----------------------------------------------------------------------------
int EchoToEdf(const char* src, const char* edf, char mode)
{
	FILE* f = NULL;
	int err = fopen_s(&f, src, "rb");
	if (err)
		return err;

	ECHO_FILE_V2_0 dat;
	if (1 != fread(&dat, sizeof(ECHO_FILE_V2_0), 1, f))
		return -1;

	EdfWriter_t dw;
	size_t writed = 0;

	if ('t' == mode)
		err = EdfOpen(&dw, edf, "wt");
	else if ('b' == mode)
		err = EdfOpen(&dw, edf, "wb");
	else
		err = -1;
	if (err)
		return err;

	double discrete = ExtractDiscrete(dat.Level);
	int maxDepthMult = 1;
	if (Discrete3000 < discrete)
		maxDepthMult = 2;
	float speed = (float)dat.Speed / 10;
	float xDiscrete = speed / 341;

	EdfHeader_t h = MakeHeaderDefault();
	if ((err = EdfWriteHeader(&dw, &h, &writed)))
		return err;

	EdfWriteInfData(&dw, UInt32, "FileType", &dat.FileType);
	EdfWriteStringBytes(&dw, "FileDescription", &dat.FileDescription, FIELD_SIZEOF(ECHO_FILE_V2_0, FileDescription));

	EdfWriteInfData(&dw, UInt16, "ResearchType", &dat.Id.ResearchType);
	EdfWriteInfData(&dw, UInt16, "DeviceType", &dat.Id.DeviceType);
	EdfWriteInfData(&dw, UInt16, "DeviceNum", &dat.Id.DeviceNum);
	EdfWriteInfData(&dw, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, UInt16, "Oper", &dat.Id.Oper);
	EdfWriteInfData(&dw, UInt16, "Field", &dat.Id.Field);
	EdfWriteStringBytes(&dw, "Cluster", &dat.Id.Cluster, FIELD_SIZEOF(RESEARCH_ID_V2_0, Cluster));
	EdfWriteStringBytes(&dw, "Well", &dat.Id.Well, FIELD_SIZEOF(RESEARCH_ID_V2_0, Well));

	char cbuf[256] = { 0 };
	snprintf(cbuf, sizeof(cbuf), "%u.%02u.%02uT%02u:%02u:%02u",
		(uint32_t)2000 + dat.Id.Time.Year, dat.Id.Time.Month, dat.Id.Time.Day,
		dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec);
	EdfWriteInfData(&dw, CString, "DateTime", &((char*) { cbuf }));
	//EdfWriteInfData(&dw, UInt8, "Year", &dat.Id.Time.Year);
	//EdfWriteInfData(&dw, UInt8, "Month", &dat.Id.Time.Month);
	//EdfWriteInfData(&dw, UInt8, "Day", &dat.Id.Time.Day);
	//EdfWriteInfData(&dw, UInt8, "Hour", &dat.Id.Time.Hour);
	//EdfWriteInfData(&dw, UInt8, "Min", &dat.Id.Time.Min);
	//EdfWriteInfData(&dw, UInt8, "Sec", &dat.Id.Time.Sec);

	EdfWriteInfData(&dw, UInt16, "RegType", &dat.Id.RegType);
	EdfWriteInfData(&dw, UInt32, "RegNum", &dat.Id.RegNum);

	EdfWriteInfData(&dw, UInt16, "Reflections", &((uint16_t) { ExtractReflections(dat.Reflections) }));
	EdfWriteInfData(&dw, Double, "Level", &((double) { ExtractLevel(dat.Level) }));
	EdfWriteInfData(&dw, Double, "Discrete", &discrete);//!!

	EdfWriteInfData(&dw, Double, "Pressure", &((double) { dat.Pressure / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "Table", &dat.Table);
	EdfWriteInfData(&dw, Single, "Speed", &speed); //!!
	EdfWriteInfData(&dw, Double, "BufPressure", &((double) { dat.BufPressure / 10.0f }));
	EdfWriteInfData(&dw, Double, "LinePressure", &((double) { dat.LinePressure / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "Current", &dat.Current);
	EdfWriteInfData(&dw, UInt8, "IdleHour", &dat.IdleHour);
	EdfWriteInfData(&dw, UInt8, "IdleMin", &dat.IdleMin);

	EdfWriteInfData(&dw, UInt16, "Mode", &dat.Mode);

	EdfWriteInfData(&dw, Single, "Acc", &((float) { dat.Acc / 10.0f }));
	EdfWriteInfData(&dw, Single, "Temp", &((float) { dat.Temp / 10.0f }));

	//EdfWriteInfData(&dw, Double, "EchoChart", &dat.Data);
	//for (size_t i = 0; i < 3000; i++)
	//	EdfWriteDataBlock(&dw, &dat.Data[i], sizeof(uint8_t));

#pragma pack(push,1)
	struct Point
	{
		float x;
		float y;
	};
#pragma pack(pop)
	TypeInfo_t pointType =
	{
		Struct, "EchoChart", { 0, NULL },
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
	EdfWriteInfo(&dw, &pointType, &writed);

	struct Point p = { 0,0 };
	for (size_t i = 0; i < 3000; i++)
	{
		if (dat.Data[i] > 127)
			p.y = (float)UnPow(-1 * (dat.Data[i] - 127), 1.0 / 0.35) / 1000;
		else
			p.y = (float)UnPow(dat.Data[i], 1.0 / 0.35) / 1000;

		p.x = xDiscrete * i * maxDepthMult;

		EdfWriteDataBlock(&dw, &p, sizeof(struct Point));
	}

	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
int EdfToEcho(const char* src, const char* dst)
{
	return 0;
}