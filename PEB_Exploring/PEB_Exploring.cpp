#include <Windows.h>
#include <stdio.h>

using namespace std;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING;

struct LDR_MODULE {
	LIST_ENTRY e[3];
	HMODULE base;
	void* entry;
	UINT size;
	UNICODE_STRING dllPath;
	UNICODE_STRING dllname;
};

int main() {

	INT_PTR peb = __readgsqword(0x60);
	
	auto InheritedAddressSpace = *(UCHAR*)(peb);
	auto ReadImageFileExecOptions = *(UCHAR*)(peb + 0x1);
	auto BeingDebugged = *(UCHAR*)(peb + 0x2);
	auto ImageBaseAddress = *(INT_PTR*)(peb + 0x10);
	auto Ldr = *(INT_PTR*)(peb + 0x18);
	auto ProcessParameters = *(INT_PTR*)(peb + 0x20);
	
	
	printf("[+] BeingDebugged	\t0x%p\n", BeingDebugged);
	printf("[+] ImageBaseAddress \t0x%p\n", ImageBaseAddress);


	// _PEB_LDR_DATA :
	auto M1flink = *(INT_PTR*)(Ldr + 0x10);
	auto Mdl = (LDR_MODULE*)M1flink;
	printf("[+] Loaded Modules :\n");
	do {
		Mdl = (LDR_MODULE*)Mdl->e[0].Flink;
		if (Mdl->base != NULL) {
			printf("\n\t%ws", Mdl->dllname.Buffer);
			printf("\t0x%p\t (Print just 3 APIs)\n", Mdl->base);
			PIMAGE_DOS_HEADER img_dos_header = (PIMAGE_DOS_HEADER)Mdl->base;
			PIMAGE_NT_HEADERS img_nt_header = (PIMAGE_NT_HEADERS)((LPBYTE)Mdl->base + img_dos_header->e_lfanew);
			PIMAGE_EXPORT_DIRECTORY img_edt = (PIMAGE_EXPORT_DIRECTORY)(
				(LPBYTE)Mdl->base + img_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			PDWORD funcAddr = (PDWORD)((LPBYTE)Mdl->base + img_edt->AddressOfFunctions);
			PDWORD funcNames = (PDWORD)((LPBYTE)Mdl->base + img_edt->AddressOfNames);
			PWORD  funcOrd = (PWORD)((LPBYTE)Mdl->base + img_edt->AddressOfNameOrdinals);
			for (DWORD i = 0; i < img_edt->AddressOfFunctions; i++) {
				LPSTR pFuncName = (LPSTR)((LPBYTE)Mdl->base + funcNames[i]);
				printf("\t\t%s\t0x%p\n", pFuncName, (LPVOID)((LPBYTE)Mdl->base + funcAddr[funcOrd[i]]));
				Sleep(10);

				if (i == 2) {
					break;
				}
			}
		}
	} while (M1flink != (INT_PTR)Mdl);
	printf("\n");

	

	// _RTL_USER_PROCESS_PARAMETERS :
	printf("[+] CurrentDirectory \t%ws\n", *(INT_PTR*)(ProcessParameters + 0x38 + 0x8));
	printf("[+] ImagePathName \t%ws\n", *(INT_PTR*)(ProcessParameters + 0x60 + 0x8));
	printf("[+] CommandLine \t%ws\n", *(INT_PTR*)(ProcessParameters + 0x70 + 0x8));
	printf("[+] WindowTitle \t%ws\n", *(INT_PTR*)(ProcessParameters + 0xb0 + 0x8));
	printf("[+] DesktopInfo \t%ws\n", *(INT_PTR*)(ProcessParameters + 0xc0 + 0x8));
	
	// Environment Variables 
	printf("\n[+] Environment Variables:\n\n");
	auto Env = *(INT_PTR*)(ProcessParameters + 0x80);
	auto EnvSize = *(INT*)(ProcessParameters + 0x3f0);

	for (int i = 0; i < EnvSize; i++) {
		printf("%c", *(CHAR*)(Env+i));
		i++;
	}

	

	return 0;

}