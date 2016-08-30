#pragma once
#include <string>
#include <functional>
class Utils
{
public:
	typedef std::function<void(int,int)> UNZIP_CALLBACK;
	//解压zip包到参数设置的目录
	static BOOL Unzip2Folder(const std::string &lpZipFile, const std::string &lpFolder, UNZIP_CALLBACK cCallBack);
	//杀死正在运行的游戏程序
	static void KillGameProcess();
	//格式化目录路径到Windows
	static void formatDirPathTOWindows(std::string *paht);
	//根绝参数设置目录路径循环创建目录
	static void foreachCreateDirectory(const std::string *path);
	//获取文件的MD值
	static BOOL GetContentMD5(
		BYTE *pszFilePath,
		BOOL bFile,
		BOOL bUpperCase,
		std::string *pszResult,
		DWORD &dwStatus);
};

