#include "_pch.h"
#include "CppUnitTest.h"

#include "BlockWriter.h"
#include "DfHeader.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		
		TEST_METHOD(TestHeader)
		{
			uint8_t hBuf[16];
			EdfHeader_t h1 = MakeHeaderDefault();
			int  hCount = HeaderToBytes(&h1, hBuf);
			EdfHeader_t h2 = MakeHeaderFromBytes(hBuf);
			int cmp = memcmp(&h1, &h2, sizeof(EdfHeader_t));
			if (0 != cmp) // "Header compare error!\n"
			{
				LOG_ERR();
				Assert::Fail();
			}
		}
	};
}
