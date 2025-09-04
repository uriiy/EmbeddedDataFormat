#include "_pch.h"
#include "converter.h"
#include "KeyValue.h"
#include "Charts.h"
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
static uint16_t PackLevel(double level, double discrete)
{
	return ((uint16_t)round(level)) | (Discrete6000 == discrete ? 0x4000 : 0);
}
//-----------------------------------------------------------------------------
static uint16_t ExtractReflections(uint16_t val)
{
	// ахтунг! параметр передаётся в двоично десятичном виде :-(
	// правильнее:  десятично-шестнадцатиричном :-)
	const uint16_t mask = 0x000F;
	uint16_t dec = (uint16_t)((((val >> 4)) & mask) * 10);
	uint16_t sig = (uint16_t)(val & mask);
	int refect = dec + sig;
	return (refect > 99) ? (uint16_t)99 : (uint16_t)refect;
}
//-----------------------------------------------------------------------------
// функция преобразования двоичного в 2/10 число. 0xffff=>65535
static uint16_t PackReflections(int16_t  chisl)
{
	int16_t  pr;
	uint32_t  sum = 0;
	for (int i = 0; i < 5; ++i)
	{
		pr = chisl / 10;
		sum += ((uint32_t)(chisl - pr * 10)) << (4 * i);
		chisl = pr;
	}
	return (uint16_t)sum;
}
//-----------------------------------------------------------------------------
static double UnPow(double v, double p)
{
	if (0 > v)
		return pow(fabs(v), p) * (-1.0f);
	return pow(v, p);
}
static double Pow(double v, double p)
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

	EdfWriteInfData(&dw, 0, UInt32, "FileType", &dat.FileType);
	EdfWriteInfDataString(&dw, 0, "FileDescription",
		&dat.FileDescription, FIELD_SIZEOF(ECHO_FILE_V2_0, FileDescription));

	EdfWriteInfo(&dw, &(const TypeRec_t){ 0, DateTimeInf}, & writed);
	EdfWriteDataBlock(&dw, &(DateTime_t)
	{
		dat.Id.Time.Year + 2000, dat.Id.Time.Month, dat.Id.Time.Day,
			dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec,
	}, sizeof(DateTime_t));

	EdfWriteInfData(&dw, 0, UInt16, "Oper", &dat.Id.Oper);
	EdfWriteInfData(&dw, 0, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, 0, UInt16, "Field", &dat.Id.Field);
	EdfWriteInfDataString(&dw, 0, "Cluster",
		&dat.Id.Cluster, FIELD_SIZEOF(RESEARCH_ID_V2_0, Cluster));
	EdfWriteInfDataString(&dw, 0, "Well",
		&dat.Id.Well, FIELD_SIZEOF(RESEARCH_ID_V2_0, Well));

	EdfWriteInfData(&dw, 0, UInt16, "ResearchType", &dat.Id.ResearchType);
	EdfWriteInfData(&dw, 0, UInt16, "RegType", &dat.Id.RegType);
	EdfWriteInfData(&dw, 0, UInt32, "RegNum", &dat.Id.RegNum);
	EdfWriteInfData(&dw, 0, UInt16, "DeviceType", &dat.Id.DeviceType);
	EdfWriteInfData(&dw, 0, UInt32, "DeviceNum", &dat.Id.DeviceNum);

	EdfWriteInfo(&dw, &(const TypeRec_t){ 0, CommentsInf}, & writed);
	EdfWriteDataBlock(&dw, &((char*) { "Reflections - число отражений" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Level - уровень без поправки на скорость звука (для скорости 341.333 м/с), м" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Pressure - затрубное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Table - номер таблицы скоростей" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Speed - скорость звука, м/с" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "BufPressure - буферное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "LinePressure - линейное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Current - ток, 0.1А" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "IdleHour - время простоя, ч" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "IdleMin - время простоя, мин" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Mode - режим исследования" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Acc - напряжение аккумулятора датчика, (В)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Temp - температура датчика, (°С)" }), sizeof(char*));

	EdfWriteInfData(&dw, 0, Double, "Discrete", &discrete);//!!
	EdfWriteInfData(&dw, 0, UInt16, "Reflections", &((uint16_t) { ExtractReflections(dat.Reflections) }));
	EdfWriteInfData(&dw, 0, Double, "Level", &((double) { ExtractLevel(dat.Level) }));

	EdfWriteInfData(&dw, 0, Double, "Pressure", &((double) { dat.Pressure / 10.0f }));
	EdfWriteInfData(&dw, 0, UInt16, "Table", &dat.Table);
	EdfWriteInfData(&dw, 0, Single, "Speed", &speed); //!!
	EdfWriteInfData(&dw, 0, Double, "BufPressure", &((double) { dat.BufPressure / 10.0f }));
	EdfWriteInfData(&dw, 0, Double, "LinePressure", &((double) { dat.LinePressure / 10.0f }));
	EdfWriteInfData(&dw, 0, UInt16, "Current", &dat.Current);
	EdfWriteInfData(&dw, 0, UInt8, "IdleHour", &dat.IdleHour);
	EdfWriteInfData(&dw, 0, UInt8, "IdleMin", &dat.IdleMin);

	EdfWriteInfData(&dw, 0, UInt16, "Mode", &dat.Mode);

	EdfWriteInfData(&dw, 0, Single, "Acc", &((float) { dat.Acc / 10.0f }));
	EdfWriteInfData(&dw, 0, Single, "Temp", &((float) { dat.Temp / 10.0f }));

	EdfWriteInfo(&dw, &(const TypeRec_t){ 0, CommentsInf}, & writed);
	EdfWriteDataBlock(&dw, &((char*) { "эхограмма" }), sizeof(char*));
	EdfWriteInfo(&dw, &(const TypeRec_t){ 0, ChartNInf}, & writed);
	EdfWriteDataBlock(&dw, &((ChartN_t[])
	{
		{ "Depth", "m", "", "глубина" },
		{ "Val", "adc", "", "амплитуда" },
	}), sizeof(ChartN_t) * 2);



	EdfWriteInfo(&dw, &(const TypeRec_t){ 0, Point2DInf}, & writed);

	struct PointXY p = { 0,0 };
	for (size_t i = 0; i < 3000; i++)
	{
		if (dat.Data[i] > 127)
			p.y = (float)UnPow(-1 * (dat.Data[i] - 127), 1.0 / 0.35) / 1000;
		else
			p.y = (float)UnPow(dat.Data[i], 1.0 / 0.35) / 1000;

		p.x = xDiscrete * i * maxDepthMult;

		EdfWriteDataBlock(&dw, &p, sizeof(struct PointXY));
	}
	fclose(f);
	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
