#pragma once


DWORD AddSection64(
	PIMAGE_NT_HEADERS64 NTHeader, //PE Header.
	std::fstream& file, //FILE.
	DWORD SectionRAW, //Raw pointer to section's headers.
	const char* filename,
	DWORD& SizeOfFile,
	void* CallBack
);
void ExpandFileForSectionHeader64(
	PIMAGE_NT_HEADERS64 NTHeader,
	std::fstream& file,
	DWORD SectionRAW,
	const char* filename,
	DWORD& SizeOfFile
);
void ExpandFileForSection64(
	PIMAGE_NT_HEADERS64 NTHeader,
	std::fstream& file,
	DWORD& SizeOfFile
);
void AddSectionHeader64(
	PIMAGE_NT_HEADERS64 NTHeader,
	std::fstream& file,
	DWORD SectionRAW,
	void* CallBack
);


void SetUpFunctions64(
	std::fstream& file,
	DWORD SectionRAW,
	DWORD64 RVA1,
	DWORD64 RVA2,
	PDWORD funcs,
	DWORD ImportTableRAW,
	PDWORD64 PrevRVA,
	DWORD SectionNum
);


