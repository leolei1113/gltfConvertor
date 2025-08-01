#include "ObjParser.h"
#include "ObjWriter.h"

#include <limits>
#include <regex>
#include <filesystem>
#include "tiny_obj_loader.h"

#define M_PI       3.14159265358979323846

tinyobj::attrib_t m_attrib; // 所有的数据放在这里
std::vector<tinyobj::shape_t> m_shapes;//子模型
std::vector<tinyobj::material_t> m_materials;

namespace fs = filesystem;
ObjParser::~ObjParser()
{

}

bool ObjParser::Load(string strObjFileName, bool bDisableOutput /*= false*/)
{
	m_strMtlLibName = "";
	m_bLoadObjSucess = false;
	m_bDisableOutput = bDisableOutput;
	m_vecVertex.clear();

	if (!fs::exists(fs::path(strObjFileName)))
	{
		std::cout << "obj file not exist: " << strObjFileName << std::endl;
		return false;
	}

	ifstream file(strObjFileName);
	std::string strLine;
	int nCurrentLine = 1;
	const int nMaxLineIndex = 100;
	if (!file.is_open()) {
		throw std::runtime_error("Unable to open file");
		return false;
	}

	while (std::getline(file, strLine)) {

		if (strLine._Starts_with("mtllib"))
		{
			m_strMtlLibName = std::regex_replace(strLine.replace(0, 7, ""), std::regex("^\\s+|\\s+$"), "");

			break;
		}

		if (nCurrentLine == nMaxLineIndex) {
			break;
		}
		++nCurrentLine;
	}

	fs::path spath = fs::path(strObjFileName).parent_path();

	string strBase = spath.string();

	const char* filename = strObjFileName.c_str();
	const char* basepath = strBase.c_str();
	bool triangulate = true;

	std::string warn;
	std::string err;

	m_bLoadObjSucess = tinyobj::LoadObj(&m_attrib, &m_shapes, &m_materials, &warn, &err, filename,
		basepath, triangulate);


	if (!warn.empty() && !m_bDisableOutput)
	{
		std::cout << "load obj warn: " << warn << std::endl;
	}

	if (!err.empty() && !m_bDisableOutput)
	{
		std::cout << "load obj error: " << err << std::endl;
		m_bLoadObjSucess = false;
		return false;
	}


	if (!m_bDisableOutput)
	{
		std::cout << "objects: " << m_shapes.size() << std::endl;
		std::cout << "vertices: " << m_attrib.vertices.size() / 3 << std::endl;
		std::cout << "texcoords: " << m_attrib.texcoords.size()/2 << std::endl;
		std::cout << "normals: " << m_attrib.normals.size() / 3 << std::endl;
		std::cout << "materials: " << m_materials.size()<< std::endl;
	}

	int index = 0;
	size_t iCount = m_attrib.vertices.size() / 3;
	for (size_t i = 0; i < iCount; ++i)
	{
		Vertex vertexTmp;
		vertexTmp.X = m_attrib.vertices[index++];
		vertexTmp.Y = m_attrib.vertices[index++];
		vertexTmp.Z = m_attrib.vertices[index++];
		m_vecVertex.push_back(vertexTmp);
	}
	return true;
}

void ObjParser::WriteIdentity(string strOutputFileFullName)
{
	ObjWriter jsonWriter = ObjWriter(strOutputFileFullName, ObjWriter::EncodeFormat::PLAIN);

	if (!jsonWriter.IsOpen())
	{
		return;
	}

	jsonWriter.WriteString("{");

	jsonWriter.WriteString("}");

	jsonWriter.Flush();

	jsonWriter.Close();

}

