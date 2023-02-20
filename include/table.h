#ifndef TABLE_H
#define TABLE_H
#include <windows.h>
#include <iostream>
#include <cstring>
/**
 * 含有所有的文件指针，负责读取文件内容
 */
class table {
public:
    table() = default;
    table(BYTE* ptr);
    table(string dir);//这个函数不能让子类重写，如果子类可以调用这个函数的话，一个文件会被多次打开，但是这个是没有必要的
    virtual ~table();

    DWORD ReadPEFile(IN string lpszFile, OUT LPVOID pFileBuffer);
    BYTE* openFile(string dir);

private:
    BYTE* pFileBuffer;
    PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_FILE_HEADER pPEHeader;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader;
	PIMAGE_SECTION_HEADER pSectionHeader;
}



#endif