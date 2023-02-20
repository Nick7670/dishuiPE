
#include "dataDirectory.h"



export::export(BYTE* ptr) : table(ptr) {
    size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[0].VirtualAddress);
	tab = PIMAGE_EXPORT_DIRECTORY(pFileBuffer + FileAddress);
}
export::export(string dir) : table(ptr) {
	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[0].VirtualAddress);
	tab = PIMAGE_EXPORT_DIRECTORY(pFileBuffer + FileAddress);
}
export::~export() {
    tab = nullptr;
}
DWORD export::PrintExport() {
	
	cout <<  hex << "Characteristics:  " << tab->Characteristics << endl;
	cout <<  "TimeDateStamp:  " << tab->TimeDateStamp << endl;
	cout <<  "MajorVersion:  " << tab->MajorVersion << endl;
	cout <<  "Name:  " << tab->Name << endl;
	cout <<  "Base:  " << tab->Base << endl;
	cout <<  "NumberOfFunctions:  " << tab->NumberOfFunctions << endl;
	cout <<  "NumberOfNames:  " << tab->NumberOfNames << endl;
	cout <<  "AddressOfFunctions:  " << tab->AddressOfFunctions << endl;
	cout <<  "AddressOfNames:  " << tab->AddressOfNames << endl;
	cout <<  "AddressOfNameOrdinals:  " << tab->AddressOfNameOrdinals << dec << endl;

	DWORD* pFunTable = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfFunctions));
	DWORD* pNameTable = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNames));
	WORD* pNameOrdinalsTable = (WORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNameOrdinals));

	cout << "输出Functions" << endl;
	for (size_t i = 0; i < tab->NumberOfFunctions; i++) {
		//printf("%x\n", *(DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, (*pFunTable))));
		printf("%x\n", *pFunTable);
		pFunTable++;
	}

	cout << "输出Names" << endl;
	BYTE* mid = nullptr;
	for (size_t i = 0; i < tab->NumberOfNames; i++) {
		printf("%x\n", *pNameTable);
		mid = (pFileBuffer + RVA_FOA(pFileBuffer, (*pNameTable)));
		while (*mid != 0) {
			printf("%x", *mid);
			mid++;
		}
		cout << endl;
		pNameTable++;
	}
	cout << "输出NameOrdinals" << endl;
	for (size_t i = 0; i < tab->NumberOfNames; i++) {
		printf("%x\n", *pNameOrdinalsTable);
		pNameOrdinalsTable++;
	}
	mid = nullptr;
	return 1;
}
DWORD export::MoveExport_FileBuffer() 
{

	//将三个表拷到新增节里面
	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//找到新的添加的节表
	DWORD* Addr = (DWORD*)(pFileBuffer + pSectionHeader->PointerToRawData);//文件中的起始地址

	DWORD* Addr_function = &(*(DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfFunctions)));
	DWORD* Addr_name = &(*(DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNames)));
	WORD* Addr_nameOrd = &(*(WORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNameOrdinals)));

	memcpy(Addr, Addr_function, 4 * tab->NumberOfFunctions);
	memcpy(Addr + tab->NumberOfFunctions, Addr_name, 4 * tab->NumberOfNames);
	memcpy((WORD*)(Addr + tab->NumberOfFunctions + tab->NumberOfNames), Addr_nameOrd, 2 * tab->NumberOfNames);
	//将名字表的名字拷入新的节里面，同时修正名字表中的地址
	BYTE* name = (BYTE*)((WORD*)(Addr + tab->NumberOfFunctions + tab->NumberOfNames) + tab->NumberOfNames);
	size_t flag = 0;
	DWORD* change_addr_name = Addr + tab->NumberOfFunctions;
	size_t size_allnames = 0;
	for (size_t i = 0; i < tab->NumberOfNames; i++) {
		BYTE* mid = (pFileBuffer + RVA_FOA(pFileBuffer, (*Addr_name)));
		*change_addr_name = pSectionHeader->VirtualAddress + 8 * tab->NumberOfFunctions + 2 * tab->NumberOfNames + size_allnames;
		//FOA_RVA(pFileBuffer,(DWORD)&(*name)-(DWORD)pFileBuffer);
		while (flag!=1) {
			if (*mid == 0) {
				flag = 1;
			}
			memcpy(name, mid, 1);
			mid++;
			name++;
			size_allnames++;
		}
		flag = 0;
		Addr_name++;
		change_addr_name++;
	}
	//拷贝IMAGE_EXPORT_DIRECTORY
	memcpy(name, tab, 40);
	//修正IMAGE_EXPORT_DIRECTORY中三个表的地址
	PIMAGE_EXPORT_DIRECTORY tab_2 = (PIMAGE_EXPORT_DIRECTORY)(name);
	tab_2->AddressOfFunctions = pSectionHeader->VirtualAddress;
	tab_2->AddressOfNames = pSectionHeader->VirtualAddress + 4 * tab->NumberOfFunctions;
	tab_2->AddressOfNameOrdinals = pSectionHeader->VirtualAddress + 8 * tab->NumberOfFunctions;

	//修改可选pe头中的地址
	pOptionHeader->DataDirectory[0].VirtualAddress = pSectionHeader->VirtualAddress + 8 * tab->NumberOfFunctions + 2 * tab->NumberOfNames + size_allnames;
	return 8 * tab->NumberOfFunctions+2*tab->NumberOfNames+size_allnames+40;
}




