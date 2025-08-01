#include "ObjWriter.h"
#include <algorithm>

 ObjWriter::ObjWriter(string strOutputFileFullName, EncodeFormat format /*= EncodeFormat.PLAIN*/)
{
	 mEncodeFormat = format;
	 if (format == EncodeFormat::PLAIN)
	 {
		 mTextWriter.open(strOutputFileFullName, ios::out);
	 }
	 else
	 {
		 mBinaryWriter.open(strOutputFileFullName, ios::out | ios::binary);
	 }
}

ObjWriter::~ObjWriter()
{

}

void ObjWriter::Close()
{
	if (EncodeFormat::PLAIN ==  mEncodeFormat)
	{
		mTextWriter.close();
	}
	else
	{
		mBinaryWriter.close();
	}
}

void ObjWriter::Clear()
{

}

void ObjWriter::Flush()
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		mTextWriter.flush();
	}
	else
	{
		mBinaryWriter.flush();
	}
}

bool ObjWriter::IsValid()
{
	return mTextWriter.is_open() || mBinaryWriter.is_open();
}

bool ObjWriter::IsOpen()
{
	if (mEncodeFormat == EncodeFormat::PLAIN)
	{
		return mTextWriter.is_open();
	}
	else
	{
		return mBinaryWriter.is_open();
	}
}

void ObjWriter::WriteString(string _string)
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		mTextWriter << _string << endl;
	}
}

void ObjWriter::WriteMtllib(string strMtllibName)
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		mTextWriter<< "mtllib " + strMtllibName << endl;
	}
	else
	{
		// Write placeholder  数据类型
		BinaryPlaceholder btype = BinaryPlaceholder::Mtllib;
		mBinaryWriter.write((const char*)&btype, sizeof(char));
 		// Write string length 数据长度
		int iSize = strMtllibName.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));
 		// Write data buffer 数据内容
 		mBinaryWriter.write(strMtllibName.data(), iSize);
	}
}

void ObjWriter::WriteGroup(string strGroupIndex, string strGroupName)
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		mTextWriter << "g " + strGroupIndex << endl;
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Group;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		auto subString = stoi(strGroupIndex.replace(0, 6, ""));
		mBinaryWriter.write((const char*)&subString, sizeof(int));

		// Group name
		btype = BinaryPlaceholder::GroupName;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		int iSize = strGroupName.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));
		
		// Write data buffer
		mBinaryWriter.write(strGroupName.data(), iSize);
	}
}

void ObjWriter::WriteMatrial(string strMaterialName)
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		mTextWriter << "usemtl " + strMaterialName << endl;
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Material;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		// Write string length
		int iSize = strMaterialName.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));
		// Write data buffer
		mBinaryWriter.write(strMaterialName.data(), iSize);
	}
}

void ObjWriter::WriteVertexs(vector<float> vecDataBuffer, bool bIsLargeObject /*= false*/)
{
	std::replace(vecDataBuffer.begin(), vecDataBuffer.end(), -0.00000000, 0.00000000);
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		int dataCount = vecDataBuffer.size() / 3;

		for (int i =0; i < dataCount; ++i)
		{
			mTextWriter<< "v" << vecDataBuffer[i*3+0]<<" " << vecDataBuffer[i * 3 + 1] <<" " << vecDataBuffer[i * 3 + 2] << endl;
		}
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Vertex;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		// Write data count
		int iSize = vecDataBuffer.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));

		// Write data buffer
		if (bIsLargeObject)
		{
			for (auto i = 0; i < vecDataBuffer.size(); ++i)
			{
				float tmp = vecDataBuffer[i];
				mBinaryWriter.write((const char*)&tmp, sizeof(tmp));
			}
		}
		else
		{
			mBinaryWriter.write((const char*)vecDataBuffer.data(), sizeof(float)* vecDataBuffer.size());
		}
	}
}

void ObjWriter::WriteUVs(vector<float> vecDataBuffer, bool bIsLargeObject /*= false*/)
{
	std::replace(vecDataBuffer.begin(), vecDataBuffer.end(), -0.00000000, 0.00000000);
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		int dataCount = vecDataBuffer.size() / 2;

		for (int i = 0; i < dataCount; ++i)
		{
			mTextWriter << "vt" << vecDataBuffer[i * 2 + 0] << " " << vecDataBuffer[i * 2 + 1]<< endl;
		}
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Uv;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		// Write data count
		int iSize = vecDataBuffer.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));

		// Write data buffer
		if (bIsLargeObject)
		{
			for (auto i = 0; i < vecDataBuffer.size(); ++i)
			{
				float tmp = vecDataBuffer[i];
				mBinaryWriter.write((const char*)&tmp, sizeof(tmp));
			}
		}
		else
		{
			mBinaryWriter.write((const char*)vecDataBuffer.data(), sizeof(float) * vecDataBuffer.size());
		}
	}
}

