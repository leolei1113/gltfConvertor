#ifndef OBJ2BIN_H_
#define OBJ2BIN_H_
#include<string>


#ifdef GV_OBJ2BIN_LIB
#define GV_OBJ_2_BINN_API __declspec(dllexport)
#else
#define GV_OBJ_2_BINN_API __declspec(dllimport)
#endif


class GV_OBJ_2_BINN_API GVObj2Bin
{
public:
	GVObj2Bin();
	~GVObj2Bin();

	//入口 
	// inputFile->输入文件  outFile->输出文件   bCopyData->是否拷贝数据据   bOutputJson->是否输出identify.json   bLargeObject->是否是大对象
	// bCheckGroupArea->是否检查组区域     bOutputSubGroups->是否输出子组  bOutputEdgeHash->输出边数据   bDisableLog->日志输出
	bool ReadObjFile(std::string strInputFile, std::string strOutFile, bool bCopyData = false, bool bOutputJson = false, bool bLargeObject = false,
		 bool bCheckGroupArea = false, bool bOutputSubGroups = false,bool bOutputEdgeHash = false, bool bDisableLog = false);
};

#endif //OBJ2BIN_H_

