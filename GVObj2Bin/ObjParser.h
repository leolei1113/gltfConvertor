#ifndef OBJPARSER_H_
#define OBJPARSER_H_

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <cmath>
#include <unordered_map>

using namespace std;
//class attrib_t;

class ObjParser
{
	enum GroupValidation
	{
		NoneFace,
		ZeroArea,
		Ok
	};
	struct Vertex
	{
		float X;
		float Y;
		float Z;

		Vertex()
		{
			X = 0.0;
			Y = 0.0;
			Z = 0.0;
		}

		Vertex(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}
	};

	struct edgeHashData
	{
		Vertex v0;
		Vertex v1;
		int nCount;
		edgeHashData()
		{
			nCount = 1;
		}

		edgeHashData(Vertex _v0, Vertex _v1, int _count)
		{
			v0 = _v0;
			v1 = _v1;
			nCount = _count;
		}
	};

	struct EdgeData
	{
		int face=0;
		int relativeV=0;
		Vertex normal;

		EdgeData(int _face, int _relativeV, Vertex n)
		{
			face = _face;
			relativeV = _relativeV;
			normal = n;
		}
	};


public:
	ObjParser() {
	};
	~ObjParser();

	//加载obj文件
	bool Load(string strObjFileName,bool bDisableOutput = false);
	//写入身份
	void WriteIdentity(string strOutputFileFullName);
	//处理子组   映射边数据其他内容暂未使用
	void ProcessSubGroups(string strOutputFolder, bool bIsLargeObject = false, bool bCheckGroupArea = false, bool bOutputSubGroups = true, bool bOutputEdgeHash = true);
	//写入二进制流数据
	void WriteToBinary(string strOutputFileFullName, bool bIsLargeObject = false, bool bCheckGroupArea = false);
private:
	//是否是有效组
	GroupValidation IsValidGroup(size_t faceNum,bool bCheckArea, vector<int> vecFaceIndices, vector<Vertex> vecVertices);

	Vertex CrossNormal(Vertex n1, Vertex n2)
	{
		return Vertex(n1.Y * n2.Z - n2.Y * n1.Z, n1.X * n2.Z - n2.X * n1.Z, n1.X * n2.Y - n2.X * n1.Y);
	}

	float VectorLength(Vertex n)
	{
		return sqrt(n.X * n.X + n.Y * n.Y + n.Z * n.Z);
	}

	Vertex GetNormal(Vertex vertices[2])
	{
		return CrossNormal(
			 Vertex(vertices[1].X - vertices[0].X, vertices[1].Y - vertices[0].Y, vertices[1].Z - vertices[0].Z),
			 Vertex(vertices[2].X - vertices[1].X, vertices[2].Y - vertices[1].Y, vertices[2].Z - vertices[1].Z));
	}

	//重新映射写入文件
	void RemapAndWriteToFile(int nGroupIndex, string strOutputFolder, vector<Vertex> vecVertices, vector<float> vecTextures, vector<float> vecNormals,
		long lVertexNum, long lFaceNum, bool bOutputUV, bool bOutputNormal, double dGroupFaceArea, bool bOutputPly, bool bOutputDesc,
		int nDescRes, bool bBinaryDesc, bool bPrebakeWireframe, int nEdgeAngleThreshold, vector<edgeHashData>& vecEdgeHashDatas,
		bool bOutputSubGroups, bool bOutputEdgeHash);
	//计算边
	void ComputeEdges(int nGroupIndex, int nEdgeAngleThreshold, vector<Vertex> vecGlobals,
		vector<edgeHashData>& vecEdgeHashDatas, string strOutputEdgeHashFileName);
private:

	bool m_bDisableOutput = false;

	bool m_bLoadObjSucess = false;

	bool m_bOutputEdgeHash = true;

	string m_strMtlLibName = "";
	//顶点坐标
	vector<Vertex> m_vecVertex;
};
#endif // ! OBJPARSER_H_

