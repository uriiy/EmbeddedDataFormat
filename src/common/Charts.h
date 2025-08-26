#ifndef CHARTS_H
#define CHARTS_H

#include "edf.h"
//-----------------------------------------------------------------------------
#pragma pack(push,1)
//-----------------------------------------------------------------------------
static const TypeInfo_t ChartXYDescriptionInf =
{
	Struct, "ChartXYDescriptionInf", { 0, NULL },
	.Childs =
	{
		.Count = 3,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ CString, "AxisX" },
			{ CString, "AxisY" },
		}
	}
};
typedef struct ChartXYDesct
{
	char* Name;
	char* AxisX;
	char* AxisY;
} ChartXYDesct_t;
//-----------------------------------------------------------------------------
static const TypeInfo_t Point2DInf =
{
	Struct, "Chart2D", { 0, NULL },
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
typedef struct PointXY
{
	float x;
	float y;
} PointXY_t;

//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
#endif