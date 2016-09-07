/***********************************************************************************
**
** Skills.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef SKILLS_H
#define SKILLS_H
//----------------------------------------------------------------------------------
#include "Globals.h"
#include "GLEngine/GLTextTexture.h"
//----------------------------------------------------------------------------------
//����� ������
class CSkill
{
	SETGET(float, BaseValue);
	SETGET(float, Value);
	SETGET(float, Cap);
	SETGET(uchar, Status);

public:
	CSkill();
	~CSkill() {}
};
//----------------------------------------------------------------------------------
//����� ����� �������
class CSkillSort
{
private:
	//��������� �������� ������
	bool SkillCmp(const char *str1, int len1, const char *str2, int len2);

public:
	CSkillSort() {}
	~CSkillSort() {}

	//�������������
	void Init();

	//����� ����������
	uchar m_Skills[60];
};
//----------------------------------------------------------------------------------
class CSkillData
{
	SETGET(uchar, Button);
	SETGETE(string, Name, OnChangeName);

public:
	CSkillData() :m_Button(0), m_Name("") {}
	~CSkillData() {m_Texture.Clear();};

	//�������� ��������
	CGLTextTexture m_Texture;
};
//----------------------------------------------------------------------------------
extern CSkillSort g_SkillSort; //��������� �� ����� ������
extern vector<CSkillData> g_Skills; //��������� �� ������
extern int g_SkillsCount; //���������� ��������� �������
extern string g_SkillName[54]; //����������� �������� �������
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
