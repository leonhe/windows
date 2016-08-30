#pragma once
#include <string>
#include <functional>
class Utils
{
public:
	typedef std::function<void(int,int)> UNZIP_CALLBACK;
	//��ѹzip�����������õ�Ŀ¼
	static BOOL Unzip2Folder(const std::string &lpZipFile, const std::string &lpFolder, UNZIP_CALLBACK cCallBack);
	//ɱ���������е���Ϸ����
	static void KillGameProcess();
	//��ʽ��Ŀ¼·����Windows
	static void formatDirPathTOWindows(std::string *paht);
	//������������Ŀ¼·��ѭ������Ŀ¼
	static void foreachCreateDirectory(const std::string *path);
	//��ȡ�ļ���MDֵ
	static BOOL GetContentMD5(
		BYTE *pszFilePath,
		BOOL bFile,
		BOOL bUpperCase,
		std::string *pszResult,
		DWORD &dwStatus);
};

