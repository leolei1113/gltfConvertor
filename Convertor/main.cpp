#include <iostream>
#include <fstream>
#include <string>
//C++ 17的文件路径处理
#include <filesystem> 
#include <windows.h>
#include <algorithm>

#include "xcafTools.h"
#include "slmTools.h"
#include "ObjClass.h"

namespace fs = std::filesystem;
fs::path inputFileOrFolder;
bool isCADMode = false;

std::string BaseWorkingDirectory;

std::string GenerateTaskID()
{
    std::string taskID = "";
    return taskID;
}

void CreateDir(std::vector<std::string> vecPaths)
{
    for (int i = 0; i < vecPaths.size(); i++)
    {
        fs::path xpath = vecPaths[i];
        if (!fs::exists(xpath) || !fs::is_directory(xpath))
        {
            bool bPath = fs::create_directories(xpath); 
            if (!bPath)
            {
                std::cout << "create path " + xpath.string() + " fail" << std::endl;
            }
        }
    }
}

void ProcessStepCAD(std::string taskConfigUUID)
{
    if (taskConfigUUID == "")
    {
        std::cout << "task uuid generate fail" << std::endl;
        return;
    }
    //中间文件夹，缩略
    if (1)  
    {
        std::string tmpFolder = BaseWorkingDirectory + "/slmConvertor/tmp";
        std::string slmFolder = tmpFolder + "/" + taskConfigUUID;
        std::string stpFolder = slmFolder + "/stp";
        std::string oTagFolder = slmFolder + "/otag";
        std::string baseSlmOutputFolder = slmFolder + "/slmoutput";
        std::string otagSlmOutputFolder = baseSlmOutputFolder + "/otag";
        std::string fullSlmOutputFolder = baseSlmOutputFolder + "/full";
        //work路径
        std::string slmWorkingDir = slmFolder + "/wdir";
        std::string slmWorkingDirFinal = slmWorkingDir + "/final";
        std::string slmWorkingDirOtag = slmWorkingDir + "/otag";
        //obj路径
        std::string objOutputFolder = slmFolder + "/obj";
        std::string srcObjOutputFolder = objOutputFolder + "/srcobj";
        std::string lodObjOutputFolder = objOutputFolder + "/lodobj";
        //最终轻量化文件夹
        std::string finalOutputFolder = BaseWorkingDirectory + "/assets/" + taskConfigUUID;
        std::string finalCadOutputFolder = finalOutputFolder + "/cad";
        //std::string finalCadJsonOutputFolder = finalOutputFolder + "/json";
        std::string finalSlmOutputFolder = finalOutputFolder + "/slm";

        //创建文件夹
        std::vector<std::string> vecPaths = { tmpFolder ,slmFolder ,stpFolder, oTagFolder ,baseSlmOutputFolder
        ,otagSlmOutputFolder ,fullSlmOutputFolder ,slmWorkingDir ,slmWorkingDirFinal ,slmWorkingDirOtag
        ,objOutputFolder ,srcObjOutputFolder ,lodObjOutputFolder
        ,finalOutputFolder ,finalCadOutputFolder ,finalSlmOutputFolder };
        CreateDir(vecPaths);
    }

    fs::path extension = inputFileOrFolder.extension();   //.stp
    std::string strExt = extension.string();
    std::string strFile = inputFileOrFolder.filename().string();
    //截出模型的名称
    int nPos = strFile.find(strExt);
    if (nPos != std::string::npos)
        strFile.erase(nPos, strExt.length());
    //将后缀全部小写
    std::transform(strExt.begin(), strExt.end(), strExt.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if(strExt ==".stp" || strExt == ".step")
    {
        xcafTools* xcaftool = new xcafTools();
        //创建承载整个模型的Obj
        objStruct objStore;
        bool bIfSuccess = xcaftool->ReadStpXcaf(inputFileOrFolder, BaseWorkingDirectory, objStore);

        if (bIfSuccess)
        {
            //实例化slm工具
            slmTools* slmtool = new slmTools();
            std::string strStpName = inputFileOrFolder.stem().string();
            bool bIfSplitSuccess = slmtool->SplitObj(strStpName, BaseWorkingDirectory);
            //轻量化整体模型
            if (1)
            {
                std::string strFullObj = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/obj/srcobj/" + strStpName + ".obj";
                std::string strFullOutput = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final";
                bool bSLMFull = slmtool->SLMProcess(BaseWorkingDirectory, "full", strFullObj, strFullOutput);
                //整体零件需要使用hierarchyParser
                std::string strExePathHP = BaseWorkingDirectory + "/slmConvertor/tools/hierarchyParser/hierarchyParser.exe";
                std::string strJson = BaseWorkingDirectory + "/assets/" + strStpName + "/cad/json";
                std::string strCommandHP = " -i " + BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/obj/srcobj/hierarchy.json"
                    + " -o " + strJson;
                fs::path pathJson = strJson;
                //创建json文件夹
                bool bCreate = true;
                if (!fs::is_directory(pathJson))
                {
                    bCreate = fs::create_directory(pathJson);
                }
                bool bFullHP = slmtool->ProcessTask(strExePathHP, strCommandHP);

                //对总装进行打包
                std::string stdDesCAD = BaseWorkingDirectory + "/assets/" + strStpName + "/cad";
                fs::path pathDesCAD = stdDesCAD;
                if(!fs::is_directory(pathDesCAD))
                {
                    bCreate = fs::create_directory(pathDesCAD);
                }
                //打包srcobj中的文件
                if (1)
                {
                    std::string strSource = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/obj/srcobj";
                    fs::path pathSource = strSource;
                    for (auto entry : fs::directory_iterator(pathSource))
                    {
                        auto path = entry.path();
                        fs::copy_file(path, stdDesCAD / path.filename(), fs::copy_options::overwrite_existing);
                    }
                }
                //打包stp文件
                if (1)
                {
                    std::string strStp = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/stp";
                    fs::path pathStp = strStp;
                    for (auto entry : fs::directory_iterator(strStp))
                    {
                        auto path = entry.path();
                        fs::copy_file(path, stdDesCAD / path.filename(), fs::copy_options::overwrite_existing);
                    }
                }
                //打包slm
                if (1)
                {
                    std::string strSLMCom = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/components.json";
                    std::string strSLMGeo = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/geoinfo.json";
                    std::string strSLMGro = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/groupdesc.json";
                    std::string strSLMOut = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/output.glb";
                    std::string strSLMPro = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/properties.json";
                    std::string strSLMSma = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/smatrix.json";
                    std::string strSLMStr = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/final/output/structdesc.json";
                    std::vector<std::string> vecZipCommand = { strSLMCom ,strSLMGeo ,strSLMGro ,strSLMOut ,strSLMPro ,strSLMSma ,strSLMStr };

                    fs::path pathSLMCom = strSLMCom; fs::path pathSLMGeo = strSLMGeo; fs::path pathSLMGro = strSLMGro; fs::path pathSLMOut = strSLMOut;
                    fs::path pathSLMPro = strSLMPro; fs::path pathSLMSma = strSLMSma; fs::path pathSLMStr = strSLMStr;

                    //7z
                    std::string str7z = BaseWorkingDirectory + "/slmConvertor/tools/7z/7za.exe";
                    std::string strZipFull = BaseWorkingDirectory + "/assets/" + strStpName + "/slm/full.zip";
                    for (int i = 0; i < vecZipCommand.size(); i++)
                    {
                        std::string zipCommand = str7z + " a " + strZipFull + " " + vecZipCommand[i];
                        bool bzip = slmtool->ProcessTask("", zipCommand);
                    }
                }
                //改名复制stats文件
                std::string strStatsOri = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/obj/srcobj/" + strStpName + "_stats.json";
                fs::path pathStatsOri = strStatsOri;
                std::string strNewStats = BaseWorkingDirectory + "/assets/" + strStpName + "/slm/stats.json";
                fs::copy(strStatsOri, strNewStats, fs::copy_options::overwrite_existing);

                //生成xxx-o.glb文件，用于剖切
                std::string strDesGlb = BaseWorkingDirectory + "/assets/" + strStpName + "/slm/full-o.glb";
                std::string strOGlbCommand = " -i " + strFullObj + " -o " + strDesGlb;
                std::string strGlbExe = BaseWorkingDirectory + "/slmConvertor/tools/obj2bin/obj2gltf.exe";
                bool bFullGlb = slmtool->ProcessTask(strGlbExe, strOGlbCommand);
            }

            //如果拆分正常，开始每个obj的轻量化
            if (bIfSplitSuccess)
            {
                std::string strTagContainer = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/otag";
                std::string strBObjContainer = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag";
                //装obj的otag文件夹不存在
                fs::path tagContainer = strTagContainer;
                if (!fs::is_directory(tagContainer))
                {
                    std::cout << "Splitted oTag obj folders do not exist." << std::endl;
                    return;
                }
                //装bobj的otag文件夹不存在
                if (!fs::is_directory(strBObjContainer))
                {
                    std::cout << "oTag bobj folders do not exist." << std::endl;
                    return;
                }

                std::vector<fs::path> vecSubdirectories;
                for (auto iter : fs::directory_iterator(tagContainer))
                {
                    if (fs::is_directory(iter.status()))
                    {
                        vecSubdirectories.push_back(iter.path());
                    }
                }
                if (vecSubdirectories.size() == 0)
                {
                    std::cout << "Splitted oTag subdirectories are empty." << std::endl;
                    return;
                }

                for (auto iter : vecSubdirectories)
                {
                    std::string strXName = iter.stem().string();
                    std::string strOtagBase = iter.string();
                    //查找并输入文件夹的基准
                    int nPos = strOtagBase.find("otag");
                    std::string strInputBase = strOtagBase.substr(0, nPos);  //G:\Convertor\x64\Debug/slmConvertor/tmp/pcilindar_asm_214/

                    std::string strInputObj = strOtagBase + "/" + strXName + ".obj";
                    std::string strOutputBase = strInputBase + "wdir/otag/" + strXName;
                    bool bSLM = slmtool->SLMProcess(BaseWorkingDirectory, strXName, strInputObj, strOutputBase);

                    //打包slm
                    if (1)
                    {
                        std::string strSLMCom = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/components.json";
                        std::string strSLMGeo = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/geoinfo.json";
                        std::string strSLMGro = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/groupdesc.json";
                        std::string strSLMOut = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/output.glb";
                        std::string strSLMPro = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/properties.json";
                        std::string strSLMSma = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/smatrix.json";
                        std::string strSLMStr = BaseWorkingDirectory + "/slmConvertor/tmp/" + strStpName + "/wdir/otag/" + strXName + "/output/structdesc.json";
                        std::vector<std::string> vecZipCommand = { strSLMCom ,strSLMGeo ,strSLMGro ,strSLMOut ,strSLMPro ,strSLMSma ,strSLMStr };

                        fs::path pathSLMCom = strSLMCom; fs::path pathSLMGeo = strSLMGeo; fs::path pathSLMGro = strSLMGro; fs::path pathSLMOut = strSLMOut;
                        fs::path pathSLMPro = strSLMPro; fs::path pathSLMSma = strSLMSma; fs::path pathSLMStr = strSLMStr;

                        //7z
                        std::string str7z = BaseWorkingDirectory + "/slmConvertor/tools/7z/7za.exe";
                        std::string strZipFull = BaseWorkingDirectory + "/assets/" + strStpName + "/slm/otag_" + strXName + ".zip";
                        for (int i = 0; i < vecZipCommand.size(); i++)
                        {
                            std::string zipCommand = str7z + " a " + strZipFull + " " + vecZipCommand[i];
                            bool bzip = slmtool->ProcessTask("", zipCommand);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(NULL, buffer, MAX_PATH) != 0)
    {
        BaseWorkingDirectory = buffer;
        int nLast = BaseWorkingDirectory.rfind('\\');
        BaseWorkingDirectory = BaseWorkingDirectory.substr(0, nLast);
    }
    else
    {
        std::cout << "get working path failed" << std::endl;
        return -1;
    }

    if (argc < 2)
    {
        std::cout << "At least need -i inputFileOrFolder -t taskConfigTemplate.json" << std::endl;
        return -2;
    }
    for (int i = 0; i < argc; i++)
    {
        std::string strArg = argv[i];
        if (strArg == "-i")
        {
            inputFileOrFolder = (argc > i + 1) ? argv[i + 1] : "";
        }
        else if (strArg == "-cad")
        {
            isCADMode = true;
        }
    }

    //文件路径是否正确
    if (!fs::exists(inputFileOrFolder))
    {
        std::cout << "Stp file does not exist!" << std::endl;
        return -3;
    }

    std::string strName = inputFileOrFolder.stem().string();

    std::string taskConfigUUID = GenerateTaskID();
    if (taskConfigUUID == "")
        taskConfigUUID = strName;

    std::string strNewTask = "New Task: " + taskConfigUUID;
    std::cout << strNewTask << std::endl;


    if (isCADMode)
    {
        ProcessStepCAD(taskConfigUUID);
    }
}