void ObjWriter::WriteNormals(vector<float> vecDataBuffer, bool bIsLargeObject /*= false*/)
{
	std::replace(vecDataBuffer.begin(), vecDataBuffer.end(), -0.00000000, 0.00000000);
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		int dataCount = vecDataBuffer.size() / 3;

		for (int i = 0; i < dataCount; ++i)
		{
			mTextWriter << "vn" << vecDataBuffer[i * 3 + 0] << " " << vecDataBuffer[i * 3 + 1] << " " << vecDataBuffer[i * 3 + 2] << endl;
		}
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Normal;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		// Write data count
		int iSize = vecDataBuffer.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));

		// Write data buffer
		if (bIsLargeObject)
		{
			for (auto i = 0; i < vecDataBuffer.size(); ++i)
			{
				float tmp = vecDataBuffer[i];

				mBinaryWriter.write((const char*)&tmp, sizeof(tmp));
			}
		}
		else
		{
			mBinaryWriter.write((const char*)vecDataBuffer.data(), sizeof(float) * vecDataBuffer.size());
		}
	}
}

void ObjWriter::WriteFaces(vector<int> vecDataBuffer, bool bHasUV, bool bHasNormal, bool bIsLargeObject /*= false*/)
{
	if (EncodeFormat::PLAIN == mEncodeFormat)
	{
		int nDataStride = (1 + (bHasUV ? 1 : 0) + (bHasNormal ? 1 : 0)) * 3;

		int nDataCount = vecDataBuffer.size() / nDataStride;

		for (auto i = 0; i < nDataCount; ++i)
		{
			if (bHasUV && bHasNormal)
			{
				mTextWriter<<"f " <<
					vecDataBuffer[i * nDataStride + 0] << "/" + vecDataBuffer[i * nDataStride + 1] << "/" + vecDataBuffer[i * nDataStride + 2] << " " <<
					vecDataBuffer[i * nDataStride + 3] << "/" + vecDataBuffer[i * nDataStride + 4] << "/" + vecDataBuffer[i * nDataStride + 5] << " " <<
					vecDataBuffer[i * nDataStride + 6] << "/" + vecDataBuffer[i * nDataStride + 7] << "/" + vecDataBuffer[i * nDataStride + 8] << endl;
			}
			else if (bHasUV && !bHasNormal)
			{
				mTextWriter<<"f " <<
					vecDataBuffer[i * nDataStride + 0] << "/" + vecDataBuffer[i * nDataStride + 1] << " " <<
					vecDataBuffer[i * nDataStride + 2] << "/" + vecDataBuffer[i * nDataStride + 3] << " " <<
					vecDataBuffer[i * nDataStride + 4] << "/" + vecDataBuffer[i * nDataStride + 5]  << endl;
			}
			else if (!bHasUV && bHasNormal)
			{
				mTextWriter << "f " <<
					vecDataBuffer[i * nDataStride + 0] << "//" << vecDataBuffer[i * nDataStride + 1] << " " <<
					vecDataBuffer[i * nDataStride + 2] << "//" << vecDataBuffer[i * nDataStride + 3] << " " <<
					vecDataBuffer[i * nDataStride + 4] << "//" << vecDataBuffer[i * nDataStride + 5] << endl;
			}
			else if (!bHasUV && !bHasNormal)
			{
				mTextWriter << "f " <<
					vecDataBuffer[i * nDataStride + 0] << " " <<
					vecDataBuffer[i * nDataStride + 1] << " " <<
					vecDataBuffer[i * nDataStride + 2] << endl;
			}
		}
	}
	else
	{
		// Write placeholder
		BinaryPlaceholder btype = BinaryPlaceholder::Face;
		mBinaryWriter.write((const char*)&btype, sizeof(char));

		// Write face flag
		bool flag = bHasUV ? 1 : 0;
		mBinaryWriter.write((const char*)&flag, sizeof(flag));
		flag = bHasNormal ? 1 : 0;
		mBinaryWriter.write((const char*)&flag, sizeof(flag));

		// Write data count
		int iSize = vecDataBuffer.size();
		mBinaryWriter.write((const char*)&iSize, sizeof(iSize));

		// Write data buffer
		if (bIsLargeObject)
		{
			for (auto i = 0; i < vecDataBuffer.size(); ++i)
			{
				int tmp = vecDataBuffer[i];
				mBinaryWriter.write((const char*)&tmp, sizeof(tmp));
			}
		}
		else
		{
			mBinaryWriter.write((const char*)vecDataBuffer.data(), sizeof(int) * vecDataBuffer.size());
		}

	}
}
