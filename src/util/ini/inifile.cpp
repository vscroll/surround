#include "inifile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

CIniFile::CIniFile()
{
    mFp = NULL;
}

CIniFile::~CIniFile()
{
    mMap.clear();
}

int CIniFile::open(const char* path)  
{
    string lastSection;
    KEYMAP lastMap;
    int  indexPos = -1;
    int  leftPos = -1;
    int  rightPos = -1;

	mFp = fopen(path, "r");
    if (mFp == NULL)
    {
        printf("open inifile %s error!\n", path);
        return -1;
    }

    mMap.clear();

    char line[LINE_LEN_MAX] = {0};
    string lineTmp;
    while (fgets(line, LINE_LEN_MAX, mFp))
    {
        lineTmp.assign(line);
        if (lineTmp.compare("\n") == 0
            || lineTmp.compare("\r\n") == 0
            || lineTmp.length() <= 2)
        {
            continue;
        }

        //删除字符串中的非必要字符
        leftPos = lineTmp.find("\n");
        if (string::npos != leftPos)
        {
            lineTmp.erase(leftPos, 1);
        }

        leftPos = lineTmp.find("\r");
        if (string::npos != leftPos)
        {
            lineTmp.erase(leftPos, 1);
        }

        //判断是否是Section
        leftPos = lineTmp.find("[");
        rightPos = lineTmp.find("]");
        if (leftPos != string::npos && rightPos != string::npos)
        {
            lineTmp.erase(leftPos, 1);
            rightPos--;
            lineTmp.erase(rightPos, 1);
            if (lastMap.size() > 0)
            {
                mMap[lastSection] = lastMap;
                lastMap.clear();
            }

            lastSection = lineTmp;
        }
        else
        {
            //是否key
            indexPos = lineTmp.find('=');
            if (string::npos != indexPos)
            {
                string key = lineTmp.substr(0, indexPos);
                string value = lineTmp.substr(indexPos+1, lineTmp.length()-indexPos-1);
                lastMap[key] = value;
            }
            else
            {
                //TODO:不符合ini键值模板的内容 如注释等
            }
        }
    }

    //插入最后一次主键
    mMap[lastSection] = lastMap;

    return 0;
}  

void CIniFile::close()
{
    if (mFp != NULL)
    {
        fclose(mFp);
        mFp = NULL;
    }
}

int CIniFile::saveAsFile(const char* path)
{
	FILE* fp = NULL;
	fp = fopen(path, "w+");
    if (fp == NULL)
    {
		return -1;
	}

	SECTIONMAP::iterator it = mMap.begin();
	for (; it != mMap.end(); ++it)
	{
		std::string section = it->first;
		char tmpSection[LINE_LEN_MAX] = {0};
		sprintf(tmpSection, "[%s]\n", section.c_str());
		fwrite(tmpSection, 1, strlen(tmpSection), fp);
		KEYMAP keyMap = it->second;
		KEYMAP::iterator keyIt = keyMap.begin();
		//printf("222 %s \n", tmpSection);
		for (; keyIt != keyMap.end(); ++keyIt)
		{
			std::string key = keyIt->first;
			std::string value = keyIt->second;
			char tmpKey[LINE_LEN_MAX] = {0};
			sprintf(tmpKey, "%s=%s\n", key.c_str(), value.c_str());
			fwrite(tmpKey, 1, strlen(tmpKey), fp);

			//printf("333 %s \n", tmpKey);
		}
        fwrite("\n", 1, 1, fp);	
	}

	fclose(fp);
    fp = NULL;
}

int CIniFile::getKey(const char* section, const char* key, char* value)
{
    KEYMAP keyMap = mMap[section];

    string tmp = keyMap[key];

    strcpy(value, tmp.c_str());

    return 0;
}

int CIniFile::setKey(const char* section, const char* key, char* value)
{
	if (mMap.find(section) == mMap.end())
	{
		KEYMAP keyMap;
		keyMap[key]	= value;
		mMap[section] = keyMap;
	}
	else
	{
    	KEYMAP keyMap = mMap[section];
		keyMap[key] = value;
	}

	return 0;
}

int CIniFile::getInt(const char* section, const char* key)  
{
    int result = 0;

    char value[LINE_LEN_MAX] = {0};
    if (0 == getKey(section, key, value))
    {
        result = atoi(value);
    }
    return result;
}  

char* CIniFile::getStr(const char* section, const char* key)
{
    memset(mValue, 0, sizeof(mValue));
    getKey(section, key, mValue);

    return mValue;
}

int CIniFile::setInt(const char* section, const char* key, int value)
{
	char tmp[LINE_LEN_MAX] = {0};
	sprintf(tmp, "%d", value);
	return setKey(section, key, tmp);
}

int CIniFile::setStr(const char* section, const char* key, const char* value)
{
	return setKey(section, key, (char*)value);
}
