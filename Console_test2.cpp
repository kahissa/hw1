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

bool read_file_using_memory_map()
{
	// current directory 를 구한다.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (buflen == 0)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x\n", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR)* buflen);
	if (GetCurrentDirectoryW(buflen, buf) == 0)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x\n", GetLastError());
		free(buf);
		return false;
	}

	// 현재 디렉토리 \\ bob.txt 파일명 생성
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	if (is_file_existsW(file_name)) {
		DeleteFileW(file_name);
	}

	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		printf("err, CreateFile(%ws) failed, gle = %u\n", file_name, GetLastError());
		return false;
	}

	// check file size
	//
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(file_handle, &fileSize) != TRUE)
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u\n", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	// bob.txt 파일에 내용 쓰기
	wchar_t strUnicode[256] = L"안녕하세요 굳 굳굳 HelloWorld";
	char strUtf8[256] = { 0, };
	DWORD numberOfBytesWritten;

	if (file_handle == INVALID_HANDLE_VALUE) {
		printf("[error] can not CreateFile, gle=0x%08x\n", GetLastError());
		return false;
	}

	int nlen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUtf8, nlen, NULL, NULL);

	WriteFile(file_handle, strUtf8, strlen(strUtf8), &numberOfBytesWritten, NULL);

	// bob.txt -> bob2.txt 파일 복사
	LPCWSTR file_name2 = L"C:\\Users\\kahissa\\Documents\\Visual Studio 2013\\Projects\\Console_test2\\Console_test2\\bob2.txt";
	CopyFile(file_name, file_name2, false);

	// [ WARN ]
	//
	// 4Gb 이상의 파일의 경우 MapViewOfFile()에서 오류가 나거나
	// 파일 포인터 이동이 문제가 됨
	// FilIoHelperClass 모듈을 이용해야 함
	//
	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0)
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass\n",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}


	// mmio를 이용하여 파일 읽기
	HANDLE file_handle2 = CreateFileW(file_name2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD file_size = (DWORD)fileSize.QuadPart;
	HANDLE file_map = CreateFileMapping(
		file_handle2,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
		);
	if (file_map == NULL)
	{
		printf("err, CreateFileMapping(%ws) failed, gle = %u\n", file_name, GetLastError());
		CloseHandle(file_handle2);
		return false;
	}

	char* file_view = (char*)MapViewOfFile(
		file_map,
		FILE_MAP_READ,
		0,
		0,
		0
		);
	if (file_view == NULL)
	{
		printf("err, MapViewOfFile(%ws) failed, gle = %u\n", file_name, GetLastError());

		CloseHandle(file_map);
		CloseHandle(file_handle2);
		return false;
	}

	
	/// UTF8
	int len = MultiByteToWideChar(CP_UTF8, 0, file_view, strlen(file_view), NULL, NULL); 
	wchar_t MultiByte[256] = { 0, };

	// UTF8 -> 유니코드
	MultiByteToWideChar(CP_UTF8, 0, file_view, strlen(file_view), MultiByte, len); 

	char strMultiByte[256] = { 0, };

	// 유니코드 -> 멀티바이트
	len = WideCharToMultiByte(CP_ACP, 0, MultiByte, -1, NULL, 0, NULL, NULL); 
	WideCharToMultiByte(CP_ACP, 0, MultiByte, -1, strMultiByte, len, NULL, NULL); 

	printf("%s\n", strMultiByte);

	// close all
	UnmapViewOfFile(file_view);
	CloseHandle(file_map);
	CloseHandle(file_handle);
	CloseHandle(file_handle2);

	// delete bob.txt, bob2.txt
	DeleteFile(file_name);
	DeleteFile(file_name2);

	return true;
}

void main() {
	read_file_using_memory_map();
}