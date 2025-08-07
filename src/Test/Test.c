#include "edf_cfg.h"
#include "converter.h"
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

	//uint8_t buf[256];

	//size_t sz = ToBytes(&tst2, buf);

	uint8_t buf2[256];
	memcpy(buf2, &tst2, sizeof(TypeInfo_t));

	//size_t r = 0, w = 0;
	//TypeInfo_t* rtst2 = FromBytes(buf, buf2,&r, &w);
	//TypeInfo_t tst22 = MakeTypeInfo("Test3", Struct, 2, (uint32_t[]) { 2 }, 1, (TypeInfo_t[]) { tst1 });
}
//-----------------------------------------------------------------------------
static void WriteTest(void)
{
	EdfWriter_t dw;
	size_t writed = 0;
	int err = OpenBinWriter(&dw, "c_test.bdf");

	EdfHeader_t h = MakeHeaderDefault();
	err = EdfWriteHeader(&dw, &h, &writed);

	TypeInfo_t t = { .Type = Int32, .Name = "weight variable" };
	err = EdfWriteInfo(&dw, &t, &writed);

	uint8_t test[100] = { 0 }; size_t len = 0;
	(*(int32_t*)test) = (int32_t)(0xFFFFFFFF);

	EdfWriteDataBlock(&dw, test, 4);
	EdfFlushDataBlock(&dw, &writed);

	err = EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "CharArrayVariable" }), &writed);

	len += GetBString("Char", test + len, sizeof(test));
	len += GetBString("Value", test + len, sizeof(test) - len);
	len += GetBString("Array     Value", test + len, sizeof(test) - len);
	EdfWriteDataBlock(&dw, test, len);
	EdfFlushDataBlock(&dw, &writed);

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


	/*
	(*(int32_t*)&test[0]) = (int32_t)(0x04000000);
	(*(int32_t*)&test[4]) = (int32_t)(0x05000000);
	(*(int16_t*)&test[8]) = (int16_t)(0x0600);
	WriteDataBlock(&t, test, 10, &dw);

	(*(int16_t*)&test[0]) = (int16_t)(0x0000);
	(*(int16_t*)&test[2]) = (int16_t)(0x1314);
	WriteDataBlock(&t, test, 4, &dw);
	*/

	EdfClose(&dw);
}
//-----------------------------------------------------------------------------
static void BinToTextTest(void)
{
	const char* src = "c_test.bdf";
	const char* dst = "c_test.tdf";
	BinToText(src, dst);
}
//-----------------------------------------------------------------------------
int main()
{
	LOG_ERR();
	TestInit();
	//TestHeader();
	WriteTest();
	BinToTextTest();
	DatToTdf("1.dat", "1.tdf");
	TestMemStream();
	return 0;
}