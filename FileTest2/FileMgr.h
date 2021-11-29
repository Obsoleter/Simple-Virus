#pragma once


DWORD AddSection(
	PIMAGE_NT_HEADERS32 NTHeader, //PE Header.
	std::fstream& file, //FILE.
	DWORD SectionRAW, //Raw pointer to section's headers.
	const char* filename,
	DWORD& SizeOfFile,
	void* CallBack
);
void ExpandFileForSectionHeader(
	PIMAGE_NT_HEADERS32 NTHeader, 
	std::fstream& file,
	DWORD SectionRAW,
	const char* filename,
	DWORD& SizeOfFile
);
void ExpandFileForSection(
	PIMAGE_NT_HEADERS32 NTHeader, 
	std::fstream& file,
	DWORD& SizeOfFile
);
void AddSectionHeader(
	PIMAGE_NT_HEADERS32 NTHeader,
	std::fstream& file,
	DWORD SectionRAW, 
	void* CallBack
);


void SetUpFunctions(
	std::fstream& file,
	DWORD SectionRAW,
	DWORD RVA1,
	DWORD RVA2,
	DWORD* funcs, 
	DWORD ImportTableRAW,
	DWORD* PrevRVA,
	DWORD SectionNum
);


DWORD GetRAW(std::fstream& file, DWORD adr, DWORD SectionRAW, DWORD SectionNum);
