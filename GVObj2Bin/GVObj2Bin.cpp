#include "GVObj2Bin.h"

#include "ObjParser.h"
#include "ObjWriter.h"

#include <filesystem>
#include <optional>
#include <map>

using namespace std;
namespace fs = filesystem;

static bool endsWith(const string& str, const std::string& suffix)
{
	return str.rfind(suffix) == str.size() - suffix.size();
}

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
			//std::cout << entry.path() << std::endl;
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
				DirectoryCopyMaterial(entry.path().string(), destDirName, copySubDirs);
			}
		}
	}
}


GVObj2Bin::GVObj2Bin()
{

}

GVObj2Bin::~GVObj2Bin()
{

}

bool GVObj2Bin::ReadObjFile(std::string strInputFile, std::string strOutFile, bool bCopyData, bool bOutputJson, bool bLargeObject,
	 bool bCheckGroupArea, bool bOutputSubGroups,bool bOutputEdgeHash, bool bDisableLog)
{
	try
	{
		long long lLength = fs::file_size(strInputFile);

		long long lFileSizeInGB = lLength / 1024 / 1024 / 1024;

		bool bLargeObjectInSize = false;

		if (lFileSizeInGB > 10)
		{
			bLargeObjectInSize = true;
		}

		ObjParser objParser;
		//加载obj文件
		objParser.Load(strInputFile, bDisableLog);
		//写入二进流数据
		objParser.WriteToBinary(strOutFile, bLargeObject || bLargeObjectInSize, bCheckGroupArea);

		fs::path path_to_file = strInputFile;

		if (bOutputSubGroups || bOutputEdgeHash)
		{
			//映射计算输出了边数据
			objParser.ProcessSubGroups(path_to_file.parent_path().string(), bLargeObject || bLargeObjectInSize, bCheckGroupArea, false, true);
		}

		if (bOutputJson)
		{
			path_to_file = strOutFile;
			auto jsonFullPath = path_to_file.parent_path().string() + "/identify.json";
			objParser.WriteIdentity(jsonFullPath);
		}

		// Copy all data from current folder to target folder
		if (bCopyData)
		{
			DirectoryCopyMaterial(strInputFile, path_to_file.parent_path().string(), true);
		}


	}
	catch (exception& e)
	{
		printf("obj Parse Failed.\n");
		return false;
	}

	printf("obj Parse Successed.\n");

	return true;
}

