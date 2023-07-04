#pragma once
#include "MeshThreadBase.h"
#include "meshDataExchangePlugin.h"
#include <QHash>
#include <unordered_map>
#include <vtkSmartPointer.h>

class QTextStream;
class vtkUnstructuredGrid;
class QFile;

namespace MeshData
{
	class MeshData;
	class MeshSet;

	class MESHDATAEXCHANGEPLUGINAPI FoamDataExchange : public MeshThreadBase
	{
	public:
		FoamDataExchange(const QString& fileName, MeshOperation operation, GUI::MainWindow *mw, int modelId = -1);
		~FoamDataExchange();

		bool read();
		bool readHeader();
		bool write();
		void run();

	private:
		QFile *_file;
		QString _baseFileName{};
		QString _filePath{};
		QString _fileName{};
		MeshData* _meshData{};
		QTextStream* _stream{};
		QHash<int, MeshSet*> _idsetList{};

		QString _describe{};
		int _ND{ 3 };
		int _staticid{ 0 };
		int _modelId;
		MeshOperation _operation;
	};
	/**
	 * @brief   OpenFOAM Mesh文件解析类。解析后所得信息中索引均以1开始，使用vtk渲染时注意索引的转换。
	 */
	class CFDOpenfoamMeshParser
	{
	public:
		static void stringSplit(const std::string str, const char split, std::vector<std::string> &vec);
		static void stringTrimmed(std::string& str);
	public:
		CFDOpenfoamMeshParser() = default;
		~CFDOpenfoamMeshParser() = default;
		bool setFile(const std::string& file);
		std::vector<double> getPoints();
		std::unordered_map<int, std::vector<int>> getFaces();
		std::unordered_map<int, std::vector<int>> getCells();
		std::unordered_map<int, int> getCellsType();
		std::unordered_map<int, std::pair<int, int>> getPointZones();
		std::unordered_map<int, std::pair<int, int>> getFaceZones();
		std::unordered_map<int, std::pair<int, int>> getCellZones();
		std::unordered_map<int, std::pair<int, int>> getFaceIDtoCellIndex();
		std::unordered_map<int, std::string> getZones();
	protected:
		bool read();
	private:
		bool readPoints();
		bool readFaces();
		bool readOwner();
		bool readNeighbour();
		bool readBoundary();
	private:
		std::string m_File{};
		std::string m_PointsString{};
		std::string m_FacesString{};
		std::string m_OwnerString{};
		std::string m_NeighbourString{};
		std::string m_BoundaryString{};
		std::vector<double> m_Points{};
		std::unordered_map<int, std::vector<int>> m_Faces{};
		std::unordered_map<int, std::vector<int>> m_Cells{};
		std::unordered_map<int, int> m_CellsType{};
		std::unordered_map<int, std::pair<int, int>> m_PointZones{};
		std::unordered_map<int, std::pair<int, int>> m_FaceZones{};
		std::unordered_map<int, std::pair<int, int>> m_CellZones{};
		std::unordered_map<int, std::pair<int, int>> m_FaceIdToCellFaceIndex{};
		std::unordered_map < int, std::string > m_Zones{};
		const std::string m_FoamLabel{ "FoamFile" };
		
	};

	class OpenFoamMeshReader
	{
	public:
		OpenFoamMeshReader() = default;
		~OpenFoamMeshReader() = default;
		void setMeshFileParser(CFDOpenfoamMeshParser* parser);
		bool readFile(const std::string& file);
		vtkSmartPointer<vtkUnstructuredGrid> getGrid();

	private:
		CFDOpenfoamMeshParser* m_Parser{ nullptr };
		vtkSmartPointer<vtkUnstructuredGrid> m_ugrid{ nullptr };
	};
}