void ObjParser::ProcessSubGroups(string strOutputFolder, bool bIsLargeObject /*= false*/, bool bCheckGroupArea /*= false*/, bool bOutputSubGroups /*= true*/, bool bOutputEdgeHash /*= true*/)
{
	if (m_bLoadObjSucess)
	{
		string strOutputFolderFullPath = strOutputFolder;

		if (bOutputSubGroups)
		{
			strOutputFolderFullPath = strOutputFolder + "/gtag/";
			fs::create_directories(strOutputFolderFullPath);
		}

		long lVertexNum = 0;
		long lFaceNum = 0;
		double dGroupFaceArea = 0;

		if (m_bDisableOutput == false)
		{
			std::cout << "Writing out groups..." << std::endl;
		}

		vector<edgeHashData> vecEdgeHashDatas;

		for (int nGIdx = 0; nGIdx < m_shapes.size(); ++nGIdx)
		{
			RemapAndWriteToFile(nGIdx, strOutputFolderFullPath, m_vecVertex, m_attrib.texcoords,
				m_attrib.normals, lVertexNum, lFaceNum, false, true,
				dGroupFaceArea, false, false, 64, true, true, 80, vecEdgeHashDatas, bOutputSubGroups, bOutputEdgeHash);

			if (m_bDisableOutput == false)
			{
				string strTmp = nGIdx + "/" + m_shapes.size();
				std::cout << "\rWritting out group... " + strTmp << std::endl;
			}
		}
		//输出边数据
		if (bOutputEdgeHash)
		{
			std::ofstream file(strOutputFolder + "/0.eh.bin", std::ios::out | std::ios::trunc| std::ios::binary);

			int nCount = vecEdgeHashDatas.size();
			file.write((const char*)&nCount, sizeof(nCount));


			for (int ehdIndex = 0; ehdIndex < nCount; ++ehdIndex)
			{
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v0.X,sizeof(float));
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v0.Y, sizeof(float));
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v0.Z, sizeof(float));
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v1.X, sizeof(float));
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v1.Y, sizeof(float));
				file.write((const char*)&vecEdgeHashDatas[ehdIndex].v1.Z, sizeof(float));
			}

			file.flush();
			file.close();
		}
	}
}

