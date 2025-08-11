#include "edf_cfg.h"
#include "converter.h"

static int CompareFiles(const char* src, const char* dst)
{
	int ret = 0;
	errno_t err = 0;
	FILE* f1 = NULL;
	err = fopen_s(&f1, src, "rb");
	if (err)
		return err;
	FILE* f2 = NULL;
	err = fopen_s(&f2, src, "rb");
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
	return 0;
}

//-----------------------------------------------------------------------------
static void TestMemStream(void)
{
	size_t writed = 0;
	MemStream_t ms = { 0 };
	uint8_t buf[256];
	int err = MemStreamOpen(&ms, buf, sizeof(buf), "wb");
	const char test[] = "qwe test 123";
	Stream_t* stream = (Stream_t*)&ms;
	err = StreamWrite(stream, &writed, test, sizeof(test) - 1);
	err = StreamWriteFmt(stream, &writed, " format %d", 1);
}
//-----------------------------------------------------------------------------
static void TestInit(void)
{
	TypeInfo_t tst1 =
	{
		.Type = UInt16,
		.Name = "Test",
		.Dims = { 3, (uint32_t[]) { 1,2,3 }} ,
	};

	TypeInfo_t tst2 =
	{
		.Type = Struct,
		.Name = "Test2",
		.Dims = {2, (uint32_t[]) { 11,22 }},
		.Childs =
		{
			.Count = 3,
			.Item = (TypeInfo_t[])
			{
				tst1,
				{.Type = UInt32, .Name = "x" },
				{.Type = UInt32, .Name = "y" },
			}
		}
	};
}
//-----------------------------------------------------------------------------
static void WriteTest(void)
{
	EdfWriter_t dw;
	size_t writed = 0;
	int err = OpenBinWriter(&dw, "t_write.bdf");

	EdfHeader_t h = MakeHeaderDefault();
	err = EdfWriteHeader(&dw, &h, &writed);

	TypeInfo_t t = { .Type = Int32, .Name = "weight variable" };
	err = EdfWriteInfo(&dw, &t, &writed);

	uint8_t test[100] = { 0 };
	(*(int32_t*)test) = (int32_t)(0xFFFFFFFF);

	EdfWriteDataBlock(&dw, test, 4);
	EdfFlushDataBlock(&dw, &writed);

	err = EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "BString Text" }), &writed);
	size_t len = 0;
	len += GetBString("Char", test + len, sizeof(test));
	len += GetBString("Value", test + len, sizeof(test) - len);
	len += GetBString("Array     Value", test + len, sizeof(test) - len);
	EdfWriteDataBlock(&dw, test, len);
	EdfFlushDataBlock(&dw, &writed);

	err = EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = Char, .Name = "Char Text", { 1, (uint32_t[]) { 20 } } }), &writed);
	len = 0;
	len += GetCString("Char", 20, test + len, sizeof(test));
	len += GetCString("Value", 20, test + len, sizeof(test) - len);
	len += GetCString("Array     Value", 20, test + len, sizeof(test) - len);
	EdfWriteDataBlock(&dw, test, len);

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
	err = EdfWriteInfo(&dw, &comlexVar, &writed);

	EdfClose(&dw);
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
	EdfFlushDataBlock(dw, &writed);

}
static void WriteTestBigVar()
{
	int err = 0;

	EdfWriter_t bw;
	err = OpenBinWriter(&bw, "t_big.bdf");
	WriteBigVar(&bw);
	EdfClose(&bw);

	EdfWriter_t tw;
	err = OpenTextWriter(&tw, "t_big.tdf");
	WriteBigVar(&tw);
	EdfClose(&tw);

	BinToText("t_big.bdf", "t_bigConv.tdf");

	err = CompareFiles("t_big.tdf", "t_bigConv.tdf");
	if (err)
		perror("err: t_big");
}
//-----------------------------------------------------------------------------
static void DatFormatTest()
{
	DatToEdf("1.dat", "1.tdf", 't');
	DatToEdf("1.dat", "1.bdf", 'b');
	BinToText("1.bdf", "1Conv.tdf");
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void BinToTextTest(void)
{
	const char* src = "t_write.bdf";
	const char* dst = "t_write.tdf";
	BinToText(src, dst);
}
//-----------------------------------------------------------------------------
int main()
{
	LOG_ERR();
	WriteTestBigVar();
	DatFormatTest();
	TestInit();
	//TestHeader();
	WriteTest();
	BinToTextTest();
	TestMemStream();
	return 0;
}