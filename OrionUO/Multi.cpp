/***********************************************************************************
**
** Multi.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "Multi.h"
//----------------------------------------------------------------------------------
CMulti::CMulti(const short &x, const short &y)
: CBaseQueueItem(), m_X(x), m_Y(y), m_MinX(0), m_MinY(0), m_MaxX(0), m_MaxY(0)
{
}
//----------------------------------------------------------------------------------
CMulti::~CMulti()
{
}
//----------------------------------------------------------------------------------