void ObjParser::WriteToBinary(string strOutputFileFullName, bool bIsLargeObject /*= false*/, bool bCheckGroupArea /*= false*/)
{
	if (m_bLoadObjSucess)
	{
		ObjWriter objWriter = ObjWriter(strOutputFileFullName, ObjWriter::EncodeFormat::BINARY);
		if (!objWriter.IsOpen())
		{
			return;
		}

		//写入mtllib
		objWriter.WriteMtllib(m_strMtlLibName);

		//写入顶点坐标
		objWriter.WriteVertexs(m_attrib.vertices, bIsLargeObject);

		//法线
		objWriter.WriteNormals(m_attrib.normals, bIsLargeObject);
#if 1
		//纹理坐标
		objWriter.WriteUVs(m_attrib.texcoords, bIsLargeObject);

		if (!m_bDisableOutput)
		{
			std::cout << "Writting data..."<< std::endl;
		}

		int nInvalidGroupCounter_NoneFace = 0;
		int nInvalidGroupCounter_ZeroArea = 0;

		for (int nGIdx = 0; nGIdx < m_shapes.size(); ++nGIdx)
		{
			//每一个子shape
			tinyobj::shape_t& shape = m_shapes[nGIdx];
			//面数
			size_t num_face = shape.mesh.num_face_vertices.size();

			// Ignore invalid group
			vector<int> vecFaceIndices;
			for (int i =0; i < shape.mesh.indices.size(); ++i)
			{
				vecFaceIndices.push_back(shape.mesh.indices[i].vertex_index);
			}

			GroupValidation groupValidate = IsValidGroup(num_face,bCheckGroupArea, vecFaceIndices, m_vecVertex);

			if (groupValidate != GroupValidation::Ok)
			{
				if (groupValidate == GroupValidation::NoneFace)
				{
					nInvalidGroupCounter_NoneFace++;
				}
				else if (groupValidate == GroupValidation::ZeroArea)
				{
					nInvalidGroupCounter_ZeroArea++;
				}

				continue;
			}
	
			//写入组 
			objWriter.WriteGroup("group_" + to_string(nGIdx), shape.name);
			//暂时与旧版保持一直都为default
			//objWriter.WriteGroup("group_" + to_string(nGIdx), "default");


			//写入usemtl材质
			if (shape.mesh.material_ids.size() > 0 && m_materials.size() >0)
			{
				int index_mat = shape.mesh.material_ids[0];//每个面的材质
				if (m_materials.size() >index_mat)
				{
					objWriter.WriteMatrial(m_materials[index_mat].name);
				}
				else
				{
					objWriter.WriteMatrial("mat_0");
				}
			}

			//当前mesh下标（每个面递增3）
			size_t index_offset = 0;
			bool bHasUV = false;
			bool bHasNormal = false;
			//每一个面 是否有纹理坐标和法线
			for (size_t j = 0; j < shape.mesh.indices.size(); ++j)
			{
				if ((shape.mesh.indices[j].texcoord_index+1) > 0 && m_attrib.texcoords.size() > 0)
				{
					bHasUV = true;
				}

				if ((shape.mesh.indices[j].normal_index+1) > 0 && m_attrib.normals.size() > 0)
				{
					bHasNormal = true;
				}

				if (bHasUV && bHasNormal)
				{
					break;
				}
			}

			vector<int> vecFaceBuffer;
			for (size_t j = 0; j < shape.mesh.indices.size(); ++j)
			{
				vecFaceBuffer.push_back(shape.mesh.indices[j].vertex_index + 1);

				if (bHasUV)
				{
					vecFaceBuffer.push_back(shape.mesh.indices[j].texcoord_index + 1);
				}

				if (bHasNormal)
				{
					vecFaceBuffer.push_back(shape.mesh.indices[j].normal_index + 1);
				}
			}

			//写入面
			objWriter.WriteFaces(vecFaceBuffer, bHasUV, bHasNormal, bIsLargeObject);


			if (m_bDisableOutput == false)
			{
				string strTmp = to_string(nGIdx) + "/" + to_string(m_shapes.size());
				std::cout << "\rWritting data... " + strTmp << std::endl;
			}
		}

		if (m_bDisableOutput == false) 
			std::cout << "" << std::endl;;

		if (bCheckGroupArea)
		{
			if (m_bDisableOutput == false || nInvalidGroupCounter_NoneFace > 0 || nInvalidGroupCounter_ZeroArea > 0)
			{
				string strTmp = ">, ZeroArea<" + to_string(nInvalidGroupCounter_ZeroArea);
				std::cout << "Invalid groups: NoneFaces<" + to_string(nInvalidGroupCounter_NoneFace) + strTmp + ">" << std::endl;;
			}
		}
#endif
		objWriter.Flush();

		objWriter.Close();
	}
}

ObjParser::GroupValidation ObjParser::IsValidGroup(size_t faceNum, bool bCheckArea, vector<int> vecFaceIndices, vector<Vertex> vecVcrtices)
{
	if (faceNum <=0)
	{
		return GroupValidation::NoneFace;
	}

	if (bCheckArea)
	{

		float surfaceArea = 0.0f;
		size_t triCount = vecFaceIndices.size() / 3;
		for (size_t i =0 ;i < triCount; ++i)
		{
			size_t baseIndex = i * 3;
			Vertex corner = vecVcrtices[vecFaceIndices[baseIndex + 0]];
			Vertex a = Vertex(vecVcrtices[vecFaceIndices[baseIndex + 1]].X - corner.X, vecVcrtices[vecFaceIndices[baseIndex + 1]].Y - corner.Y, vecVcrtices[vecFaceIndices[baseIndex + 1]].Z - corner.Z);
			Vertex b = Vertex(vecVcrtices[vecFaceIndices[baseIndex + 2]].X - corner.X, vecVcrtices[vecFaceIndices[baseIndex + 2]].Y - corner.Y, vecVcrtices[vecFaceIndices[baseIndex + 2]].Z - corner.Z);
			float subArea = VectorLength(CrossNormal(a, b));
			surfaceArea += subArea;
		}

		if (surfaceArea < 0.00001f)
		{
			return GroupValidation::ZeroArea;
		}
	}

	return GroupValidation::Ok;
}