import::import(BYTE* ptr) : table(ptr) {

	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[1].VirtualAddress);
	tab = PIMAGE_IMPORT_DESCRIPTOR(pFileBuffer + FileAddress);
}

import::import(string dir) : table(dir) {
	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[1].VirtualAddress);
	tab = PIMAGE_IMPORT_DESCRIPTOR(pFileBuffer + FileAddress);
}

import::~import() {
    tab = nullptr;
}

DWORD import::PrintImport() {
	DWORD flag_1 = 0x80000000;
	DWORD flag_2 = 0x7fffffff;
	DWORD mid_1 = 0;
	BYTE* mid_2 = nullptr;

	cout << hex;
	while (tab->FirstThunk != 0 && tab->OriginalFirstThunk != 0) {
		DWORD* original = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->OriginalFirstThunk));
		DWORD* firstthunk = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->FirstThunk));
		while (*original!=0) {
			if (*original & flag_1 == flag_1) {//最高位为1
				//取出低31位为函数序号
				mid_1 = *original & flag_2;
				cout << "函数序号为： " << mid_1 << endl;
				original++;
			} else {
				PIMAGE_IMPORT_BY_NAME tab_2 = (PIMAGE_IMPORT_BY_NAME)(pFileBuffer + RVA_FOA(pFileBuffer, *original));
				mid_2 = (BYTE*)&(tab_2->Name);
				cout << "函数名字为： ";
				while (*mid_2 != 0) {
					printf("%c",*mid_2);
					mid_2++;
				}
				cout << endl
				original++;
			}

		}
		cout << "TimeDateStamp: " << tab->TimeDateStamp << endl;
		cout << "ForwarderChain: " << tab->ForwarderChain << endl;

		cout << "Name: ";
		BYTE* name = (BYTE*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->Name));
		while (*name != 0) {
			printf("%c", *name);
			name++;
		}
		cout << endl;

		while (*firstthunk != 0) {
			if (*firstthunk & flag_1 == flag_1) {//最高位为1
				//取出低31位为函数序号
				mid_1 = *firstthunk & flag_2;
				cout <<  "函数序号为： " << mid_1 << endl;
				firstthunk++;
			} else {
				PIMAGE_IMPORT_BY_NAME tab_2 = (PIMAGE_IMPORT_BY_NAME)(pFileBuffer + RVA_FOA(pFileBuffer, *firstthunk));
				mid_2 = (BYTE*)&(tab_2->Name);
				cout << "函数名字为： ";
				while (*mid_2 != 0) {
					printf("%c", *mid_2);
					mid_2++;
				}
				cout << endl;
				firstthunk++;
			}
		}
		tab++;
		cout << "------------------------------------" << endl;
	}
	return 1;
}
DWORD import::PrintBoundImport() {
	IMAGE_DATA_DIRECTORY bound = pOptionHeader->DataDirectory[11];
	size_t FileAddress = RVA_FOA(pFileBuffer, bound.VirtualAddress);
	PIMAGE_BOUND_IMPORT_DESCRIPTOR btab = PIMAGE_BOUND_IMPORT_DESCRIPTOR(pFileBuffer + FileAddress);
	PIMAGE_BOUND_IMPORT_DESCRIPTOR btab_1 = PIMAGE_BOUND_IMPORT_DESCRIPTOR(pFileBuffer + FileAddress);
	PIMAGE_BOUND_FORWARDER_REF btab_2 = PIMAGE_BOUND_FORWARDER_REF(pFileBuffer + FileAddress);
	size_t i = 0;

	while (btab_1->NumberOfModuleForwarderRefs != 0 && btab_1->OffsetModuleName != 0) {
		cout <<  "TimeDateStamp: " << btab_1->TimeDateStamp << endl;
		cout <<  "OffsetModuleName: " << (DWORD)btab + btab->OffsetModuleName << endl;
		cout <<  "NumberOfModuleForwarderRefs: " << btab_1->NumberOfModuleForwarderRefs << endl;
		btab_2 = (PIMAGE_BOUND_FORWARDER_REF)btab_1 + 1;
		while (i < btab_1->NumberOfModuleForwarderRefs) {
			cout << "TimeDateStamp: " << btab_2->TimeDateStamp << endl;
			cout <<  "OffsetModuleName" << (DWORD)btab + btab_2->OffsetModuleName << endl;
			cout <<  "Reserved: " << btab_2->Reserved << endl;
			btab_2++;
		}
		btab_1++;
	}
	return 1;
	/*
	typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
		DWORD   TimeDateStamp;
		WORD    OffsetModuleName;
		WORD    NumberOfModuleForwarderRefs;
		// Array of zero or more IMAGE_BOUND_FORWARDER_REF follows
	} IMAGE_BOUND_IMPORT_DESCRIPTOR, * PIMAGE_BOUND_IMPORT_DESCRIPTOR;

	typedef struct _IMAGE_BOUND_FORWARDER_REF {
		DWORD   TimeDateStamp;
		WORD    OffsetModuleName;
		WORD    Reserved;
	} IMAGE_BOUND_FORWARDER_REF, * PIMAGE_BOUND_FORWARDER_REF;
*/
}
DWORD import::IejectImport(PIMAGE_IMPORT_DESCRIPTOR rejecttab, ThreePoint* name) {

	pFileBuffer = enlarge_last_section_filebuffer(1000);
	BYTE* first = name->first;
	BYTE* second = name->second;
	BYTE* third = name->third;
	DWORD first_len = getstr(first);
	DWORD second_len = getstr(second);
	DWORD third_len = getstr(third);

	IMAGE_DATA_DIRECTORY import = pOptionHeader->DataDirectory[1];
	size_t FileAddress = RVA_FOA(pFileBuffer, import.VirtualAddress);
	PIMAGE_IMPORT_DESCRIPTOR tab = PIMAGE_IMPORT_DESCRIPTOR(pFileBuffer + FileAddress);

	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//指向最后一个节
	//在扩大节中找到导入应该拷贝的地方
	BYTE* pnew = pFileBuffer + (pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData - 1000);

	memcpy(pnew, tab, import.Size);//将所有导入表拷贝到扩大节中
	PIMAGE_IMPORT_DESCRIPTOR pnewtab = (PIMAGE_IMPORT_DESCRIPTOR)pnew;
	pnewtab += import.Size / 20 - 1;//指向全零的导入表
	memcpy(pnewtab, rejecttab, 20);//拷入新的导入表
	memset(pnewtab + 1, 0, 20);//将最后20字节赋值为全零
	
	//1324000012340000xxxxxx0xxxxxx0
	//构造IAT INT NAME 表
	DWORD* AllName = (DWORD*)(pnewtab + 2);
	*AllName = (DWORD)(AllName + 4);//修改IAT表中的第一个地址
	*(AllName + 2) = (DWORD)(AllName + 4);//修改INT表中的第一个地址与IAT表中第一个地址相同
	BYTE* IAT_Name = (BYTE*)AllName;
	memcpy(IAT_Name + 2, first, first_len + 1);//
	memcpy(IAT_Name + first_len + 3, third, third_len + 1);
	//memcpy(IAT_Name + first_len + second_len + 6, third, third_len + 1);


	//对注入的dll的导入表进行赋值
	pnewtab->OriginalFirstThunk = FOA_RVA(pFileBuffer, (BYTE)(pnewtab + 2) - (BYTE)pFileBuffer);
	pnewtab->FirstThunk = FOA_RVA(pFileBuffer, (BYTE)(pnewtab + 2) - (BYTE)pFileBuffer + 8);
	pnewtab->Name = FOA_RVA(pFileBuffer, (BYTE)(pnewtab + 2) - (BYTE)pFileBuffer + 16 + 2 + first_len + 1);

	pOptionHeader->DataDirectory[1].VirtualAddress = FOA_RVA(pFileBuffer, (DWORD)(pnew - (BYTE)pFileBuffer));
	pOptionHeader->DataDirectory[1].Size += 20;
	return 1;

}


