#include "converter.h"
#include "edf_cfg.h"
#include "assert.h"

static int CompareFiles(const char* src, const char* dst)
{
	int ret = 0;
	errno_t err = 0;
	FILE* f1 = NULL;
	err = fopen_s(&f1, src, "rb");
	if (err)
		return err;
	FILE* f2 = NULL;
	err = fopen_s(&f2, dst, "rb");
	if (err)
		return err;
	uint8_t buf1[1024];
	uint8_t buf2[1024];
	size_t readed1 = 0;
	size_t readed2 = 0;
	do
	{
		readed1 = fread(buf1, 1, sizeof(buf1), f1);
		readed2 = fread(buf2, 1, sizeof(buf2), f2);
		if (readed1 != readed2 || memcmp(buf1, buf2, readed1))
		{
			ret = 1;
			break;
		}

	} while (!feof(f1) && feof(f2));


	fclose(f1);
	fclose(f2);
	return ret;
}

//-----------------------------------------------------------------------------
static void TestMemStream(void)
{
	size_t writed = 0;
	MemStream_t ms = { 0 };
	uint8_t buf[256];
	int err = MemStreamOpen(&ms, buf, sizeof(buf), "w");
	const char test[] = "qwe test 123";
	Stream_t* stream = (Stream_t*)&ms;
	err = StreamWrite(stream, &writed, test, sizeof(test) - 1);
	err = StreamWriteFmt(stream, &writed, " format %d", 1);
}
//-----------------------------------------------------------------------------
static int WriteSample(EdfWriter_t* dw)
{
	size_t writed = 0;
	int err = 0;

	EdfHeader_t h = MakeHeaderDefault();
	err = EdfWriteHeader(dw, &h, &writed);

#pragma pack(push,1)
	typedef struct KeyValue
	{
		char* Key;
		char* Value;
	} KeyValue_t;
	TypeInfo_t keyValueType =
	{
		.Type = Struct, .Name = "KeyValue", .Dims = {0, NULL},
		.Childs =
		{
			.Count = 2,
			.Item = (TypeInfo_t[])
			{
				{ CString, "Key" },
				{ CString, "Value" },
			}
		}
	};
#pragma pack(pop)

	err = EdfWriteInfo(dw, &keyValueType, &writed);
	EdfWriteDataBlock(dw, &((KeyValue_t) { "Key1", "Value1" }), sizeof(KeyValue_t));
	EdfWriteDataBlock(dw, &((KeyValue_t) { "Key2", "Value2" }), sizeof(KeyValue_t));
	EdfWriteDataBlock(dw, &((KeyValue_t) { "Key3", "Value3" }), sizeof(KeyValue_t));

	EdfWriteInfData(dw, String, "тестовый ключ", "String Value");

	TypeInfo_t t = { .Type = Int32, .Name = "weight variable" };
	err = EdfWriteInfo(dw, &t, &writed);
	uint8_t test[100] = { 0 };
	(*(int32_t*)test) = (int32_t)(0xFFFFFFFF);
	EdfWriteDataBlock(dw, test, 4);
	EdfFlushDataBlock(dw, &writed);

	TypeInfo_t td = { .Type = Double, .Name = "TestDouble" };
	err = EdfWriteInfo(dw, &td, &writed);
	double dd = 1.1;
	EdfWriteDataBlock(dw, &dd, sizeof(double));
	dd = 2.1;
	EdfWriteDataBlock(dw, &dd, sizeof(double));
	dd = 3.1;
	EdfWriteDataBlock(dw, &dd, sizeof(double));

	err = EdfWriteInfo(dw, &((TypeInfo_t) { .Type = String, .Name = "BString Text" }), &writed);
	size_t len = 0;
	len += GetBString("Char", test + len, sizeof(test));
	len += GetBString("Value", test + len, sizeof(test) - len);
	len += GetBString("Array     Value", test + len, sizeof(test) - len);
	EdfWriteDataBlock(dw, test, len);
	EdfFlushDataBlock(dw, &writed);

	err = EdfWriteInfo(dw, &((TypeInfo_t) { .Type = Char, .Name = "Char Text", { 1, (uint32_t[]) { 20 } } }), &writed);
	len = 0;
	len += GetCString("Char", 20, test + len, sizeof(test));
	len += GetCString("Value", 20, test + len, sizeof(test) - len);
	len += GetCString("Array     Value", 20, test + len, sizeof(test) - len);
	EdfWriteDataBlock(dw, test, len);

	TypeInfo_t comlexVar =
	{
		.Type = Struct, .Name = "ComplexVariable", .Dims = {0, NULL},
		.Childs =
		{
			.Count = 2,
			.Item = (TypeInfo_t[])
			{
				(TypeInfo_t)
				{
					Int64, "time"
				},
				(TypeInfo_t)
				{
					Struct, "State", { 1, (uint32_t[]) { 3 }} ,
					.Childs =
					{
						.Count = 3,
						.Item = (TypeInfo_t[])
						{
							(TypeInfo_t)
							{
								Int8, "text"
							},
							(TypeInfo_t)
							{
								Struct, "Pos",{0, NULL} ,
								.Childs =
								{
									.Count = 2,
									.Item = (TypeInfo_t[])
									{
										{ Int32, "x" },
										{ Int32, "y" },
									}
								}
							},
							(TypeInfo_t)
							{
								Double, "Temp",{ 2, (uint32_t[]) { 2,2 }},
							},
						}
					}
				},
			}
		}
	};
	err = EdfWriteInfo(dw, &comlexVar, &writed);
#pragma pack(push,1)
	struct ComplexVariable
	{
		int64_t time;
		struct State
		{
			int8_t text;
			struct
			{
				int32_t x;
				int32_t y;
			} Pos;
			double Temp[2][2];
		} State[3];
	};
#pragma pack(pop)
	struct ComplexVariable cv =
	{
		.time = -123,
		.State =
		{
			{ 1, { 11, 12 }, {1.1,1.2,1.3,1.4 } },
			{ 2, { 21, 22 }, {2.1,2.2,2.3,2.4 } },
			{ 3, { 31, 32 }, {3.1,3.2,3.3,3.4 } },
		}
	};
	EdfWriteDataBlock(dw, &cv, sizeof(struct ComplexVariable));
	return err;
}
//-----------------------------------------------------------------------------
static int WriteTest()
{
	EdfWriter_t w;
	int err = 0;
	// TEXT write
	err = EdfOpen(&w, "t_write.tdf", "wt");
	WriteSample(&w);
	EdfClose(&w);
	// test append
	memset(&w, 0, sizeof(EdfWriter_t));
	err = EdfOpen(&w, "t_write.tdf", "at");
	if (0 != err)
		return err;
	EdfWriteInfData(&w, Int32, "Int32 Key", &((int32_t) { 0xb1b2b3b4 }));
	EdfClose(&w);

	// BINary write
	err = EdfOpen(&w, "t_write.bdf", "wb");
	WriteSample(&w);
	EdfClose(&w);
	// test append
	memset(&w, 0, sizeof(EdfWriter_t));
	if ((err = EdfOpen(&w, "t_write.bdf", "ab")))
		return err;
	EdfWriteInfData(&w, Int32, "Int32 Key", &((int32_t) { 0xb1b2b3b4 }));
	EdfClose(&w);

	BinToText("t_write.bdf", "t_writeConv.tdf");
	err = CompareFiles("t_write.tdf", "t_writeConv.tdf");
	if (err)
		LOG_ERRF("err %d: t_write files not equal", err);
	assert(0 == err);
	return err;
}
//-----------------------------------------------------------------------------
static void WriteBigVar(EdfWriter_t* dw)
{
	int err = 0;
	size_t writed = 0;
	EdfHeader_t h = MakeHeaderDefault();
	err = EdfWriteHeader(dw, &h, &writed);

	size_t arrLen = (size_t)(BLOCK_SIZE / sizeof(uint32_t) * 2.5);
	TypeInfo_t t = { .Type = Int32, .Name = "variable", .Dims = { 1, (uint32_t[]) { arrLen }} };
	err = EdfWriteInfo(dw, &t, &writed);

	uint32_t test[1000] = { 0 };
	for (uint32_t i = 0; i < arrLen; i++)
		test[i] = i;
	EdfWriteDataBlock(dw, test, sizeof(uint32_t) * arrLen);

	uint8_t* test2 = (uint8_t*)test;
	EdfWriteDataBlock(dw, test2, 15);
	EdfWriteDataBlock(dw, test2 + 15, 149);
	EdfWriteDataBlock(dw, test2 + 15 + 149, (sizeof(uint32_t) * arrLen) - 15 - 149);

	EdfFlushDataBlock(dw, &writed);
}
static void WriteTestBigVar()
{
	int err = 0;

	EdfWriter_t bw;
	err = EdfOpen(&bw, "t_big.bdf", "wb");
	WriteBigVar(&bw);
	EdfClose(&bw);

	EdfWriter_t tw;
	err = EdfOpen(&tw, "t_big.tdf", "wt");
	WriteBigVar(&tw);
	EdfClose(&tw);

	BinToText("t_big.bdf", "t_bigConv.tdf");

	err = CompareFiles("t_big.tdf", "t_bigConv.tdf");
	if (err)
		LOG_ERRF("err: t_big %d", err);
	assert(0 == err);
}
//-----------------------------------------------------------------------------
static void DatFormatTest()
{
	assert(0 == DatToEdf("1DAT.dat", "1DAT.tdf", 't'));
	assert(0 == DatToEdf("1DAT.dat", "1DAT.bdf", 'b'));
	assert(0 == BinToText("1DAT.bdf", "1DATConv.tdf"));
	assert(0 == CompareFiles("1DAT.tdf", "1DATConv.tdf"));
	assert(0 == EdfToDat("1DAT.bdf", "1DATConv.dat"));
	assert(0 == CompareFiles("1DAT.dat", "1DATConv.dat"));

	assert(0 == EchoToEdf("1E.E", "1E.tdf", 't'));
	assert(0 == EchoToEdf("1E.E", "1E.bdf", 'b'));
	assert(0 == BinToText("1E.bdf", "1EConv.tdf"));
	assert(0 == CompareFiles("1E.tdf", "1EConv.tdf"));
	assert(0 == EdfToEcho("1E.bdf", "1EConv.E"));
	//assert(0 == CompareFiles("1E.E", "1EConv.E"));

	assert(0 == DynToEdf("1D.D", "1D.tdf", 't'));
	assert(0 == DynToEdf("1D.D", "1D.bdf", 'b'));
	assert(0 == BinToText("1D.bdf", "1DConv.tdf"));
	assert(0 == CompareFiles("1D.tdf", "1DConv.tdf"));
	assert(0 == EdfToDyn("1D.bdf", "1DConv.D"));
	//assert(0 == CompareFiles("1D.D", "1DConv.D"));
}
//-----------------------------------------------------------------------------
static void MbCrc16accTest()
{
	const char* test =
		"some test data text 1"
		"some test data text 2"
		"some test data text 3"
		"some test data text 4";
	size_t len = strnlength(test, 256);
	uint16_t crc = MbCrc16(test, len);

	uint16_t crcAcc = 0xFFFF;
	crcAcc = MbCrc16acc(test, 17, crcAcc);
	crcAcc = MbCrc16acc(test + 17, len - 17, crcAcc);
	assert(crcAcc == crc);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main()
{
	LOG_ERR();
	MbCrc16accTest();
	WriteTestBigVar();
	DatFormatTest();
	WriteTest();
	TestMemStream();
	return 0;
}