void ObjParser::RemapAndWriteToFile(int nGroupIndex, string strOutputFolder, vector<Vertex> vecVertices, vector<float> vecTextures, vector<float> vecNormals,
	long lVertexNum, long lFaceNum, bool bOutputUV, bool bOutputNormal, double dGroupFaceArea, bool bOutputPly, bool bOutputDesc,
	int nDescRes, bool bBinaryDesc, bool bPrebakeWireframe, int nEdgeAngleThreshold, vector<edgeHashData>& vecEdgeHashDatas, 
	bool bOutputSubGroups, bool bOutputEdgeHash)
{
#if 0  //未使用到

	int intMax = numeric_limits<int>::max();
	int intMin = numeric_limits<int>::min();

	float floatMax = numeric_limits<float>::max();
	float floatMin = numeric_limits<float>::min();

	float fMinX = numeric_limits<float>::max(), fMinZ = numeric_limits<float>::max(), fMinY = numeric_limits<float>::max();
	float fMaxX = numeric_limits<float>::min(), fMaxZ = numeric_limits<float>::min(), fMaxY = numeric_limits<float>::min();
	long lMinIndex = numeric_limits<long>::max(), lMaxIndex = numeric_limits<long>::min();


	double areaOfAllFaces = 0.0;

	string mtlFolder = "";
	string objFileName = "sub_" + to_string(ngroupIndex);
	objFileName += ".obj";

	string mtlFileName = "sub_" + to_string(ngroupIndex);
	mtlFileName+=".mtl";
	string dscFileName = "sub_" + to_string(ngroupIndex);
	dscFileName+=".info";
	string plyFileName = "sub_" + to_string(ngroupIndex);
	plyFileName+=".ply";
	string eaFileName = "sub_" + to_string(ngroupIndex);
	eaFileName += ".exa.json";
	string edgeHashFile = "sub_" + to_string(ngroupIndex);
	edgeHashFile += ".eh";

	if (outputSubGroups)
	{
	
	}
#endif

	if (bPrebakeWireframe)
	{
#if 0 
		vector<Vertex> vecEdgeColors;
		vecEdgeColors.push_back(Vertex(0.0f, 0.0f, 0.0f));
		vecEdgeColors.push_back(Vertex(0.0f, 0.0f, 1.0f));
		vecEdgeColors.push_back(Vertex(0.0f, 1.0f, 0.0f));
		vecEdgeColors.push_back(Vertex(0.0f, 1.0f, 1.0f));
		vecEdgeColors.push_back(Vertex(1.0f, 0.0f, 0.0f));
		vecEdgeColors.push_back(Vertex(1.0f, 0.0f, 1.0f));
		vecEdgeColors.push_back(Vertex(1.0f, 1.0f, 0.0f));
		vecEdgeColors.push_back(Vertex(1.0f, 1.0f, 1.0f));

		unordered_map<string, unsigned int> edgeColorMap;
		for (auto i = 0; i < vecEdgeColors.size(); ++i)
		{
			//edgeColorMap.Add(((byte)edgeColors[i].X + "-" + (byte)edgeColors[i].Y + "-" + (byte)edgeColors[i].Z), (uint)i);
			string strTmp = to_string(vecEdgeColors.at(i).X)  + "-";
			strTmp += to_string(vecEdgeColors.at(i).Y)+ "-";
			strTmp +=to_string(vecEdgeColors.at(i).Z);
			edgeColorMap.insert(std::pair<string, unsigned int>(strTmp, i));

		}
#endif
// 		for (var i = 0; i < edgeColors.Length; ++i)
// 		{
// 			eaWireframe.buffer.Add(new ExtraBuffer(edgeColors[i].X, edgeColors[i].Y, edgeColors[i].Z, 0.0f));
// 		}
		
		//计算边
		ComputeEdges(nGroupIndex,nEdgeAngleThreshold, vecVertices, vecEdgeHashDatas, "");
	}


}

