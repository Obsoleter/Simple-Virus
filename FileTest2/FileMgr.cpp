#include "pch.h"
using namespace std;


DWORD AddSection(PIMAGE_NT_HEADERS32 NTHeader, fstream& file, DWORD SectionRAW, const char* filename, DWORD& SizeOfFile, void* CallBack)
{
	DWORD RAW = SectionRAW;
	RAW += NTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	if ((NTHeader->OptionalHeader.SizeOfHeaders - RAW) < sizeof(IMAGE_SECTION_HEADER))
		ExpandFileForSectionHeader(NTHeader, file, SectionRAW, filename, SizeOfFile);
	DWORD NewSection = SectionRAW + NTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	NTHeader->FileHeader.NumberOfSections++;
	NTHeader->OptionalHeader.SizeOfImage += NTHeader->OptionalHeader.SectionAlignment;
	NTHeader->OptionalHeader.SizeOfInitializedData += NTHeader->OptionalHeader.FileAlignment;
	NTHeader->FileHeader.Characteristics |= 0x0001;
	file.seekp(SectionRAW - sizeof(IMAGE_NT_HEADERS32));
	file.write((char*)NTHeader, sizeof(IMAGE_NT_HEADERS32));
	AddSectionHeader(NTHeader, file, SectionRAW, CallBack);
	ExpandFileForSection(NTHeader, file, SizeOfFile);
	return NewSection;
}


void ExpandFileForSectionHeader(PIMAGE_NT_HEADERS32 NTHeader, fstream& file, DWORD SectionRAW, const char* filename, DWORD& SizeOfFile)
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


void ExpandFileForSection(PIMAGE_NT_HEADERS32 NTHeader, fstream& file, DWORD& SizeOfFile)
{
	char c = 0;
	file.seekp(SizeOfFile);
	for (int k = 0; k < NTHeader->OptionalHeader.FileAlignment; k++) file.write(&c, 1);
	SizeOfFile += NTHeader->OptionalHeader.FileAlignment;
}


void AddSectionHeader(PIMAGE_NT_HEADERS32 NTHeader, fstream& file, DWORD SectionRAW, void* CallBack)
{
	IMAGE_SECTION_HEADER SectionHeader;
	DWORD RAW = SectionRAW + (NTHeader->FileHeader.NumberOfSections - 2) * sizeof(IMAGE_SECTION_HEADER);
	file.seekg(RAW);
	file.read((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
	SectionHeader.PointerToRawData += SectionHeader.SizeOfRawData;
	SectionHeader.SizeOfRawData = NTHeader->OptionalHeader.FileAlignment;
	SectionHeader.VirtualAddress += (1 + SectionHeader.Misc.VirtualSize / NTHeader->OptionalHeader.SectionAlignment) * NTHeader->OptionalHeader.SectionAlignment;
	SectionHeader.Misc.VirtualSize = 144;
	typedef void(*_SectionCALLBACK)(PIMAGE_NT_HEADERS32, PIMAGE_SECTION_HEADER);
	_SectionCALLBACK SectionCALLBACK = (_SectionCALLBACK)CallBack;
	SectionCALLBACK(NTHeader, &SectionHeader);
	file.seekp(SectionRAW + sizeof(IMAGE_SECTION_HEADER)*(NTHeader->FileHeader.NumberOfSections - 1));
	file.write((char*)&SectionHeader, sizeof(IMAGE_SECTION_HEADER));
}




void SetUpFunctions(fstream& file, DWORD SectionRAW, DWORD RVA1, DWORD RVA2, DWORD* funcs, DWORD ImportTableRAW, DWORD* PrevRVA, DWORD SectionNum)
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
			file.read((char*)&PrevRVA[0], 4);
			file.seekp(RAW);
			file.write((char*)&RVA1, 4);

			RAW = GetRAW(file, d.OriginalFirstThunk, SectionRAW, SectionNum);
			file.seekp(RAW);
			file.write((char*)&RVA1, 4);
			//file.seekg(GetRAW(file, PrevRVA[0], SectionRAW, SectionNum));
			//file.read((char*)&hint[0], 2);

			//file.seekp(GetRaw(name));
			//file.write((char*)&hint[0], 2);
			//file.write(func1, 16);



			RAW = GetRAW(file, d.FirstThunk, SectionRAW, SectionNum) + 4;
			funcs[1] = d.FirstThunk + 4;
			file.seekg(RAW);
			file.read((char*)&PrevRVA[1], 4);
			file.seekp(RAW);
			file.write((char*)&RVA2, 4);

			RAW = GetRAW(file, d.OriginalFirstThunk, SectionRAW, SectionNum) + 4;
			file.seekp(RAW);
			file.write((char*)&RVA2, 4);
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




DWORD GetRAW(fstream& file, DWORD adr, DWORD SectionRAW, DWORD SectionNum)
{
	DWORD last = file.tellg();
	IMAGE_SECTION_HEADER header;
	DWORD off;
	file.seekg(SectionRAW);
	for (DWORD i = 0; i < SectionNum; i++)
	{
		file.read((char*)&header, sizeof(header));
		off = header.VirtualAddress;
		if ((adr >= off) && (adr < (off + header.SizeOfRawData)))
		{
			file.seekg(last);
			return (header.PointerToRawData + (adr - header.VirtualAddress));
		}
	}
	file.seekg(last);
	return 0;
}