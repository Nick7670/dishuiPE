#ifndef TOOLS_H
#define TOOLS_H

#include <cstring>
#include <windows.h>
#include <iostream>
#include "table.h"

using namespace std;
class tools : public table {
public:
    tools() = default;
    tools(BYTE* ptr);
    tools(string dir);
    ~tools();
    void MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size);
    BYTE* CopyFileBufferToImageBuffer();
    BYTE* CopyImageBufferToNewBuffer();
    int alignment(int num, int alignment);
    DWORD RVA_FOA(DWORD RVA);
    DWORD FOA_RVA(DWORD FOA);
    DWORD getstr(BYTE* name);
    bool check();
    int numSections;

}

#endif