void ObjParser::ComputeEdges(int nGroupIndex,int nEdgeAngleThreshold, vector<Vertex> vecGlobals,
	vector<edgeHashData>& vecEdgeHashDatas, string strOutputEdgeHashFileName)
{
	if (nEdgeAngleThreshold <=0)
	{
		//暂不实现
	}
	else
	{
		auto precisioDicemal = 4;
		auto precision = pow(10.0, precisioDicemal);
		auto thresholdDot = cos(M_PI /180.0* nEdgeAngleThreshold);

		int indexArr[3] = {0,0,0};
		string hashes[3];
		Vertex _triangles[3];

		unordered_map<string, EdgeData> edgeData;
		unordered_map<string, edgeHashData> edgeCountMap;
		vector<string> vecEdgeCountMapKey;

		//每一个子shape
		tinyobj::shape_t shape = m_shapes[nGroupIndex];
		//面数
		size_t num_face = shape.mesh.num_face_vertices.size();

		int nIndex = 0;
		for (size_t i =0; i < num_face; i++)
		{
			indexArr[0] =shape.mesh.indices[nIndex++].vertex_index;
			indexArr[1] = shape.mesh.indices[nIndex++].vertex_index;
			indexArr[2] = shape.mesh.indices[nIndex++].vertex_index;

			_triangles[0] = vecGlobals[indexArr[0]];
			_triangles[1] = vecGlobals[indexArr[1]];
			_triangles[2] = vecGlobals[indexArr[2]];
			// TODO Get Normal
			Vertex _normal = GetNormal(_triangles);

			// create hashes for the edge from the vertices
			hashes[0] = to_string(lround(_triangles[0].X * precision)) + "," + to_string(lround(_triangles[0].Y * precision)) + "," + to_string(lround(_triangles[0].Z * precision));
			hashes[1] = to_string(lround(_triangles[1].X * precision)) + "," + to_string(lround(_triangles[1].Y * precision)) + "," + to_string(lround(_triangles[1].Z * precision));
			hashes[2] = to_string(lround(_triangles[2].X * precision)) + "," + to_string(lround(_triangles[2].Y * precision)) + "," + to_string(lround(_triangles[2].Z * precision));

			// skip degenerate triangles
			if (hashes[0] == hashes[1] || hashes[1] == hashes[2] || hashes[2] == hashes[0])
			{
				continue;
			}

			// iterate over every edge
			for (auto j =0; j < 3; j++)
			{
				// get the first and next vertex making up the edge
				auto jNext = (j + 1) % 3;
				auto jNextNext = (j + 2) % 3;

				auto vecHash0 = hashes[j];
				auto vecHash1 = hashes[jNext];

				string hash = vecHash0 + '_' + vecHash1;
				string reverseHash = vecHash1 + '_' + vecHash0;


				if (m_bOutputEdgeHash)
				{
					bool hasFound = false;

					auto it = edgeCountMap.find(hash);
					if (it != edgeCountMap.end()) {
						hasFound = true;
						edgeCountMap[hash].nCount++;

					}
					else {
						auto it1 = edgeCountMap.find(reverseHash);
						if (it1 != edgeCountMap.end())
						{
							hasFound = true;
							edgeCountMap[reverseHash].nCount++;
						}
					}

					if (hasFound == false)
					{
						// if we've already got an edge here then skip adding a new one 
						edgeCountMap.insert(pair<string, edgeHashData>(hash, edgeHashData(_triangles[j], _triangles[jNext], 1)));
						//存储键值保证数据顺序
						vecEdgeCountMapKey.push_back(hash);
					}
				}
				else //暂不实现
				{
					
				}
			}
		}

		if (m_bOutputEdgeHash)
		{
			for (const auto& strKey : vecEdgeCountMapKey)
			{
				auto it = edgeCountMap.find(strKey);
				if (it != edgeCountMap.end()) {
				
					if (it->second.nCount == 1)
					{
						vecEdgeHashDatas.push_back(it->second);
					}
				}
			}
		}

		// iterate over all remaining, unmatched edges and add them to the vertex array
		//暂不实现

	}
}

