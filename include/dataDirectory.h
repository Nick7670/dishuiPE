#ifndef _DATADIRECTORY_H
#define _DATADIRECTORY_H
#include <windows.h>
#include <cstring>

class export: public table {
public:
    export() = default;
    export(BYTE* ptr);
    export(string dir);
    ~export();
    DWORD PrintExport();
    DWORD MoveExport_FileBuffer();
private:
    PIMAGE_EXPORT_DIRECTORY tab;
};

class import: public table {
public:
    import() = default;
    import(BYTE* ptr);
    import(string dir);
    ~import();
    DWORD PrintImport();
    DWORD PrintBoundImport();
    DWORD IejectImport(PIMAGE_IMPORT_DESCRIPTOR rejecttab, ThreePoint* name);
    DWORD* GetFunctionAddrByName(BYTE* name);
    DWORD* GetFunctionAddrByOrdinals(DWORD num);
private:
    PIMAGE_IMPORT_DESCRIPTOR tab;
};

class relocation: public table {
public:
    relocation() = default;
    relocation(BYTE* ptr);
    relocation(string dir);
    ~relocation();
    DWORD PrintRelocation();
    DWORD MoveRelocation_FileBuffer();
private:
    PIMAGE_BASE_RELOCATION tab;
};


class resource: public table {
public:
    resource() = default;
    resource(BYTE* ptr);
    resource(string dir);
    ~resource();
    DWORD printResource(BYTE type,BYTE* name);
private:
    PIMAGE_RESOURCE_DIRECTORY tab;
};


#endif
