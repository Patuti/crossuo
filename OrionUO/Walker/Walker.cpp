/***********************************************************************************
**
** Walker.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "Walker.h"
//----------------------------------------------------------------------------------
CWalker *g_Walker = NULL;
//----------------------------------------------------------------------------------
CWalker::CWalker()
: m_Sequence(0)
{
	memset(m_Direction, 0, sizeof(m_Direction));
	m_FastWalkStack.Reset();
}
//---------------------------------------------------------------------------
void CWalker::IncSequence()
{
	if (m_Sequence == 255)
		m_Sequence = 1;
	else
		m_Sequence++;
}
//---------------------------------------------------------------------------
void CWalker::SetSequence(const uchar &seq, const uchar &dir)
{
	m_Sequence = seq;
	m_Direction[seq] = dir;
}
//----------------------------------------------------------------------------------
