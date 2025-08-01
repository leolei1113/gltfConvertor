#ifndef OBJWRITER_H_
#define OBJWRITER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;

class ObjWriter
{

public:

	enum EncodeFormat
	{
		PLAIN=0,
		BINARY,
	};

	enum BinaryPlaceholder
	{
		Mtllib = 1,
		Material = 2,
		Group = 3,
		Vertex = 4,
		Uv = 5,
		Normal = 6,
		Face = 7,
		GroupName = 8,
	};

public:
	ObjWriter() 
	{
	};

	 ObjWriter(string strOutputFileFullName, EncodeFormat format = EncodeFormat::PLAIN);

	~ObjWriter();


	void Close();

	void Clear();

	void Flush();

	bool IsValid();

	bool IsOpen();
	//写入字符串
	void WriteString(string strString);
	//写入Mtllib名称
	void WriteMtllib(string strMtllibName);
	//写入组
	void WriteGroup(string strGroupIndex, string strGroupName);
	//写入mtl名称
	void WriteMatrial(string strMaterialName);
	//顶点
	void WriteVertexs(vector<float> vecDataBuffer, bool bIsLargeObject = false);
	//纹理坐标
	void WriteUVs(vector<float> vecDataBuffer, bool bIsLargeObject = false);
	//法线
	void WriteNormals(vector<float> vecDataBuffer, bool bIsLargeObject = false);
	//面
	void WriteFaces(vector<int> vecDataBuffer, bool bHasUV, bool bHasNormal, bool bIsLargeObject = false);
private:


	EncodeFormat mEncodeFormat = EncodeFormat::PLAIN;

	ofstream		mTextWriter;
	ofstream		mBinaryWriter;

// 	StreamWriter mTextWriter = null;
// 	BinaryWriter mBinaryWriter = null;

};

#endif //OBJWRITER_H_

