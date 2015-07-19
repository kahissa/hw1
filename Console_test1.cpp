#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>
#include <crtdbg.h>
#include <stdint.h>

bool is_file_existsW(_In_ const wchar_t* file_path) {
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH)))
		return false;

	WIN32_FILE_ATTRIBUTE_DATA info = { 0 };

	if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info) == 0)
		return false;
	else
		return true;
}

bool create_bob_txt() {
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);

	if (buflen == 0) {
		printf("[error] GetCurrentDirectoryW() failed!!! gle = 0x%08x\n", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR)*buflen);

	if (GetCurrentDirectoryW(buflen, buf) == 0) {
		printf("[error] GetCurrentDirectoryW() failed!!! gle = 0x%08x\n", GetLastError());
		free(buf);
		return false;
	}

	// current directory\\bob.txt ����
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf))) {
		printf("[error] can not create bob.txt\n");
		free(buf);
		return false;
	}
	free(buf);
	buf = NULL;

	if (is_file_existsW(file_name)) {
		DeleteFileW(file_name);
	}

	// ���� ����
	HANDLE file_handle = CreateFileW(
		file_name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	/////////////////////////////////////////////////////////
	DWORD numberOfBytesWritten;
	int result;
	unsigned char mark[3];

	mark[0] = 0xEF;
	mark[1] = 0xBB;

	// UTF-8
	mark[2] = 0xBF;	

	wchar_t strUnicode[256] = L"�ȳ��ϼ��� �� ���� HelloWorld";
	char strUtf8[256] = { 0, };

	if (file_handle == INVALID_HANDLE_VALUE) {
		printf("[error] can not CreateFile, gle=0x%08x\n", GetLastError());
		return false;
	}

	int nlen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUtf8, nlen, NULL, NULL);

	result = WriteFile(file_handle, &mark, 3, &numberOfBytesWritten, NULL);
	result = WriteFile(file_handle, strUtf8, strlen(strUtf8), &numberOfBytesWritten, NULL);

	// bob.txt -> bob2.txt ���� ����
	LPCWSTR file_name2 = L"C:\\Users\\kahissa\\Documents\\Visual Studio 2013\\Projects\\Console_test1\\bob2.txt";
	CopyFile(file_name, file_name2, false);


	// bob2.txt ���� ���� �б�
	char readBuf[256] = { 0, };
	DWORD dwRead = 0;
	BOOL readOK;
	HANDLE file_handle2 = CreateFileW(file_name2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file_handle2 == INVALID_HANDLE_VALUE) {
		printf("[error] can not file open, gle=0x%08x\n", GetLastError());
	}

	readOK = ReadFile(file_handle2, readBuf, 256, &dwRead, NULL);	// ���� ���� �о rBuf�� ����

	if (readOK && dwRead == 0)
		printf("[error] can not read file, gle=0x%08x\n", GetLastError());


	//readBuf�� UTF8 ����
	int len = MultiByteToWideChar(CP_UTF8, 0, readBuf, strlen(readBuf), NULL, NULL);

	wchar_t MultiByte[256] = { 0, };

	// UTF8 -> �����ڵ�
	MultiByteToWideChar(CP_UTF8, 0, readBuf, strlen(readBuf), MultiByte, len);

	char MultiByte2[256] = { 0, };

	/// �����ڵ� -> ��Ƽ����Ʈ
	len = WideCharToMultiByte(CP_ACP, 0, MultiByte, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, MultiByte, -1, MultiByte2, len, NULL, NULL);

	printf("%s\n", MultiByte2 + 1);	// header �������� ���

	CloseHandle(file_handle);
	CloseHandle(file_handle2);

	DeleteFile(file_name);
	DeleteFile(file_name2);

	return true;
}

void main() {
	create_bob_txt();
}