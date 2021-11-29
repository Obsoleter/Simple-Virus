#include "pch.h"
using namespace std;


DWORD AddSection64(PIMAGE_NT_HEADERS64 NTHeader, fstream& file, DWORD SectionRAW, const char* filename, DWORD& SizeOfFile, void* CallBack)
{
	DWORD RAW = SectionRAW;
	RAW += NTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	if ((NTHeader->OptionalHeader.SizeOfHeaders - RAW) < sizeof(IMAGE_SECTION_HEADER))
		ExpandFileForSectionHeader64(NTHeader, file, SectionRAW, filename, SizeOfFile);
	DWORD NewSection = SectionRAW + NTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	NTHeader->FileHeader.NumberOfSections++;
	NTHeader->OptionalHeader.SizeOfImage += NTHeader->OptionalHeader.SectionAlignment;
	NTHeader->OptionalHeader.SizeOfInitializedData += NTHeader->OptionalHeader.FileAlignment;
	NTHeader->FileHeader.Characteristics |= 0x0001;
	file.seekp(SectionRAW - sizeof(IMAGE_NT_HEADERS64));
	file.write((char*)NTHeader, sizeof(IMAGE_NT_HEADERS64));
	AddSectionHeader64(NTHeader, file, SectionRAW, CallBack);
	ExpandFileForSection64(NTHeader, file, SizeOfFile);
	return NewSection;
}









void ExpandFileForSectionHeader64(PIMAGE_NT_HEADERS64 NTHeader, fstream& file, DWORD SectionRAW, const char* filename, DWORD& SizeOfFile)
{
	file.seekg(0);
	fstream file2;
	file2.open("FileMgr.time", ios::out | ios::in | ios::binary | ios::app);
	char c;
	int i = 0;
	for (; i < NTHeader->OptionalHeader.SizeOfHeaders; i++)
	{
		file.read(&c, 1);
		file2.write(&c, 1);
	}
	c = 0;
	for (int k = 0; k < NTHeader->OptionalHeader.FileAlignment; k++) file2.write(&c, 1);
	for (; i < SizeOfFile; i++)
	{
		file.read(&c, 1);
		file2.write(&c, 1);
	}
	file2.close();
	file.close();
	remove(filename);
	rename("FileMgr.time", filename);
	file.open(filename, ios::out | ios::in | ios::binary | ios::_Nocreate | ios::ate);
	NTHeader->OptionalHeader.SizeOfHeaders += NTHeader->OptionalHeader.FileAlignment;
	IMAGE_SECTION_HEADER SectionHeader;
	for (int k = 0; k < NTHeader->FileHeader.NumberOfSections; k++)
	{
		file.seekg(SectionRAW + sizeof(IMAGE_SECTION_HEADER)*k);
		file.read((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
		SectionHeader.PointerToRawData += NTHeader->OptionalHeader.FileAlignment;
		file.seekp(SectionRAW + sizeof(IMAGE_SECTION_HEADER)*k);
		file.write((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
	}
	SizeOfFile += NTHeader->OptionalHeader.FileAlignment;
}




void ExpandFileForSection64(PIMAGE_NT_HEADERS64 NTHeader, fstream& file, DWORD& SizeOfFile)
{
	char c = 0;
	file.seekp(SizeOfFile);
	for (int k = 0; k < NTHeader->OptionalHeader.FileAlignment * 5; k++) file.write(&c, 1);
	SizeOfFile += NTHeader->OptionalHeader.FileAlignment * 5;
}




void AddSectionHeader64(PIMAGE_NT_HEADERS64 NTHeader, fstream& file, DWORD SectionRAW, void* CallBack)
{
	IMAGE_SECTION_HEADER SectionHeader;
	DWORD RAW = SectionRAW + (NTHeader->FileHeader.NumberOfSections - 2) * sizeof(IMAGE_SECTION_HEADER);
	file.seekg(RAW);
	file.read((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
	SectionHeader.PointerToRawData += SectionHeader.SizeOfRawData;
	SectionHeader.SizeOfRawData = NTHeader->OptionalHeader.FileAlignment * 5;
	SectionHeader.VirtualAddress += (1 + SectionHeader.Misc.VirtualSize / NTHeader->OptionalHeader.SectionAlignment) * NTHeader->OptionalHeader.SectionAlignment;
	SectionHeader.Misc.VirtualSize = NTHeader->OptionalHeader.FileAlignment * 5;
	typedef void(*_SectionCALLBACK)(PIMAGE_NT_HEADERS64, PIMAGE_SECTION_HEADER);
	_SectionCALLBACK SectionCALLBACK = (_SectionCALLBACK)CallBack;
	SectionCALLBACK(NTHeader, &SectionHeader);
	file.seekp(SectionRAW + sizeof(IMAGE_SECTION_HEADER)*(NTHeader->FileHeader.NumberOfSections - 1));
	file.write((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
}






void SetUpFunctions64(fstream& file, DWORD SectionRAW, DWORD64 RVA1, DWORD64 RVA2, PDWORD funcs, DWORD ImportTableRAW, PDWORD64 PrevRVA, DWORD SectionNum)
{
	IMAGE_IMPORT_DESCRIPTOR d;
	DWORD RAW;
	bool iskernel = 1;
	char mod[] = "KERNEL32", modname[8];
	RAW = ImportTableRAW;
	while (1)
	{
		file.seekg(RAW);
		file.read((char*)&d, sizeof(d));
		if (d.FirstThunk == 0) break;
		file.seekg(GetRAW(file, d.Name, SectionRAW, SectionNum));
		file.read(modname, 8);
		for (int k = 0; k < 8; k++)
		{
			if (modname[k] != mod[k])
			{
				iskernel = 0;
				k = 8;
			}
		}
		if (iskernel)
		{
			cout << "OPA!" << endl;
			RAW = GetRAW(file, d.FirstThunk, SectionRAW, SectionNum);
			funcs[0] = d.FirstThunk;
			file.seekg(RAW);
			file.read((char*)&PrevRVA[0], 8);
			file.seekp(RAW);
			file.write((char*)&RVA1, 8);

			RAW = GetRAW(file, d.OriginalFirstThunk, SectionRAW, SectionNum);
			file.seekp(RAW);
			file.write((char*)&RVA1, 8);
			//file.seekg(GetRAW(file, PrevRVA[0], SectionRAW, SectionNum));
			//file.read((char*)&hint[0], 2);

			//file.seekp(GetRaw(name));
			//file.write((char*)&hint[0], 2);
			//file.write(func1, 16);



			RAW = GetRAW(file, d.FirstThunk, SectionRAW, SectionNum) + 8;
			funcs[1] = d.FirstThunk + 8;
			file.seekg(RAW);
			file.read((char*)&PrevRVA[1], 8);
			file.seekp(RAW);
			file.write((char*)&RVA2, 8);

			RAW = GetRAW(file, d.OriginalFirstThunk, SectionRAW, SectionNum) + 8;
			file.seekp(RAW);
			file.write((char*)&RVA2, 8);
			//file.seekg(GetRaw(PrevRVA[1]));
			//file.read((char*)&hint[1], 2);

			//file.seekp(GetRaw(name));
			//file.write((char*)&hint[1], 2);
			//file.write(func2, 14);


			break;
		}
		else iskernel = 1;
		cout << modname << endl;
		RAW += sizeof(d);
	}

}