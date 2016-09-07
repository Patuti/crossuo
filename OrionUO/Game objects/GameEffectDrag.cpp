/***********************************************************************************
**
** GameEffectDrag.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "GameEffectDrag.h"
//----------------------------------------------------------------------------------
CGameEffectDrag::CGameEffectDrag()
: CGameEffect(), m_OffsetX(0), m_OffsetY(0)
{
}
//----------------------------------------------------------------------------------
CGameEffectDrag::~CGameEffectDrag()
{
}
//----------------------------------------------------------------------------------
/*!
�������� ������
@return
*/
void CGameEffectDrag::Update()
{
	m_OffsetX += 10;
	m_OffsetY += 10;
}
//----------------------------------------------------------------------------------