DWORD* import::GetFunctionAddrByName(BYTE* name) 
{
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((BYTE*)pFileBuffer + *(((DWORD*)pFileBuffer) + 15));
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) & (pNTHeader->OptionalHeader);

	IMAGE_DATA_DIRECTORY exp = pOptionHeader->DataDirectory[0];
	size_t FileAddress = RVA_FOA(pFileBuffer, exp.VirtualAddress);
	PIMAGE_EXPORT_DIRECTORY tab = PIMAGE_EXPORT_DIRECTORY(pFileBuffer + FileAddress);

	DWORD* pFunTable = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfFunctions));
	WORD* pNameOrdinalsTable = (WORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNames));
	DWORD* pNameTable = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfNames));//里面存的都是名字的RVA

	for (size_t i = 0; i < tab->NumberOfNames; i++) {
		BYTE* mid = (pFileBuffer + RVA_FOA(pFileBuffer, (*(pNameTable+i))));
		size_t flag_1 = 0;
		size_t flag_2 = 0;
		size_t flag_3 = 0;
		BYTE* mid_1 = mid;
		BYTE* mid_2 = name;
		//1 首先判断长度是否相等

		while (*mid_1 != 0) {
			flag_1++;
			mid_1++;
		}
		while (*mid_2 != 0) {
			flag_2++;
			mid_2++;
		}
		//2 判断每个字符是否都相等
		if (flag_1 == flag_2) {
			mid_1 = mid;
			mid_2 = name;
			for (size_t i = 0; i < flag_1; i++) {
				if (*mid_1 == *mid_2) flag_3++;
				mid_1++;
				mid_2++;
			}
			if (flag_1 == flag_3) {
				//说明名字相等
				pNameOrdinalsTable += i;
				pFunTable += *pNameOrdinalsTable;
				return pFunTable;

			}
		}
	}
	return 0;
}
DWORD* import::GetFunctionAddrByOrdinals(DWORD num) 
{
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((BYTE*)pFileBuffer + *(((DWORD*)pFileBuffer) + 15));
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) & (pNTHeader->OptionalHeader);

	IMAGE_DATA_DIRECTORY exp = pOptionHeader->DataDirectory[0];
	size_t FileAddress = RVA_FOA(pFileBuffer, exp.VirtualAddress);
	PIMAGE_EXPORT_DIRECTORY tab = PIMAGE_EXPORT_DIRECTORY(pFileBuffer + FileAddress);

	size_t index = num - tab->Base;
	DWORD* FunctionAddr = (DWORD*)(pFileBuffer + RVA_FOA(pFileBuffer, tab->AddressOfFunctions));
	FunctionAddr += index;
	return FunctionAddr;
}


