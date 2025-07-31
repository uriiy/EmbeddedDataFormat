// df_c.c : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include "_pch.h"
#include "BlockWriter.h"
#include "DfHeader.h"
#include "TypeInfo.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

//-----------------------------------------------------------------------------
void TestInit()
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

	uint8_t buf[256];

	size_t sz = ToBytes(&tst2, buf);

	uint8_t buf2[256];
	memcpy(buf2, &tst2, sizeof(TypeInfo_t));

	size_t r = 0, w = 0;
	//TypeInfo_t* rtst2 = FromBytes(buf, buf2,&r, &w);

	TypeInfo_t tst22 = MakeTypeInfo("Test3", Struct, 2, (uint32_t[]) { 2 }, 1, (TypeInfo_t[]) { tst1 });
}
//-----------------------------------------------------------------------------
void WriteTest()
{
	FILE* f = fopen("c_test.bdf", "wb");
	DataWriter_t dw =
	{
		.Stream = {.Instance = f, StreamWrite, StreamRead },
		.Seq = 0,
		.BlockLen = 0, .BufLen = 0,
		.WritePrimitive = BinToBin,
		.Flush = FlushBinBlock,
		.SepBeginStruct = NoWrite,
		.SepEndStruct = NoWrite,
		.SepBeginArray = NoWrite,
		.SepEndArray = NoWrite,
		.SepVar = NoWrite,
		.SepRecBegin = NoWrite,
		.SepRecEnd = NoWrite,
	};

	DfHeader_t h = MakeHeaderDefault();
	WriteHeaderBlock(&h, &dw);

	TypeInfo_t t = { .Type = Int32, .Name = "weight variable" };
	WriteVarInfoBlock(&t, &dw);

	uint8_t test[100]; size_t len = 0;
	(*(int32_t*)test) = (int32_t)(0xFFFFFFFF);

	WriteDataBlock(test, 4, &dw);
	FlushDataBlock(&dw);

	WriteVarInfoBlock(&((TypeInfo_t) { .Type = String, .Name = "CharArrayVariable" }), &dw);

	len += GetBString("Char", test + len, sizeof(test));
	len += GetBString("Value", test + len, sizeof(test) - len);
	len += GetBString("Array     Value", test + len, sizeof(test) - len);
	WriteDataBlock(test, len, &dw);
	FlushDataBlock(&dw);

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
	WriteVarInfoBlock(&comlexVar, &dw);


	/*
	(*(int32_t*)&test[0]) = (int32_t)(0x04000000);
	(*(int32_t*)&test[4]) = (int32_t)(0x05000000);
	(*(int16_t*)&test[8]) = (int16_t)(0x0600);
	WriteDataBlock(&t, test, 10, &dw);

	(*(int16_t*)&test[0]) = (int16_t)(0x0000);
	(*(int16_t*)&test[2]) = (int16_t)(0x1314);
	WriteDataBlock(&t, test, 4, &dw);
	*/

	FlushDataBlock(&dw);
	fflush(f);
	fclose(f);

}
//-----------------------------------------------------------------------------
void BinToText(const char* src, const char* dst)
{
	FILE* fsrc = fopen(src, "rb");
	FILE* fdst = fopen(dst, "wb");

	DataWriter_t br =
	{
		.Stream = {.Instance = fsrc, .Read = StreamRead },
		.Seq = 0,
		.BlockLen = 0, .BufLen = 0,
		.WritePrimitive = BinToBin,
		.Flush = FlushBinBlock,
		.SepBeginStruct = NoWrite,
		.SepEndStruct = NoWrite,
		.SepBeginArray = NoWrite,
		.SepEndArray = NoWrite,
		.SepVar = NoWrite,
		.SepRecBegin = NoWrite,
		.SepRecEnd = NoWrite,
	};

	DataWriter_t tw =
	{
		.Stream = {.Instance = fdst, .Write = StreamWrite },
		.Seq = 0,
		.BlockLen = 0, .BufLen = 0,
		.WritePrimitive = BinToStr,
		.Flush = FlushTxtBlock,
		.SepBeginStruct = SepBeginStruct,
		.SepEndStruct = SepEndStruct,
		.SepBeginArray = SepBeginArray,
		.SepEndArray = SepEndArray,
		.SepVar = SepVar,
		.SepRecBegin = SepRecBegin,
		.SepRecEnd = SepRecEnd,
	};


	DfHeader_t* h = (DfHeader_t*)br.Buf;
	ReadHeaderBlock(&br, h);

	int len = sprintf(br.Block, "~ version=%d.%d.%d bs=%d encoding=%d flags=%d \n"
		, h->VersMajor, h->VersMinor, h->VersPatch
		, h->Blocksize, h->Encoding, h->Flags);
	fwrite(br.Block, 1, len, fdst);


	while (-1 != ReadBlock(&br))
	{
		switch (br.Block[0])
		{
		default: break;
		case btVarInfo:
		{
			uint8_t* src = &br.Block[4];
			TypeInfo_t* t = tw.t = (TypeInfo_t*)&br.Buf;
			uint8_t* mem = (uint8_t*)&br.Buf + sizeof(TypeInfo_t);
			len = FromBytes(&src, t, &mem);
			len = ToString(t, br.Block, 0);
			fwrite("\n\n? ", 1, 4, fdst);
			fwrite(br.Block, 1, len, fdst);
			fflush(fdst);
			tw.BlockLen = 0;
		}
		break;
		case btVarData:
		{
			int ret = WriteDataBlock(&br.Block[4], br.BlockLen - 4, &tw);
			FlushDataBlock(&tw);
		}
		break;
		}
	}

	fclose(fsrc);
	fclose(fdst);
}
//-----------------------------------------------------------------------------
void BinToTextTest()
{
	const char* src = "c_test.bdf";
	const char* dst = "c_test.tdf";
	BinToText(src, dst);
}
//-----------------------------------------------------------------------------
void FilenameToTdf(const char* input, char* output)
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
	if (2 < argc && NULL != argv[1] && 't' == *argv[2])
	{
		char fn[512];
		FilenameToTdf(argv[1], fn);
		BinToText(argv[1], fn);

	}

#ifdef _DEBUG
	LOG_ERR();
	TestInit();
	//TestHeader();
	WriteTest();
	BinToTextTest();

#endif // DEBUG

	return 0;
}

//#ifdef __cplusplus
//}
//#endif
