// MIT License

#ifndef WISP_TFP_H
#define WISP_TFP_H

#include <stdint.h>
#include <common/str.h>
#include <common/fs.h>
#include <xuocore/mappedfile.h>

namespace Wisp
{
class CTextFileParser
{
public:
    astr_t RawLine = "";

private:
    CMappedFile m_File;
    uint8_t *m_Ptr = 0;
    uint8_t *m_EOL = 0;
    uint8_t *m_End = 0;
    char m_Delimiters[50];
    int m_DelimitersSize = 0;
    char m_Comentaries[50];
    int m_ComentariesSize = 0;
    char m_Quotes[50];
    int m_QuotesSize = 0;
    bool m_Trim = false;

    void GetEOL();
    bool IsDelimiter();
    void SkipToData();
    bool IsComment();
    bool IsQuote();
    bool IsSecondQuote();
    astr_t ObtainData();
    astr_t ObtainQuotedData();
    void StartupInitalize(const char *delimiters, const char *comentaries, const char *quotes);
    void SaveRawLine();

public:
    CTextFileParser(
        const fs_path &path,
        const char *delimiters = "",
        const char *comentaries = "",
        const char *quotes = "");
    ~CTextFileParser();

    void Restart();
    bool IsEOF();

    std::vector<astr_t> ReadTokens(bool trim = true);
    std::vector<astr_t> GetTokens(const char *str, bool trim = true);
};

class CTextFileWriter
{
private:
    FILE *m_File{ nullptr };

public:
    CTextFileWriter(const fs_path &path);
    ~CTextFileWriter();

    bool Opened() { return (m_File != nullptr); };
    void Close();
    void WriteString(const astr_t &key, const astr_t &value);
    void WriteInt(const astr_t &key, int value);
    void WriteBool(const astr_t &key, bool value);
};

}; // namespace Wisp

#endif // WISP_TFP_H