relocation::relocation(BYTE* ptr) : talbe(ptr) {

	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[5].VirtualAddress);
	PIMAGE_BASE_RELOCATION tab = PIMAGE_BASE_RELOCATION(pFileBuffer + FileAddress);
}

relocation::reloaction(string dir) : table(dir) {

	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[5].VirtualAddress);
	PIMAGE_BASE_RELOCATION tab = PIMAGE_BASE_RELOCATION(pFileBuffer + FileAddress);
}

relocation::~relocation() {
	tab = nullptr;
}

DWORD relocation::PrintRelocation() {


	BYTE inital_1 = 0xF0;
	WORD inital_2 = 0xFFF;
	BYTE mid_1 = 0;
	WORD mid_2 = 0;
	BYTE* high_one = (BYTE*)tab + 9;
	WORD* low_three = (WORD*)tab + 4;
	while (tab->VirtualAddress != 0 || tab->SizeOfBlock != 0)
	{
		printf("%s%x\n", "******************virturaladdress******************: ", tab->VirtualAddress);
		printf("%s%x\n", "******************sizeofbolck******************:  ", tab->SizeOfBlock);
		size_t num_items = (tab->SizeOfBlock - 8) / 2;
		for (size_t i = 0; i < num_items; i++) {
			mid_1 = *high_one;
			mid_1 = (mid_1 & inital_1) >> 4;
			if (mid_1 == 3) {
				mid_2 = *low_three;
				mid_2 = mid_2 & inital_2;
				printf("%s%x\n", "低12位:  ", mid_2);
			}	
			if (i==num_items-1) {
				printf("%s%x\n", "低12位:  ", 0);
				cout << "此block已结束" << endl;
			}
			high_one += 2;
			low_three++;
		}

		tab = PIMAGE_BASE_RELOCATION((BYTE*)tab+tab->SizeOfBlock);
		high_one = (BYTE*)tab + 9;
		low_three = (WORD*)tab + 4;
	}
	return 1;
}
DWORD reloction::MoveRelocation_FileBuffer(BYTE* pFileBuffer) {

	size_t num_sections = pPEHeader->NumberOfSections;
	pSectionHeader = pSectionHeader + (num_sections - 1);//找到新的添加的节表
	DWORD* Addr = (DWORD*)(pFileBuffer + pSectionHeader->PointerToRawData);//文件中的起始地址

	memcpy(Addr, tab, exp.Size);

	pOptionHeader->DataDirectory[5].VirtualAddress = pSectionHeader->VirtualAddress;
	return exp.Size;
}

