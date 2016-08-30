#include "stdafx.h"
#include "Utils.h"
#include <windows.h>
#include <comutil.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <TlHelp32.h>
#include <cwchar>

#include <fstream>
#include "minizip/unzip.h"
#include "zlib.h"

//格式化目录路径到WINDOWS样式
void  Utils::formatDirPathTOWindows(std::string *path)
{
	const char *PRE = "\/";
	auto pos = path->find_first_of(PRE);
	while (pos != std::string::npos)
	{
		path->replace(pos, 1, "\\");
		pos = path->find_first_of(PRE, pos + 1);
	}
}

void Utils::foreachCreateDirectory(const std::string *path)
{
	auto st_pos = path->find_first_of("\\");
	while (st_pos != std::string::npos)
	{
		auto dir = path->substr(0, st_pos);
		if (!PathIsDirectoryA(dir.c_str())){
			if (!CreateDirectoryA(dir.c_str(), NULL)){
				
				std::string err_msg("create directory fail:");
				err_msg.append(dir.c_str());
				printDebugInfo(err_msg.c_str());
			}
		}
		st_pos = path->find_first_of("\\", st_pos + 1);
	}
}

BOOL Utils::Unzip2Folder(const std::string &lpZipFile, const std::string &lpFolder, UNZIP_CALLBACK cCallBack)
{


	const char* version = zlibVersion();
	std::string version_info("Zlib Version:");
	printDebugInfo(version_info.c_str());
	unzFile unzip_fp = unzOpen(lpZipFile.c_str());

	uLong size;
	unz_global_info info;
	if (UNZ_OK != unzGetGlobalInfo(unzip_fp, &info)){
		return FALSE;
	}
	unz_file_info zFileInfo;
	char *fileName = new char[PATHLEN];
	uLong num;
	for (int i = 0; i < info.number_entry; i++)
	{

		// 遍历所有文件  
		if (UNZ_OK != unzGetCurrentFileInfo(unzip_fp, &zFileInfo, fileName, PATHLEN, NULL, 0, NULL, 0))
		{
			//错误处理信息  
			printDebugInfo("error");
			continue;
		}
		int res = unzOpenCurrentFile(unzip_fp);
		if (res!=UNZ_OK)
		{
			std::string error_msg("Open unzip in file faile:");
			error_msg.append(fileName);
			printDebugInfo(error_msg.c_str());
			continue;
		}
		int filelength = zFileInfo.uncompressed_size;
		char *buffer = new char[zFileInfo.uncompressed_size];
		memset(buffer, 0, filelength);
		int erro = unzReadCurrentFile(unzip_fp, buffer, static_cast<unsigned>(zFileInfo.uncompressed_size));
		if (erro<0)
		{
			std::string error_msg("Read Currrent Fail:");
			error_msg.append(fileName);
			printDebugInfo(error_msg.c_str());
			continue;
		}
		unzCloseCurrentFile(unzip_fp);

		std::string filepath(lpFolder);
		filepath.append("/");
		filepath.append(fileName);

		Utils::formatDirPathTOWindows(&filepath);
		Utils::foreachCreateDirectory(&filepath);

		std::fstream fs(filepath, std::fstream::out | std::fstream::binary);
		fs.write(buffer, filelength);
		fs.close();
		do{
			delete buffer;
			buffer = nullptr;
		} while (buffer);
		
		if (cCallBack)
		{
			cCallBack(i+1 , info.number_entry);
		}

		unzGoToNextFile(unzip_fp);
	}

	unzClose(unzip_fp);
	return TRUE;
}


void Utils::KillGameProcess()
{
	PROCESSENTRY32 pe32;

	//使用该结构之前声明大小
	pe32.dwSize = sizeof(pe32);

	//给系统内所有进程一个快照
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf_s("CreateToolhelp32Snapshot 调用失败!\n");
	}

	//遍历快照，轮流显示每个进程的信息
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		printf_s("进程名称:");
		wprintf_s(L"%ls", pe32.szExeFile);
		printf_s("\n进程ID:%u\n", pe32.th32ProcessID);
		bMore = ::Process32Next(hProcessSnap, &pe32);

		if (wcscmp(pe32.szExeFile, L"QDress.exe") == 0)
		{

			BOOL bRet = FALSE;
			//打开目标程序，取得句柄
			HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (hProcess != NULL)
			{
				//终止进程
				bRet = ::TerminateProcess(hProcess, 0);
			}
			if (bRet)
			{
				printf("成功终止进程!\n");
			}
			else{
				printf("终止进程失败!\n");
			}
		}

		printf("\n");
	}

	//清除Snapshot对象
	::CloseHandle(hProcessSnap);
}


#define CHECK_NULL_RET(bCondition) if (!bCondition) goto Exit0
#define BUFSIZE 1024
#define MD5LEN  16

BOOL  Utils::GetContentMD5(
	BYTE *pszFilePath,
	BOOL bFile,
	BOOL bUpperCase,
	std::string *pszResult,
	DWORD &dwStatus)
{
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigitsL[] = "0123456789abcdef";
	CHAR rgbDigitsU[] = "0123456789ABCDEF";
	CHAR *rgbDigits = bUpperCase ? rgbDigitsU : rgbDigitsL;
	TCHAR szResult[MD5LEN * 2 + 1] = { 0 };

	dwStatus = 0;
	bResult = CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT);
	CHECK_NULL_RET(bResult);

	bResult = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	CHECK_NULL_RET(bResult);

	if (bFile)
	{
		hFile = CreateFile((TCHAR *)pszFilePath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		CHECK_NULL_RET(!(INVALID_HANDLE_VALUE == hFile));

		while (bResult = ReadFile(hFile, rgbFile, BUFSIZE,
			&cbRead, NULL))
		{
			if (0 == cbRead)
			{
				break;
			}

			bResult = CryptHashData(hHash, rgbFile, cbRead, 0);
			CHECK_NULL_RET(bResult);
		}
	}
	else
	{
		bResult = CryptHashData(hHash, pszFilePath, strlen((CHAR *)pszFilePath), 0);
		CHECK_NULL_RET(bResult);
	}

	cbHash = MD5LEN;
	if (bResult = CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		
		for (DWORD i = 0; i < cbHash; i++)
		{
			TCHAR bufs = rgbDigits[rgbHash[i] >> 4];
			TCHAR buf2 = rgbDigits[rgbHash[i] & 0xf];
			
			pszResult->append((char*)&bufs);
			pszResult->append((char*)&buf2);
		}
		bResult = TRUE;
	}

Exit0:
	dwStatus = GetLastError();
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return bResult;
}
