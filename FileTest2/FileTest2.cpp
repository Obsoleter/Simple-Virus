﻿#include "pch.h"
using namespace std;

fstream f;
DWORD EBP, SP, SP_NUM, RAW;
DWORD GetRaw(DWORD adr);
DWORD GetRVA(DWORD adr);
DWORD GetOff(WORD);
DWORD FileAlign;
DWORD SecAlign;
char FileName[64];
bool x64;


void SectionSetUp(PIMAGE_NT_HEADERS32 NTHeader, PIMAGE_SECTION_HEADER SectionHeader)
{
	SectionHeader->Characteristics = 0xE0000060;
	SectionHeader->Name[0] = '.';
	SectionHeader->Name[1] = 'f';
	SectionHeader->Name[2] = 'a';
	SectionHeader->Name[3] = 'f';
	SectionHeader->Name[4] = 0;
}

void SectionSetUp64(PIMAGE_NT_HEADERS64 NTHeader, PIMAGE_SECTION_HEADER SectionHeader)
{
	SectionHeader->Characteristics = 0xE0000040;
	cout << (void*)SectionHeader->Characteristics << endl;
	SectionHeader->Name[0] = '.';
	SectionHeader->Name[1] = 'f';
	SectionHeader->Name[2] = 'a';
	SectionHeader->Name[3] = 'f';
	SectionHeader->Name[4] = 0;
	cout << "x64 callback!\n";
	system("pause");
}