resource::resource(BYTE* ptr) : table(ptr) {
	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[2].VirtualAddress);
	PIMAGE_RESOURCE_DIRECTORY tab = PIMAGE_RESOURCE_DIRECTORY(pFileBuffer + FileAddress);
}

resource::resource(string dir) : table(dir) {
	size_t FileAddress = RVA_FOA(pFileBuffer, pOptionHeader->DataDirectory[2].VirtualAddress);
	PIMAGE_RESOURCE_DIRECTORY tab = PIMAGE_RESOURCE_DIRECTORY(pFileBuffer + FileAddress);
}

resource::~resource() {
	tab = nullptr;
}

DWORD reource::printResource(BYTE* pFileBuffer,BYTE type,BYTE* name) {

	DWORD baseAddr = resource.VirtualAddress;
	//根据给定的资源类型，在第一层中找到对应的项
	DWORD num_resource_1 = tab->NumberOfIdEntries + tab->NumberOfNamedEntries;
	DWORD num_resource_2 = 0;
	DWORD num_resource_3 = 0;
	PIMAGE_RESOURCE_DIRECTORY next_addr_1 = nullptr;
	PIMAGE_RESOURCE_DIRECTORY next_addr_2 = nullptr;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pitem_1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(tab+1);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pitem_2 = nullptr;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pitem_3 = nullptr;
	PIMAGE_DATA_DIRECTORY data = nullptr;
	for (DWORD i = 0; i < num_resource_1; i++) {
		//判断资源类型是否为数字
		if (pitem_1->NameIsString == 0) {
			//与给定的类型进行比较
			if (pitem_1->Id == type) {
				next_addr_1 = (PIMAGE_RESOURCE_DIRECTORY)(pFileBuffer + RVA_FOA(pFileBuffer, baseAddr + pitem_1->OffsetToDirectory));
				//开始进入第二层
				num_resource_2 = next_addr_1->NumberOfIdEntries + next_addr_1->NumberOfNamedEntries;
				pitem_2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(next_addr_1 + 1);
				
				for (DWORD n = 0; n < num_resource_2; n++) {
					next_addr_2 = (PIMAGE_RESOURCE_DIRECTORY)(pFileBuffer + RVA_FOA(pFileBuffer, baseAddr + pitem_2->OffsetToDirectory));
					num_resource_3 = next_addr_2->NumberOfIdEntries + next_addr_2->NumberOfNamedEntries;

					pitem_3= (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(next_addr_2 + 1);
					for (DWORD m = 0; m < num_resource_3; m++) {
						data = (PIMAGE_DATA_DIRECTORY)(pFileBuffer + RVA_FOA(pFileBuffer, baseAddr + pitem_3->OffsetToDirectory));
						printf("%s%d\n", "ID: ", pitem_2->Id);
						printf("%s%x\n", "VirtualAddress: ", data->VirtualAddress);
						printf("%s%x\n", "Size: ", data->Size);
						pitem_3++;
					}
					pitem_2++;
				}
			}
		}
		//判断资源类型是否为字符
		else 
		{
			
		}
		pitem_1++;
	
	}

	//在对应第二层的项中，循环打印资源的信息
	return 0;
}