#ifndef INIFILE_20170605_H
#define INIFILE_20170605_H

#include <map>
#include <string>

class CIniFile
{

#define LINE_LEN_MAX    256   

// Parameter: <key, value> 
typedef std::map<std::string, std::string> KEYMAP; 
 
// SECTION : <section, <key, value>>
typedef std::map<std::string, KEYMAP> SECTIONMAP;  

public:
    CIniFile();
    virtual ~CIniFile();

public:
    int open(const char* path);
    void close();

    int  getInt(const char* section, const char* key);
    char *getStr(const char* section, const char* key);

    int setInt(const char* section, const char* key, int value);
    int setStr(const char* section, const char* key, const char* value);

	int saveAsFile(const char* path);
private:
    int getKey(const char* section, const char* key, char* value);
    int setKey(const char* section, const char* key, char* value);

    char mValue[LINE_LEN_MAX];
    FILE* mFp;
    SECTIONMAP mMap;
};

#endif //  INIFILE_20170605_H 

