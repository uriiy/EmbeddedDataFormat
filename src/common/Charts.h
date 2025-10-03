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
			{ String, "Name" },
			{ String, "AxisX" },
			{ String, "AxisY" },
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
static const TypeInfo_t ChartNInf =
{
	Struct, "ChartNInf", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ String, "Name" },
			{ String, "Unit" },
			{ String, "API Code" },
			{ String, "Description" },
		}
	}
};
typedef struct ChartN
{
	char* Name;
	char* Unit;
	char* ApiCode;
	char* Desc;
} ChartN_t;
//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
#endif