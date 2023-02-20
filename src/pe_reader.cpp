#include <iostream>
#include <windows.h>
#include <cstring>

#define size_dos 64
#define size_pe_sign 4
#define size_fileheader 20
#define size_sectionheader 40
using namespace std;
typedef struct _ThreePoint {
	BYTE* first;
	BYTE* second;
	BYTE* third;
}ThreePoint;

int main() {
	string dir;
	cout << "please input file path" << endl;
	cin >> dir;
	dosHeader dos(dir);
	dos.PrintPEInfo();
	return 0;
}
