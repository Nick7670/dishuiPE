
#include "header.h"


dosHeader::~dosHeader() {

}


DWORD dosHeader::PrintPEInfo(IN BYTE* pFileBuffer) {
	cout << hex;
	cout << "e_magic:" << pDosHeader->e_magic << endl;
	cout << "e_lfanew:" << pDosHeader->e_lfanew << endl;
	cout << "Signature:   " << pNTHeader->Signature << endl;
	cout << "Machine:   " << pPEHeader->Machine << endl;
	cout << "NumberOfSections:   " << pPEHeader->NumberOfSections << endl;
	cout << "TimeDateStamp:   " << pPEHeader->TimeDateStamp << endl;
	cout << "PointerToSymbolTable:   " << pPEHeader->PointerToSymbolTable << endl;
	cout << "NumberOfSymbols:   " << pPEHeader->NumberOfSymbols << endl;
	cout << "SizeOfOptionalHeader:   " << pPEHeader->SizeOfOptionalHeader << endl;
	cout << "Characteristics:   " << pPEHeader->Characteristics << endl;

	cout << "Magic:   " << pOptionHeader->Magic << endl;
	cout << "AddressOfEntryPoint:   " << pOptionHeader->AddressOfEntryPoint << endl;
	cout << "ImageBase:   " << pOptionHeader->ImageBase << endl;
	cout << "SectionAlignment:   " << pOptionHeader->SectionAlignment << endl;
	cout << "FileAlignment:   " << pOptionHeader->FileAlignment << endl;
	cout << "SizeOfImage:   " << pOptionHeader->SizeOfImage << endl;
	cout << "SizeOfHeaders:   " << pOptionHeader->SizeOfHeaders << endl;
	size_t num_sections= pPEHeader->NumberOfSections;
	for (size_t i = 0; i < num_sections; i++) {
		cout << "Name:   " << pSectionHeader->Name << endl;
		cout << "Misc:   " << pSectionHeader->Misc << endl;
		cout << "VirtualAddress:   " << pSectionHeader->VirtualAddress << endl;
		cout << "SizeOfRawData:   " << pSectionHeader->SizeOfRawData << endl;
		cout << "PointerToRawData:   " << pSectionHeader->PointerToRawData << endl;
		cout << "PointerToRelocations:   " << pSectionHeader->PointerToRelocations << endl;
		cout << "PointerToLinenumbers:   " << pSectionHeader->PointerToLinenumbers << endl;
		cout << "NumberOfRelocations:   " << pSectionHeader->NumberOfRelocations << endl;
		cout << "NumberOfLinenumbers:   " << pSectionHeader->NumberOfLinenumbers << endl;
		cout << "Characteristics:   " << pSectionHeader->Characteristics << endl;
		pSectionHeader = pSectionHeader + 1;
	}
	cout << dec;
	return 0;
}





