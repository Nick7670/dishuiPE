#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <windows.h>
#include <malloc.h>
#include "table.h"
using namespace std;
class dosHeader : public table {
public:
    dosHeader() = default;
    dosHeader(BYTE* ptr) : table(ptr){};
    dosHeader(string dir) : table(dir){};
    ~dosHeader();
    DWORD PrintPEInfo();
}

class section : public table {
public:
    section() = default;
    section(BYTE* ptr) : table(ptr){};
    section(string dir) : table(dir){};
    ~section();
    combin_section();
    BYTE* enlarge_last_section_filebuffer(BYTE* pFileBuffer, size_t num);
    BYTE* enlarge_last_section_imagebuffer(BYTE* pImageBuffer, size_t num);
    BYTE* AddSectionHeader_filebuffer(BYTE* pFileBuffer, PIMAGE_SECTION_HEADER pSectionHeader_3);
    BYTE* AddSectionHeader_imagebuffer(BYTE* pImageBuffer, PIMAGE_SECTION_HEADER pSectionHeader_3);
}
#endif
