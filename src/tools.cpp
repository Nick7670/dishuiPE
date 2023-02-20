#include "tools.h"

tools::tools(BYTE* ptr) : table(ptr) {
	numSections = pPEHeader->NumberOfSections;
}
tools::tools(string dir) : table(dir) {
	numSections = pPEHeader->NumberOfSections;
}


void tools::MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size) {
	string dir;
	cout << "请输入保存文件路径" << endl;
	cin >> dir;
	FILE* fp = fopen(dir, "wb");
	fwrite(pMemBuffer, size, 1, fp);
	fclose(fp);
}

BYTE* tools::CopyFileBufferToImageBuffer() {
	if (!check()) {
		cout << "File point is nullptr" << endl;
		return;
	}
	BYTE* imagebuffer = (BYTE*)malloc(pOptionHeader->SizeOfImage);

	memset(imagebuffer, 0, pOptionHeader->SizeOfImage);
	memcpy(imagebuffer, pFileBuffer, pOptionHeader->SizeOfHeaders);

	for (size_t i = 0; i < numSections; i++) {
		memcpy(imagebuffer + pSectionHeader->VirtualAddress, pFileBuffer + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData);
		pSectionHeader = pSectionHeader + 1;
	}

	return imagebuffer;
}
BYTE* tools::CopyImageBufferToNewBuffer(IN BYTE* pImageBuffer) {
	if (!check()) {
		cout << "File point is nullptr" << endl;
		return;
	}
	pSectionHeader = pSectionHeader + (numSections - 1);

	BYTE* newbuffer = (BYTE*)malloc(pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData);
	memset(newbuffer, 0, pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData);
	pSectionHeader = pSectionHeader - (numSections - 1);
	memcpy(newbuffer, pImageBuffer, pOptionHeader->SizeOfHeaders);

	for (size_t i = 0; i < numSections; i++){
		memcpy(newbuffer + pSectionHeader->PointerToRawData, pImageBuffer + pSectionHeader->VirtualAddress, pSectionHeader->SizeOfRawData);
		pSectionHeader = pSectionHeader + 1;
	}
	return newbuffer;
}

DWORD tools::RVA_FOA(DWORD RVA){
	if (!check()) {
		cout << "File point is nullptr" << endl;
		return;
	}
	if (RVA < pOptionHeader->SizeOfHeaders) {
		return RVA;
	} else if (RVA > pOptionHeader->SizeOfImage)  {
		cout << "error" << endl;
		return 0;
	} else {
		size_t i = 0;
		size_t distance = 0;
		while (i < numSections) {
			if (pSectionHeader->VirtualAddress <= RVA && RVA <= pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize) {
				distance = RVA - pSectionHeader->VirtualAddress;
				break;
			}
			i++;
			pSectionHeader += 1;
		}
		size_t foa = pSectionHeader->PointerToRawData + distance;
		return foa;
	}

}
DWORD tools::FOA_RVA(DWORD FOA) {
	if (!check()) {
		cout << "File point is nullptr" << endl;
		return;
	}
	pSectionHeader = pSectionHeader + (numSections - 1);

	if (FOA < pOptionHeader->SizeOfHeaders) {
		return FOA;
	} else if (FOA> pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData) {
		cout << "error" << endl;
		return FOA;
	} else {
		pSectionHeader = pSectionHeader - (numSections - 1);
		size_t i = distance = 0;
		while (i < numSections) {
			if (pSectionHeader->PointerToRawData <= FOA && FOA <= pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData) {
				distance = FOA - pSectionHeader->PointerToRawData;
				break;
			}
			i++;
			pSectionHeader += 1;
		}
		size_t RVA = pSectionHeader->VirtualAddress + distance;

		return RVA;
	}

}

DWORD tools::getstr(BYTE* name) {
	DWORD len = 0;
	while (*name != 0) {
		len++;
		name++;
	}
	return len;
}

bool tools::check() {
	if (pFileBuffer == nullptr) return false;
	return true;
}

int tools::alignment(int num, int alignment) {
	if (num % alignment != 0) {
		int mid = 0;
		while (mid - num > 0) {
			mid += alignment;
		}
		alignment += mid - num;
		return alignment;
	} else {
		return alignment;
	}
}