int main()
{
	cout << "Enter File's Name: ";
	cin >> FileName;
	f.open(FileName, ios::out | ios::in | ios::binary | ios::_Nocreate | ios::ate);
	if (!f.is_open())
	{
		cout << "Error occured!\n";
		system("pause");
		return -1;
	}
	DWORD max = f.tellg();
	f.seekg(0);
	IMAGE_DOS_HEADER head1;
	IMAGE_NT_HEADERS32 head2;
	IMAGE_SECTION_HEADER sec;
	f.read((char*)&head1, sizeof(head1));
	f.seekg(head1.e_lfanew);
	f.read((char*)&head2, sizeof(head2));
	SP = f.tellg();
	SP_NUM = head2.FileHeader.NumberOfSections;
	FileAlign = head2.OptionalHeader.FileAlignment;
	SecAlign = head2.OptionalHeader.SectionAlignment;
	EBP = head2.OptionalHeader.ImageBase;
	if (f.is_open()) cout << "Opened!\n";
	if (head2.FileHeader.SizeOfOptionalHeader == 0xF0)
	{
		x64 = true;
		cout << "x64 file!\n";
		system("pause");
	}
	if (!x64)
	{
		AddSection(&head2, f, SP, FileName, max, &SectionSetUp);
		DWORD RVA1, RVA2, funcs[2], PrevRVA[2];
		char f1[] = "\0\0LoadLibraryA", f2[] = "\0\0GetProcAddress";
		f.seekg(SP + (head2.FileHeader.NumberOfSections - 1) * sizeof(sec));
		f.read((char*)&sec, sizeof(sec));
		RVA1 = sec.VirtualAddress;
		RVA2 = sec.VirtualAddress + 15;
		RAW = GetRaw(head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		SetUpFunctions(f, SP, RVA1, RVA2, funcs, RAW, PrevRVA, head2.FileHeader.NumberOfSections);
		f.seekg(GetRaw(PrevRVA[0]));
		WORD hint;
		f.read((char*)&hint, 2);
		*(PWORD)f1 = hint;
		f.seekg(GetRaw(PrevRVA[1]));
		f.read((char*)&hint, 2);
		*(PWORD)f2 = hint;
		f.seekp(sec.PointerToRawData);
		f.write(f1, 15);
		f.write(f2, 17);
		char str1[] = "USER32.dll", str2[] = "MessageBoxA";
		f.write(str1, 11);//offset - 32
		f.write(str2, 12);//offset - 43
		char opc[] = { 0x68, 0x11, 0x11, 0x11, 0x11, 0xFF, 0x15, 0x22, 0x22, 0x22,
			0x22, 0x68, 0x33, 0x33, 0x33, 0x33, 0x50, 0xFF, 0x15, 0x44, 0x44, 0x44,
			0x44, 0x50, 0x6A, 0x10, 0x68, 0x55, 0x55, 0x55, 0x55, 0x68, 0x55, 0x55,
			0x55, 0x55, 0x6A, 0x00, 0xFF, 0x54, 0x24, 0x10, 0xEB, 0xEC };
		*(PDWORD)&opc[1] = head2.OptionalHeader.ImageBase + sec.VirtualAddress + 32;
		*(PDWORD)&opc[7] = head2.OptionalHeader.ImageBase + funcs[0];
		*(PDWORD)&opc[0xC] = head2.OptionalHeader.ImageBase + sec.VirtualAddress + 43;
		*(PDWORD)&opc[0x13] = head2.OptionalHeader.ImageBase + funcs[1];
		*(PDWORD)&opc[0x1B] = head2.OptionalHeader.ImageBase + sec.VirtualAddress + 32;
		*(PDWORD)&opc[0x20] = head2.OptionalHeader.ImageBase + sec.VirtualAddress + 32;
		f.write(opc, 44);
		f.seekp(head1.e_lfanew);
		head2.OptionalHeader.AddressOfEntryPoint = sec.VirtualAddress + 55;
		f.write((char*)&head2, sizeof(head2));
	}
	else
	{
		cout << "x64 else!\n";
		system("pause");
		IMAGE_NT_HEADERS64 head64;
		f.seekg(head1.e_lfanew);
		f.read((char*)&head64, sizeof(head64));
		SP = f.tellg();
		SP_NUM = head64.FileHeader.NumberOfSections;
		


		AddSection64(&head64, f, SP, FileName, max, &SectionSetUp64);
		DWORD64 RVA1, RVA2, PrevRVA[2];
		DWORD funcs[2];
		char f1[] = "\0\0LoadLibraryA", f2[] = "\0\0GetProcAddress";
		f.seekg(SP + (head64.FileHeader.NumberOfSections - 1) * sizeof(sec));
		f.read((char*)&sec, sizeof(sec));
		RVA1 = sec.VirtualAddress;
		RVA2 = sec.VirtualAddress + 15;
		RAW = GetRaw(head64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		SetUpFunctions64(f, SP, RVA1, RVA2, funcs, RAW, PrevRVA, head64.FileHeader.NumberOfSections);
		f.seekg(GetRaw(PrevRVA[0]));
		WORD hint;
		f.read((char*)&hint, 2);
		*(PWORD)f1 = hint;
		f.seekg(GetRaw(PrevRVA[1]));
		f.read((char*)&hint, 2);
		*(PWORD)f2 = hint;
		f.seekp(sec.PointerToRawData);
		f.write(f1, 15);
		f.write(f2, 17);


		char str1[] = "KERNEL32.dll", str2[] = "GetStdHandle", str3[] = "WriteConsoleA", str4[] = "ReadConsoleA", str5[] = "VirtualProtect", str6[] = "AttachConsole", str7[] = "AllocConsole";
		char Msg1[] = "Enter code: ", Msg2[] = "Enter code: ", passw[] = "36740";
		char symb;
		DWORD count;
		char FreeCon[] = "FreeConsole";


		f.write(str1, 13);//offset - 32
		f.write(str2, 13);//offset - 45
		f.write(str3, 14);//offset - 58
		f.write(str4, 13);//offset - 72
		f.write(str5, 15);//offset - 85
		f.write(str6, 14);//offset - 100
		f.write(str7, 13);//offset - 114
		f.write(Msg1, 12);//offset - 127
		f.write(Msg2, 12);//offset - 139
		f.write(passw, 5);//offset - 151
		f.write(&symb, 1);//offset - 156
		f.write((char*)&count, 4);//offset - 157
		f.write(FreeCon, 12);//offset - 161

		

		

		char opc1[] = { 0x48, 0x89, 0xE5, 0x55, 0x48, 0x81, 0xEC, 0x00, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xF0, 0x48, 0x89, 0xC1, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xE8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xE0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xD8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xC0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xA8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xA0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x98, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x90, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x88, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x80, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x85, 0x78, 0xFF, 0xFF, 0xFF, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x85, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xA8, 0xFF, 0x55, 0xA0, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x0B, 0xFF, 0x55, 0xE8, 0x48, 0x89, 0x45, 0xD0, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x0A, 0xFF, 0x55, 0xE8, 0x48, 0x89, 0x45, 0xC8, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x01, 0x48, 0x8B, 0x4D, 0xD0, 0x48, 0x8B, 0x55, 0x88, 0x49, 0xC7, 0xC0, 0x0C, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xE0, 0x48, 0x8B, 0x4D, 0xC8, 0x48, 0x8B, 0x55, 0x90, 0x49, 0xC7, 0xC0, 0x0C, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xD8, 0x48, 0x8B, 0x7D, 0x90, 0x48, 0x31, 0xC0, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x8B, 0x03, 0xFF, 0xC8, 0x48, 0x01, 0xC7, 0x80, 0x3F, 0x0A, 0x74, 0x35, 0x48, 0x8B, 0x4D, 0xC8, 0x48, 0x8B, 0x95, 0x78, 0xFF, 0xFF, 0xFF, 0x49, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xD8, 0x48, 0x8B, 0x9D, 0x78, 0xFF, 0xFF, 0xFF, 0x80, 0x3B, 0x0A, 0x75, 0xD0, 0xE9, 0x6F, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x83, 0x3B, 0x07, 0x0F, 0x85, 0x5F, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x75, 0x80, 0x48, 0x8B, 0x7D, 0x90, 0xB0, 0x05, 0x8A, 0x3E, 0x8A, 0x1F, 0x38, 0xDF, 0x0F, 0x85, 0x49, 0xFF, 0xFF, 0xFF, 0x48, 0xFF, 0xC6, 0x48, 0xFF, 0xC7, 0xFE, 0xC8, 0x75, 0xEA, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xB8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xB0, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC2, 0x10, 0x00, 0x00, 0x00, 0x49, 0xC7, 0xC0, 0x64, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xC0, 0x48, 0x8B, 0x45, 0xB8, 0x48, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x03, 0x48, 0x8B, 0x45, 0xB0, 0x48, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC3, 0x08, 0x48, 0x89, 0x03, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC2, 0x10, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x4D, 0x31, 0xC0, 0x44, 0x8B, 0x03, 0x4C, 0x8B, 0x4D, 0x90, 0xFF, 0x55, 0xC0, 0xFF, 0x55, 0x98, 0x48, 0x81, 0xC4, 0x00, 0x01, 0x00, 0x00, 0x5D, 0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xFF, 0xE0 };
		char opc[] = { 0x50, 0x53, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x56, 0x57, 0x55, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xEC, 0x00, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xF0, 0x48, 0x89, 0xC1, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xE8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xE0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xD8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xC0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xA8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xA0, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x98, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x90, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x88, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0x80, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x85, 0x78, 0xFF, 0xFF, 0xFF, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x85, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xA8, 0xFF, 0x55, 0xA0, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x0B, 0xFF, 0x55, 0xE8, 0x48, 0x89, 0x45, 0xD0, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x0A, 0xFF, 0x55, 0xE8, 0x48, 0x89, 0x45, 0xC8, 0x48, 0x31, 0xC9, 0x83, 0xE9, 0x01, 0x48, 0x8B, 0x4D, 0xD0, 0x48, 0x8B, 0x55, 0x88, 0x49, 0xC7, 0xC0, 0x0C, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xE0, 0x48, 0x8B, 0x4D, 0xC8, 0x48, 0x8B, 0x55, 0x90, 0x49, 0xC7, 0xC0, 0x0C, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xD8, 0x48, 0x8B, 0x7D, 0x90, 0x48, 0x31, 0xC0, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x8B, 0x03, 0xFF, 0xC8, 0x48, 0x01, 0xC7, 0x80, 0x3F, 0x0A, 0x74, 0x35, 0x48, 0x8B, 0x4D, 0xC8, 0x48, 0x8B, 0x95, 0x78, 0xFF, 0xFF, 0xFF, 0x49, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xDB, 0x48, 0x89, 0x5C, 0x24, 0x20, 0xFF, 0x55, 0xD8, 0x48, 0x8B, 0x9D, 0x78, 0xFF, 0xFF, 0xFF, 0x80, 0x3B, 0x0A, 0x75, 0xD0, 0xE9, 0x6F, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x83, 0x3B, 0x07, 0x0F, 0x85, 0x5F, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x75, 0x80, 0x48, 0x8B, 0x7D, 0x90, 0xB0, 0x05, 0x8A, 0x3E, 0x8A, 0x1F, 0x38, 0xDF, 0x0F, 0x85, 0x49, 0xFF, 0xFF, 0xFF, 0x48, 0xFF, 0xC6, 0x48, 0xFF, 0xC7, 0xFE, 0xC8, 0x75, 0xEA, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xB8, 0x48, 0x8B, 0x4D, 0xF0, 0x48, 0x8D, 0x15, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x45, 0xB0, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC2, 0x10, 0x00, 0x00, 0x00, 0x49, 0xC7, 0xC0, 0x64, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8D, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xC0, 0x48, 0x8B, 0x45, 0xB8, 0x48, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x03, 0x48, 0x8B, 0x45, 0xB0, 0x48, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC3, 0x08, 0x48, 0x89, 0x03, 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC2, 0x10, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x9D, 0x70, 0xFF, 0xFF, 0xFF, 0x4D, 0x31, 0xC0, 0x44, 0x8B, 0x03, 0x4C, 0x8B, 0x4D, 0x90, 0xFF, 0x55, 0xC0, 0xFF, 0x55, 0x98, 0x48, 0x81, 0xC4, 0x00, 0x01, 0x00, 0x00, 0x5D, 0x5F, 0x5E, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0x5B, 0x58, 0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xFF, 0xE0 };


		*(PDWORD)&opc[0xB + 3 + 10] = sec.VirtualAddress + 32 - (sec.VirtualAddress + 173 + 0x12 + 10);
		*(PDWORD)&opc[0x12 + 2 + 10] = funcs[0] - (sec.VirtualAddress + 173 + 0x18 + 10);

		*(PDWORD)&opc[0x1F + 3 + 10] = sec.VirtualAddress + 45 - (sec.VirtualAddress + 173 + 0x26 + 10);
		*(PDWORD)&opc[0x26 + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x2C + 10);

		*(PDWORD)&opc[0x34 + 3 + 10] = sec.VirtualAddress + 58 - (sec.VirtualAddress + 173 + 0x3B + 10);
		*(PDWORD)&opc[0x3B + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x41 + 10);

		*(PDWORD)&opc[0x49 + 3 + 10] = sec.VirtualAddress + 72 - (sec.VirtualAddress + 173 + 0x50 + 10);
		*(PDWORD)&opc[0x50 + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x56 + 10);

		*(PDWORD)&opc[0x5E + 3 + 10] = sec.VirtualAddress + 85 - (sec.VirtualAddress + 173 + 0x65 + 10);
		*(PDWORD)&opc[0x65 + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x6B + 10);

		*(PDWORD)&opc[0x73 + 3 + 10] = sec.VirtualAddress + 100 - (sec.VirtualAddress + 173 + 0x7A + 10);
		*(PDWORD)&opc[0x7A + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x80 + 10);

		*(PDWORD)&opc[0x88 + 3 + 10] = sec.VirtualAddress + 114 - (sec.VirtualAddress + 173 + 0x8F + 10);
		*(PDWORD)&opc[0x8F + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x95 + 10);

		*(PDWORD)&opc[0x9D + 3 + 10] = sec.VirtualAddress + 161 - (sec.VirtualAddress + 173 + 0xA4 + 10);
		*(PDWORD)&opc[0xA4 + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0xAA + 10);

		*(PDWORD)&opc[0xAE + 3 + 10] = sec.VirtualAddress + 127 - (sec.VirtualAddress + 173 + 0xB5 + 10);
		*(PDWORD)&opc[0xB9 + 3 + 10] = sec.VirtualAddress + 139 - (sec.VirtualAddress + 173 + 0xC0 + 10);
		*(PDWORD)&opc[0xC4 + 3 + 10] = sec.VirtualAddress + 151 - (sec.VirtualAddress + 173 + 0xCB + 10);
		*(PDWORD)&opc[0xCF + 3 + 10] = sec.VirtualAddress + 156 - (sec.VirtualAddress + 173 + 0xD6 + 10);
		*(PDWORD)&opc[0xDD + 3 + 10] = sec.VirtualAddress + 157 - (sec.VirtualAddress + 173 + 0xE4 + 10);

		*(PDWORD)&opc[0x1D6 + 3 + 10] = PrevRVA[0] + 2 - (sec.VirtualAddress + 173 + 0x1DD + 10);
		*(PDWORD)&opc[0x1DD + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x1E3 + 10);

		*(PDWORD)&opc[0x1EB + 3 + 10] = PrevRVA[1] + 2 - (sec.VirtualAddress + 173 + 0x1F2 + 10);
		*(PDWORD)&opc[0x1F2 + 2 + 10] = funcs[1] - (sec.VirtualAddress + 173 + 0x1F8 + 10);

		*(PDWORD)&opc[0x1FC + 3 + 10] = funcs[0] - (sec.VirtualAddress + 173 + 0x203 + 10);

		*(PDWORD)&opc[0x21F + 3 + 10] = funcs[0] - (sec.VirtualAddress + 173 + 0x226 + 10);

		*(PDWORD)&opc[0x22D + 3 + 10] = funcs[0] - (sec.VirtualAddress + 173 + 0x234 + 10);

		*(PDWORD)&opc[0x23B + 3 + 10] = funcs[0] - (sec.VirtualAddress + 173 + 0x242 + 10);

		*(PDWORD64)&opc[0x268 + 2 + 20] = head64.OptionalHeader.AddressOfEntryPoint + head64.OptionalHeader.ImageBase;

		opc[0x20A + 3 + 10] = 0x40;

		


		f.write(opc, 648);
		f.seekp(head1.e_lfanew);
		head64.OptionalHeader.AddressOfEntryPoint = sec.VirtualAddress + 173;
		f.write((char*)&head64, sizeof(head64));
		cout << (void*)(sec.VirtualAddress - funcs[0]) << endl;
		system("pause");
	}



	/*

	RAW = f.tellg();
	RAW += SP_NUM * sizeof(sec);
	if ((head2.OptionalHeader.SizeOfHeaders - RAW) < sizeof(sec))
	{
		f.seekg(0);
		fstream f2;
		f2.open("File.exe.new", ios::out | ios::in | ios::binary | ios::app);
		char c;
		int i = 0;
		for (; i < head2.OptionalHeader.SizeOfHeaders; i++)
		{
			f.read(&c, 1);
			f2.write(&c, 1);
		}
		c = 0;
		for (int k = 0; k < FileAlign; k++) f2.write(&c, 1);
		for (; i < max; i++)
		{
			f.read(&c, 1);
			f2.write(&c, 1);
		}
		f2.close();
		f.close();
		remove("File.exe");
		rename("File.exe.new", "File.exe");
		f.open("File.exe", ios::out | ios::in | ios::binary | ios::_Nocreate | ios::ate);

		head2.OptionalHeader.SizeOfHeaders += FileAlign;

		for (int k = 0; i < SP_NUM; k++)
		{
			f.seekg(SP + sizeof(sec)*i);
			f.read((char*)&sec, sizeof(sec));
			sec.PointerToRawData += FileAlign;
			if (sec.VirtualAddress == head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) 
				sec.Misc.VirtualSize += 20;
			f.seekp(SP + sizeof(sec)*i);
			f.write((char*)&sec, sizeof(sec));
		}
	}

	head2.FileHeader.NumberOfSections++;
	head2.OptionalHeader.SizeOfImage += SecAlign;
	head2.OptionalHeader.SizeOfInitializedData += FileAlign;
	head2.FileHeader.Characteristics |= 0x0001;
//	f.seekp(head1.e_lfanew);
//	f.write((char*)&head2, sizeof(head2));

	RAW = SP + (SP_NUM - 1) * sizeof(sec);
	f.seekg(RAW);
	f.read((char*)&sec, sizeof(sec));

	sec.Characteristics = 0xE0000060;
	sec.Misc.VirtualSize = 144;
	sec.PointerToRawData += sec.SizeOfRawData;
	sec.SizeOfRawData = FileAlign;
	sec.VirtualAddress += (1 + sec.Misc.VirtualSize / SecAlign) * SecAlign;
	sec.Name[0] = '.';
	sec.Name[1] = 'f';
	sec.Name[2] = 'a';
	sec.Name[3] = 'f';
	sec.Name[4] = 0;
	f.seekp(SP + sizeof(sec)*SP_NUM);
	f.write((char*)&sec, sizeof(sec));








	IMAGE_IMPORT_DESCRIPTOR d;
	IMAGE_THUNK_DATA th;
	DWORD call[2];
	WORD hint[2];
	DWORD name;
	char func1[] = "GetProcAddress\0";
	char func2[] = "LoadLibraryA\0";
	bool iskernel = 1;
	char mod[] = "KERNEL32", modname[8];
	RAW = GetRaw(head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	while (1)
	{
		f.seekg(RAW);
		f.read((char*)&d, sizeof(d));
		if (d.FirstThunk == 0) break;
		f.seekg(GetRaw(d.Name));
		f.read(modname, 8);
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
			RAW = GetRaw(d.FirstThunk);
			call[0] = d.FirstThunk;
			f.seekg(RAW);
			f.read((char*)&name, 4);
			f.seekg(GetRaw(name));
			f.read((char*)&hint[0], 2);

			f.seekp(GetRaw(name));
			f.write((char*)&hint[0], 2);
			f.write(func1, 16);



			RAW = GetRaw(d.FirstThunk) + 4;
			call[1] = d.FirstThunk + 4;
			f.seekg(RAW);
			f.read((char*)&name, 4);
			f.seekg(GetRaw(name));
			f.read((char*)&hint[1], 2);

			f.seekp(GetRaw(name));
			f.write((char*)&hint[1], 2);
			f.write(func2, 14);


			break;
		}
		else iskernel = 1;
		cout << modname << endl;
		RAW += sizeof(d);
	}








	DWORD adr = 0;
	char opcodes[0x2C];
	char str1[] = "NOPE!";//offset - 34
	char str2[] = "USER32.dll\0\0";//offset - 40
	char str3[] = "MessageBoxA";//offset - 53









	opcodes[0] = 0x68;//65
	*(PDWORD)&opcodes[1] = sec.VirtualAddress + EBP + 40;
	opcodes[5] = 0xFF;
	opcodes[6] = 0x15;
	*(PDWORD)&opcodes[7] = call[1] + EBP;
	opcodes[11] = 0x68;
	*(PDWORD)&opcodes[12] = sec.VirtualAddress + EBP + 53;
	opcodes[16] = 0x50;
	opcodes[17] = 0xFF;
	opcodes[18] = 0x15;
	*(PDWORD)&opcodes[19] = call[0] + EBP;
	opcodes[23] = 0x50;
	opcodes[24] = 0x6A;
	opcodes[25] = 0x10;
	opcodes[26] = 0x68;
	*(PDWORD)&opcodes[27] = sec.VirtualAddress + EBP + 34;
	opcodes[31] = 0x68;
	*(PDWORD)&opcodes[32] = sec.VirtualAddress + EBP + 34;
	opcodes[36] = 0x6A;
	opcodes[37] = 0;
	opcodes[38] = 0xFF;
	opcodes[39] = 0x54;
	opcodes[40] = 0x24;
	opcodes[41] = 0x10;
	opcodes[42] = 0xEB;
	opcodes[43] = 0xEE;
	


	f.seekp(max);
	for (int k = 0; k < FileAlign; k++) f.write((char*)&adr, 1);



	f.seekp(max);
	f.write((char*)&hint[0], 2);
	f.write(func1, 16);
	f.write((char*)&hint[1], 2);
	f.write(func2, 14);
	f.write(str1, 6); 
	f.write(str2, 13);
	f.write(str3, 12);
	f.write(opcodes, 0x2B);




//	head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
//	head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
	head2.OptionalHeader.AddressOfEntryPoint = 65 + sec.VirtualAddress;

	f.seekp(head1.e_lfanew);
	f.write((char*)&head2, sizeof(head2));

	


	RAW = GetRaw(head2.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	IMAGE_BASE_RELOCATION rel;
	while (1)
	{
		f.read((char*)&rel, 8);
		if (rel.VirtualAddress == 0) break;
		RAW += rel.SizeOfBlock;
	}
	rel.VirtualAddress = sec.VirtualAddress;
	rel.SizeOfBlock = 20;
	f.seekp(RAW);
	f.write((char*)&rel, 8);
	WORD off;
	off = 0x3000 | 66;
	f.write((char*)&off, 2);
	off = 0x3000 | 72;
	f.write((char*)&off, 2);
	off = 0x3000 | 77;
	f.write((char*)&off, 2);
	off = 0x3000 | 84;
	f.write((char*)&off, 2);
	off = 0x3000 | 92;
	f.write((char*)&off, 2);
	off = 0x3000 | 97;
	f.write((char*)&off, 2);




*/
for (int i = 0; i < 3; i++)
{
	f.seekg(SP + sizeof(sec)*i);
	f.read((char*)&sec, sizeof(sec));
	cout << sec.Name << "   Char: " << (void*)sec.Characteristics << endl;
}

	
	f.close();
	system("pause");
	return 0;
}











DWORD GetRaw(DWORD adr)
{
	DWORD last = f.tellg();
	f.seekg(SP);
	IMAGE_SECTION_HEADER header;
	DWORD off;
	for (DWORD i = 0; i < SP_NUM; i++)
	{
		f.read((char*)&header, sizeof(header));
		off = header.VirtualAddress;
		if ((adr >= off) && (adr < (off + header.SizeOfRawData)))
		{
			f.seekg(last);
			return (header.PointerToRawData + (adr - header.VirtualAddress));
		}
	}
	f.seekg(last);
	return 0;
}


DWORD GetRVA(DWORD adr)
{
	DWORD last = f.tellg();
	f.seekg(SP);
	IMAGE_SECTION_HEADER header;
	DWORD off;
	for (DWORD i = 0; i < SP_NUM; i++)
	{
		f.read((char*)&header, sizeof(header));
		off = header.PointerToRawData;
		if ((adr >= off) && (adr < (off + header.SizeOfRawData)))
		{
			f.seekg(last);
			return (header.VirtualAddress + (adr - header.PointerToRawData));
		}
	}
	f.seekg(last);
	return 0;
}

DWORD GetOff(WORD f)
{
	return f & 0x0FFF;
}