BYTE* section::combin_section(BYTE* pImageBuffer,size_t num) {
	size_t num_sections = pPEHeader->NumberOfSections;	
	if (num > num_sections) {
		printf("此节后面没有节表");
		return pImageBuffer;
	}
	pSectionHeader += num - 1;
	PIMAGE_SECTION_HEADER pSectionHeader_2 = pSectionHeader + 1;

	int mem_alignment = alignment(pSectionHeader_2->Misc.VirtualSize, pOptionHeader->SectionAlignment);
	int file_alignment = alignment(pSectionHeader_2->SizeOfRawData, pOptionHeader->FileAlignment);
	pSectionHeader->Misc.VirtualSize = (pSectionHeader_2->VirtualAddress + mem_alignment) - pSectionHeader->VirtualAddress;
	pSectionHeader->SizeOfRawData = (pSectionHeader_2->PointerToRawData + file_alignment) - pSectionHeader->PointerToRawData;
	//找到需要合并两个节中的第一个节，将virtualsize改为第二个节终点减去第一个节起点（内存中），sizeofraw同理

	pSectionHeader->Characteristics = (pSectionHeader->Characteristics | pSectionHeader_2->Characteristics);
	//修改第一个节的属性，“或”上第二个节的

	for (size_t i = num; i < num_sections;i++) {
		pSectionHeader += 1;
		pSectionHeader_2 += 1;
		memcpy(pSectionHeader, pSectionHeader_2, 40);

	}
	//将节表往上提，覆盖第二个节表
	pPEHeader->NumberOfSections -= 1;
	
	//改numofsections

	return pImageBuffer;
}
BYTE* section::enlarge_last_section_filebuffer(BYTE* pFileBuffer, size_t num) {
	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//指向最后一个节

	pOptionHeader->SizeOfImage += num;//num为virtual_size 与 sizeofraw相等

	//修改sizeofimage
	size_t n = pSectionHeader->Misc.VirtualSize > pSectionHeader->SizeOfRawData ? pSectionHeader->Misc.VirtualSize : pSectionHeader->SizeOfRawData;
	n += num;
	pSectionHeader->Misc.VirtualSize = n;
	pSectionHeader->SizeOfRawData = n;
	//修改最后一个节的属性（virtualsize sizeofraw）

	BYTE* pnewbuffer = (BYTE*)malloc(pSectionHeader->PointerToRawData + n);
	memset(pnewbuffer, 0, pSectionHeader->PointerToRawData + n);
	memcpy(pnewbuffer, pFileBuffer, pSectionHeader->PointerToRawData + n - num);
	//申请一个新的空间，复制进去
	return pnewbuffer;
}
BYTE* section::enlarge_last_section_imagebuffer(BYTE* pImageBuffer, size_t num) {
	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//指向最后一个节

	pOptionHeader->SizeOfImage += num;//num为virtual_size 与 sizeofraw相等

	//修改sizeofimage
	size_t n = pSectionHeader->Misc.VirtualSize > pSectionHeader->SizeOfRawData ? pSectionHeader->Misc.VirtualSize : pSectionHeader->SizeOfRawData;
	n += num;
	pSectionHeader->Misc.VirtualSize = n;
	pSectionHeader->SizeOfRawData = n;
	//修改最后一个节的属性（virtualsize sizeofraw）
	BYTE* pnewbuffer = (BYTE*)malloc(pOptionHeader->SizeOfImage);
	memset(pnewbuffer, 0, pOptionHeader->SizeOfImage);
	memcpy(pnewbuffer, pImageBuffer, pOptionHeader->SizeOfImage - num);
	//申请一个新的空间，复制进去
	return pnewbuffer;
	//返回指针
}
//此函数直接传入PIMAGE_SECTION_HEADER是不合理的。如果直接传入的话，必须手动计算virtualaddress以及pointerofraw
BYTE* section::AddSectionHeader_filebuffer(BYTE* pFileBuffer, PIMAGE_SECTION_HEADER pSectionHeader_3) {

	//首先检查空间是否足够
	if (pOptionHeader->SizeOfHeaders - (pDosHeader->e_lfanew + size_pe_sign + size_fileheader + pPEHeader->SizeOfOptionalHeader + pPEHeader->NumberOfSections * size_sectionheader) < 80)
	{
		printf("空间不足，无法直接添加节表");
		return pFileBuffer;
	}
	pPEHeader->NumberOfSections += 1;//修改头部信息
	pOptionHeader->SizeOfImage += pSectionHeader_3->Misc.VirtualSize;

	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//找到节表最后全0的区域
	memcpy(pSectionHeader, pSectionHeader_3, 40);//载入新的节表
	memset(pSectionHeader + 1, 0, 40);//令最后40字节全0
	pSectionHeader--;

	size_t newbuffer_size = pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData + pSectionHeader_3->SizeOfRawData;
	//filebuffer的总大小
	BYTE* newbuffer = (BYTE*)malloc(newbuffer_size);

	memset(newbuffer, 0, newbuffer_size);
	memcpy(newbuffer, pFileBuffer, newbuffer_size- pSectionHeader_3->SizeOfRawData);

	return newbuffer;
}

//此函数直接传入PIMAGE_SECTION_HEADER是不合理的。如果直接传入的话，必须手动计算virtualaddress以及pointerofraw
BYTE* section::AddSectionHeader_imagebuffer(BYTE* pImageBuffer, PIMAGE_SECTION_HEADER pSectionHeader_3) {
	//首先检查空间是否足够
	if (pOptionHeader->SizeOfHeaders - (pDosHeader->e_lfanew + size_pe_sign + size_fileheader + pPEHeader->SizeOfOptionalHeader + pPEHeader->NumberOfSections * size_sectionheader) < 80) 
	{
		printf("空间不足，无法直接添加节表");
		return pImageBuffer;
	}
	pPEHeader->NumberOfSections += 1;
	pOptionHeader->SizeOfImage += pSectionHeader_3->Misc.VirtualSize;


	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);
	memcpy(pSectionHeader, pSectionHeader_3, 40);

	//需要申请一块新的内存
	BYTE* newbuffer = (BYTE*)malloc(pOptionHeader->SizeOfImage);

	memset(newbuffer, 0, pOptionHeader->SizeOfImage);
	memcpy(newbuffer, pImageBuffer, (pOptionHeader->SizeOfImage) - (pSectionHeader_3->Misc.VirtualSize));
	
	return newbuffer;
}