#include <iostream>
#include <Windows.h>
#include <string>
constexpr int adler_mod =65521;

//Funcion hash
int adler32(std::string func_name)
{
	int A = 1;
	int B = 0;

	for(int i = 0; i < func_name.size(); i++)
	{
		A = (A + static_cast<int>(func_name[i])) % adler_mod;
		B = (B + A) % adler_mod;
	}

	int result = (B << 16) + A;
	return result;
}


PDWORD LookupHashedFunc(const char* library, int hash)
{
	//Cargamos la libreria
	auto libHandle = LoadLibraryA(library);
	if (!libHandle) {
		std::cout << "Could not find library" << std::endl;
		exit(1);
	}
	
	//Obtenemos sus encabezados
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)libHandle;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)libHandle + dosHeader->e_lfanew);

	//Obtenemos la tabla de exportaciones
	DWORD exportDirAddr = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY) ((DWORD_PTR) libHandle + exportDirAddr);
	
	//Obtenemos punteros a las funciones
	PDWORD functionsAddr = (PDWORD) ((DWORD_PTR) libHandle + exportDir->AddressOfFunctions);
	PDWORD functionsNamesAddr = (PDWORD) ((DWORD_PTR) libHandle + exportDir->AddressOfNames);
	PWORD functionsNamesOrdAddr = (PWORD) ((DWORD_PTR) libHandle + exportDir->AddressOfNameOrdinals);

	//Iteramos por cada funcion
	for (DWORD i = 0; i < exportDir->NumberOfFunctions; i++) {
		
		DWORD functionNameAddr = functionsNamesAddr[i];
		DWORD_PTR func = (DWORD_PTR) libHandle+ functionNameAddr;

		char* funcName = (char*)func;
		
		//Calculamos el hash del nombre de la funcion
		int funcHash = adler32(std::string(funcName));
		
		//Si coincide, obtenemos la direcion y la devolvemos
		if (funcHash == hash) {
			DWORD_PTR funcAddressRVA = functionsAddr[functionsNamesOrdAddr[i]];
			PDWORD funcAddress = PDWORD ((DWORD_PTR) libHandle + funcAddressRVA);
			return funcAddress;
		}
	}

	return 0;
}

//Prototipo de la funcion MessageBoxA
using messageBoxFunc = int(NTAPI*)(
	HWND   hWnd,
	LPCSTR lpText,
	LPCSTR lpCaption,
	UINT   uType
);


int main(int argc,char** argv)
{
	//Introducimos el nombre del modulo y el hash de la funcion
	PDWORD lookup_func = LookupHashedFunc("User32", 0x197f0430);
	if (lookup_func == 0) {
		std::cout << "Could not find hashed function" << std::endl;
		return 1;
	}
	
	//Convertimos el puntero a un puntero de la funcion MessageBoxA
	messageBoxFunc func = (messageBoxFunc)lookup_func;
	
	//Ejecutamos finalmente la funcion
	func(0, "Soy un fantasma!", "Boo!", 0);


	return 0;
}
