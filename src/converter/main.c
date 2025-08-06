#include "_pch.h"
#include "edf_cfg.h"

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

	EdfWriteDataBlock(test, 4, &dw);
	EdfFlushDataBlock(&dw);

	err = EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "CharArrayVariable" }), &writed);

	len += GetBString("Char", test + len, sizeof(test));
	len += GetBString("Value", test + len, sizeof(test) - len);
	len += GetBString("Array     Value", test + len, sizeof(test) - len);
	EdfWriteDataBlock(test, len, &dw);
	EdfFlushDataBlock(&dw);

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
static void BinToText(const char* src, const char* dst)
{
	EdfWriter_t br = { 0 };
	EdfWriter_t tw = { 0 };
	if (OpenBinReader(&br, src))
		LOG_ERR();
	if(OpenTextWriter(&tw, dst))
		LOG_ERR();

	size_t writed = 0;
	int err = 0;

	while (-1 != EdfReadBlock(&br))
	{
		switch (br.Block[0])
		{
		default: break;
		case btHeader:
			if (4 + 16 == br.BlockLen)
			{
				EdfHeader_t h = { 0 };
				err = MakeHeaderFromBytes(&br.Block[4], br.BlockLen - 4, &h);
				if (!err)
					err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			uint8_t* src = &br.Block[4];
			TypeInfo_t* t = (TypeInfo_t*)&br.Buf;
			tw.t = t;
			uint8_t* mem = (uint8_t*)&br.Buf + sizeof(TypeInfo_t);
			err = FromBytes(&src, t, &mem);
			if (!err)
				err = EdfWriteInfo(&tw, t, &writed);
		}
		break;
		case btVarData:
		{
			err = EdfWriteDataBlock(&br.Block[4], br.BlockLen - 4, &tw);
			if (!err)
				EdfFlushDataBlock(&tw);
		}
		break;
		}
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}
	EdfClose(&br);
	EdfClose(&tw);
}
//-----------------------------------------------------------------------------
static void BinToTextTest(void)
{
	const char* src = "c_test.bdf";
	const char* dst = "c_test.tdf";
	BinToText(src, dst);
}
//-----------------------------------------------------------------------------
static void FilenameToTdf(const char* input, char* output)
{
	size_t len = strlen(input);
	if (4 < len && 0 == strcmp(input + len - 4, ".bdf"))
	{
		memcpy(output, input, len);
		*(output + len) = '\0';
		output += len - 3;
		*output = 't';
	}
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#ifdef _DEBUG
	LOG_ERR();
	TestInit();
	//TestHeader();
	WriteTest();
	BinToTextTest();
#else
	if (2 < argc && NULL != argv[1] && 't' == *argv[2])
	{
		char fn[512];
		FilenameToTdf(argv[1], fn);
		BinToText(argv[1], fn);
	}
#endif // DEBUG
	return 0;
}