int EdfToEcho(const char* edfFile, const char* echoFile)
{
	int err = 0;

	EdfWriter_t br;
	size_t writed = 0;
	if ((err = EdfOpen(&br, edfFile, "rb")))
		return err;

	FILE* f = NULL;
	if ((err = fopen_s(&f, echoFile, "wb")))
		return err;

	double discrete = Discrete3000;
	ECHO_FILE_V2_0 dat = { 0 };
	size_t recN = 0;
	PointXY_t record = { 0 };

	int skip = 0;
	uint8_t bDst[3 * 256 + 8] = { 0 };
	MemStream_t msDst = { 0 };
	if ((err = MemStreamOpen(&msDst, bDst, sizeof(bDst), 0, "w")))
		return err;

	while (!(err = EdfReadBlock(&br)))
	{
		MemStream_t src = { 0 };
		if ((err = MemStreamInOpen(&src, br.Block, br.BlockLen)))
			return err;

		switch (br.BlockType)
		{
		default: break;
		case btHeader:
			if (16 == br.BlockLen)
			{
				//EdfHeader_t h = { 0 };
				//err = MakeHeaderFromBytes(br.Block, br.BlockLen, &h);
				//if (!err)
				//	err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			skip = 0;
			msDst.WPos = 0;
			br.t = NULL;
			err = StreamWriteBinToCBin(br.Block, br.BlockLen, NULL, br.Buf, sizeof(br.Buf), NULL, &br.t);
			if (!err)
			{
				writed = 0;
			}
			else
			{
				err = 0;
				//return err;// ignore wrong or too big info block
			}
		}
		break;
		case btVarData:
		{
			//EdfWriteDataBlock(&tw, &br.Block, br.BlockLen);
			//EdfFlushDataBlock(&tw, &writed);
			if (0 == _stricmp(br.t->Inf.Name, "FileType"))
				dat.FileType = *((uint32_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "FileDescription"))
			{
				//err = EdfSreamBinToCBin(&FileDescriptionInf, &src, &msDst,
				//	&(void*){dat.FileDescription}, & skip);
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(ECHO_FILE_V2_0, FileDescription));
				memset(dat.FileDescription, 0, FIELD_SIZEOF(ECHO_FILE_V2_0, FileDescription));
				memcpy(dat.FileDescription, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Inf.Name, "ResearchType"))
				dat.Id.ResearchType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "DeviceType"))
				dat.Id.DeviceType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "DeviceNum"))
				dat.Id.DeviceNum = *((uint32_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Oper"))
				dat.Id.Oper = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Shop"))
				dat.Id.Shop = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Field"))
				dat.Id.Field = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Cluster"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
				memcpy(dat.Id.Cluster, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Inf.Name, "Well"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
				memcpy(dat.Id.Well, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Inf.Name, DateTimeInf.Name))
			{
				DateTime_t* t = NULL;
				if (!(err = EdfReadBin(&DateTimeInf, &src, &msDst, &t, &skip)))
				{
					dat.Id.Time.Year = (uint8_t)(t->Year - 2000);
					dat.Id.Time.Month = t->Month;
					dat.Id.Time.Day = t->Day;
					dat.Id.Time.Hour = t->Hour;
					dat.Id.Time.Min = t->Min;
					dat.Id.Time.Sec = t->Sec;
				}
			}
			else if (0 == _stricmp(br.t->Inf.Name, "RegType"))
				dat.Id.RegType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "RegNum"))
				dat.Id.RegNum = *((uint32_t*)br.Block);

			else if (0 == _stricmp(br.t->Inf.Name, "Discrete"))
				discrete = *((double*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Reflections"))
				dat.Reflections = PackReflections(*((uint16_t*)br.Block));
			else if (0 == _stricmp(br.t->Inf.Name, "Level"))
				dat.Level = PackLevel(*((double*)br.Block), discrete);
			else if (0 == _stricmp(br.t->Inf.Name, "Pressure"))
				dat.Pressure = (int16_t)round(*((double*)br.Block) * 10);
			else if (0 == _stricmp(br.t->Inf.Name, "Table"))
				dat.Table = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Speed"))
				dat.Speed = (uint16_t)round(*((float*)br.Block) * 10);
			else if (0 == _stricmp(br.t->Inf.Name, "BufPressure"))
				dat.BufPressure = (int16_t)round(*((double*)br.Block) * 10);
			else if (0 == _stricmp(br.t->Inf.Name, "LinePressure"))
				dat.LinePressure = (int16_t)round(*((double*)br.Block) * 10);
			else if (0 == _stricmp(br.t->Inf.Name, "Current"))
				dat.Current = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "IdleHour"))
				dat.IdleHour = *((uint8_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "IdleMin"))
				dat.IdleMin = *((uint8_t*)br.Block);
			else if (0 == _stricmp(br.t->Inf.Name, "Acc"))
				dat.Acc = (int16_t)round(*((float*)br.Block) * 10);
			else if (0 == _stricmp(br.t->Inf.Name, "Temp"))
				dat.Temp = (int16_t)round(*((float*)br.Block) * 10);

			else if (0 == _stricmp(br.t->Inf.Name, "Chart2D"))
			{
				PointXY_t* s = NULL;
				while (!(err = EdfReadBin(&Point2DInf, &src, &msDst, &s, &skip))
					&& recN <= FIELD_SIZEOF(ECHO_FILE_V2_0, Data))
				{
					dat.Data[recN] = (int8_t)round(pow(fabs(s->y * 1000), 0.35));
					if (0 > s->y)
						dat.Data[recN] += 127;
					recN++;
					record = *s;
					s = NULL;
					skip = 0;
					msDst.WPos = 0;
				}
				skip = -skip;
				err = 0;
			}//else
		}//case btVarData:
		break;
		}//switch (br.BlockType)
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}//while (!(err = EdfReadBlock(&br)))

	dat.crc = MbCrc16(&dat, sizeof(ECHO_FILE_V2_0) - 2);

	if (1 != fwrite(&dat, sizeof(ECHO_FILE_V2_0), 1, f))
		return -1;

	fclose(f);
	EdfClose(&br);
	return 0;
}