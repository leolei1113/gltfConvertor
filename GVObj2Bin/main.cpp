#include <iostream>
#include <string>
#include <optional>
#include <chrono>
#include <filesystem>
#include <locale>
#include <map>

#include "ObjParser.h"
#include "ObjWriter.h"

using namespace std;
namespace fs = filesystem;

bool endsWith(const string& str, const std::string& suffix) 
{
	return str.rfind(suffix) == str.size() - suffix.size();
}

struct CLIArguments
{
	std::string sourceFileFullName = "";
	std::string outputFileFullName = "";

	bool copyData = false;

	bool outputJson = false;

	bool isLargeObject = false;

	bool useMtlAsGroup = false;

	bool useObjAsGroup = false;

	bool ignoreGFlag = false;

	bool checkGroupArea = false;

	bool outputSubGroups = false;

	bool outputEdgeHash = true;

	bool disableLog = false;
};

static void DirectoryCopyMaterial(string sourceDirName, string destDirName, bool copySubDirs)
{
	// Get the subdirectories for the specified directory.
	fs::path spath = fs::path(sourceDirName).parent_path();

	if (!fs::exists(spath))
	{
		throw std::runtime_error("Source directory does not exist or could not be found: "
			+ sourceDirName);
	}

	if (!fs::exists(fs::path(destDirName)))
	{
		fs::create_directories(destDirName);
	}

	for (const auto& entry : fs::directory_iterator(spath)) {
		if (fs::is_regular_file(entry.status())) 
		{
			std::cout << entry.path() << std::endl;
			string strFullname = entry.path().string();

			if (!endsWith(strFullname, ".obj"))
			{
				fs::copy(strFullname, destDirName + "/" + entry.path().filename().string(), fs::copy_options::overwrite_existing);
			}

		}
		else if (fs::is_directory(entry.status()))
		{
			if (copySubDirs)
			{
				DirectoryCopyMaterial(entry.path().string(),destDirName, copySubDirs);
			}
		}
	}
}


int main(int argc, char* argv[])
{
// 	ObjWriter objWriter =  ObjWriter("D:\\code\\C++\\cax\\deploy\\convertor\\SLMConvertor\\tmp\\pcilindar_asm_214\\wdir\\otag\\otag_0\\bos3obj\\0\\1.bobj", ObjWriter::EncodeFormat::BINARY);
// 
// 	vector<int> dataBuffer;
// 	dataBuffer.push_back(198);
// // 	dataBuffer.push_back(198);
// // 	dataBuffer.push_back(36);
// // 	dataBuffer.push_back(36);
// 
// 
// 	objWriter.WriteFaces(dataBuffer,false,true,false);
// 
// 
// 
// 
// 	objWriter.Flush();
// 
// 	objWriter.Close();


	std::map<string, int> mapArgs;
	mapArgs["-i"] = 0;
	mapArgs["-o"] = 1;
	mapArgs["-copy"] = 2;
	mapArgs["-json"] = 3;
	mapArgs["-largeobject"] = 4;
	mapArgs["-useMtlAsGroup"] = 5;
	mapArgs["-useObjAsGroup"] = 6;
	mapArgs["-ignoreGFlag"] = 7;
	mapArgs["-outputSubGroups"] = 8;
	mapArgs["-outputEdgeHash"] = 9;
	mapArgs["-checkArea"] = 10;
	mapArgs["-disableLog"] = 11;

	std::optional<CLIArguments>  optArgs = std::make_optional<CLIArguments>();

	if (argc < 2)
	{
		printf("Useage: ObjSplitter -i [inputObjFileFullName] -o [outputFolderFullPath]\n");
	}
	else
	{
		for (int i = 0; i < argc; ++i)
		{

			if (mapArgs.count(argv[i]) == 0)
			{
				continue;
			}

			switch (mapArgs[argv[i]])
			{
			case 0:
				optArgs->sourceFileFullName = (argc > i + 1) ? argv[i + 1] : "";
				i++;
				break;

			case 1:
				optArgs->outputFileFullName = (argc > i + 1) ? argv[i + 1] : "";
				i++;
				break;

			case 2:
				optArgs->copyData = true;
				break;

			case 3:
				optArgs->outputJson = true;
				break;

			case 4:
				optArgs->isLargeObject = true;
				break;

			case 5:
				optArgs->useMtlAsGroup = true;
				break;

			case 6:
				optArgs->useObjAsGroup = true;
				break;

			case 7:
				optArgs->ignoreGFlag = true;
				break;

			case 8:
				optArgs->outputSubGroups = true;
				break;

			case 9:
				optArgs->outputEdgeHash = true;
				break;

			case 10:
				optArgs->checkGroupArea = true;
				break;

			case 11:
				optArgs->disableLog = true;
				break;
			}
		}
	}


	try
	{
		long long length = fs::file_size(optArgs->sourceFileFullName);

		long long fileSizeInGB = length / 1024 / 1024 / 1024;

		bool bLargeObjectInSize = false;

		if (fileSizeInGB > 10)
		{
			bLargeObjectInSize = true;
		}

		ObjParser objParser;
		//加载obj文件
		objParser.Load(optArgs->sourceFileFullName, optArgs->useMtlAsGroup, optArgs->useObjAsGroup,
			optArgs->ignoreGFlag, optArgs->disableLog);

		objParser.WriteToBinary(optArgs->outputFileFullName,optArgs->isLargeObject || bLargeObjectInSize, optArgs->checkGroupArea);

		fs::path path_to_file = optArgs->sourceFileFullName;

		if (optArgs->outputSubGroups || optArgs->outputEdgeHash)
		{
			objParser.ProcessSubGroups(path_to_file.parent_path().string(), optArgs->isLargeObject || bLargeObjectInSize, optArgs->checkGroupArea, false, true);
		}

		if (optArgs->outputJson)
		{
			path_to_file = optArgs->outputFileFullName;
			auto jsonFullPath = path_to_file.parent_path().string() + "/identify.json";
			objParser.WriteIdentity(jsonFullPath);
		}

		// Copy all data from current folder to target folder
		if (optArgs->copyData)
		{
			DirectoryCopyMaterial(optArgs->sourceFileFullName, path_to_file.parent_path().string(), true);
		}

		auto start = std::chrono::high_resolution_clock::now();
		/*打印版本信息*/
		//printf("Step Export Version : %s\n", STEPVERSION);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		std::cout << "\nExecution Time: " << duration.count() << " s" << std::endl;
	}
	catch (exception& e)
	{
		printf("obj Parse Failed.\n");
		return 0;
	}

	printf("obj Parse Successed.\n");

	return 0;
}