#include "table.h"

using namespace std;

table::table(BYTE* ptr) {
    if (ptr == nullptr) return;
    pFileBuffer = ptr;
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((BYTE*)pFileBuffer + *(((DWORD*)pFileBuffer) + 15));
	pPEHeader = (PIMAGE_FILE_HEADER) & (pNTHeader->FileHeader);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) & (pNTHeader->OptionalHeader);
	pSectionHeader = (PIMAGE_SECTION_HEADER)(pOptionHeader + 1);
}

table::table(string dir) {
    if (openFile(dir) == nullptr) return;
    
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((BYTE*)pFileBuffer + *(((DWORD*)pFileBuffer) + 15));
	pPEHeader = (PIMAGE_FILE_HEADER) & (pNTHeader->FileHeader);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) & (pNTHeader->OptionalHeader);
	pSectionHeader = (PIMAGE_SECTION_HEADER)(pOptionHeader + 1);
}
table::~table() {
    pDosHeader = nullptr;
	pNTHeader = nullptr;
	pPEHeader = nullptr;
	pOptionHeader = nullptr;
	pSectionHeader = nullptr;
	if (pFileBuffer != nullptr) {
		free(pFileBuffer);
	}
    pFileBuffer = nullptr
}

DWORD table::ReadPEFile(IN string lpszFile, OUT LPVOID pFileBuffer) {
	FILE* f;
	f = fopen(lpszFile, "rb");
	if (!f){
		cout << " 无法打开 EXE 文件! " << endl;
		return 0;
	}
	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	pFileBuffer = (LPVOID*)malloc(file_size);
	if (!pFileBuffer) {
		cout << " 分配空间失败! " << endl;
		fclose(f);
		return 0;
	}
	size_t num = fread(pFileBuffer, file_size, 1, f);
	if (!num) {
		cout << " 读取数据失败! " << endl;
		free(pFileBuffer);
		fclose(f);
		return 0;
	}
	fclose(f);
	return file_size;
}
BYTE* table::openFile(string dir) {
	FILE* f = fopen(dir, "rb");
	if (!f){
		cout << " 无法打开 EXE 文件! " << endl;
		return nullptr;
	}
	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	pFileBuffer = (BYTE*)malloc(file_size);
	memset(pFileBuffer, 0, file_size);
	if (!pFileBuffer) {
		printf(" 分配空间失败! ");
		fclose(f);
		return nullptr;
	}
	size_t num = fread(pFileBuffer, file_size, 1, f);
	if (!num) {
		cout << " 读取数据失败! " << endl;
		free(pFileBuffer);
		fclose(f);
		return nullptr;
	}
	fclose(f);
	return pFileBuffer;
}