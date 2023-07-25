#ifndef _MSHDATAEXCHANGE_H_
#define _MSHDATAEXCHANGE_H_

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

	enum meshType
	{
		typeNone = 0,
		typeGambit, typeFluent,
	};
	//面的信息
	typedef struct
	{
		int *vertics;
		int cl;
		int cr;
		int type;
		QList<int> pointList;
	}FaceData;

	typedef struct
	{
		int first_index;
		int last_index;
		int zone_id;
		int nd;
		int type;
		int element_type;
		QList<FaceData> facedatas;
	}MshFaces;//13，面列表

	class MESHDATAEXCHANGEPLUGINAPI MSHdataExchange : public MeshThreadBase
	{
	public:
		MSHdataExchange(const QString& fileName, MeshOperation operation, GUI::MainWindow *mw, int modelId = -1);
		~MSHdataExchange();

		bool read();
		bool write();
		void run() ;

	private:
		meshType mMeshType{ typeNone };
		int _totalNumber;

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
	 * @brief   msh文件解析类。解析后所得信息中索引均以1开始，使用vtk渲染时注意索引的转换。
	 */
	class CFDMshFileParser
	{
	public:
		CFDMshFileParser() = default;
		~CFDMshFileParser() = default;
		/**
		 * 指定要解析的文件并执行解析。
		 * @param  file
		 * @return 成功则返回true，否则返回false。
		 */
		bool setFile(const std::string& file);
		/**
		 * 获取点坐标信息。
		 * @return 以[x1,y1,z1,x2,y2,z2，...]形式存储的所有点的坐标信息。
		 */
		std::vector<double> getPoints() ;
		/**
		 * 获取面拓扑信息。
		 * @return key为面ID，value为点ID数组。ID均以1开始。
		 */
		std::unordered_map<int, std::vector<int>> getFaces() ;
		/**
		 * 获取单元拓扑信息。
		 * @return key为单元ID，value为面ID数组。ID均以1开始。
		 */
		std::unordered_map<int, std::vector<int>> getCells() ;
		/**
		 * 获取单元类型信息。
		 * @return key为面ID，value为整型类型，表示的单元类型如下：
		 * 1：三节点三角形；2：四节点四面体；3：四节点四边形；4:八节点六面体；5:五节点金字塔形；6：六节点五面体；7：多面体。
		 */
		std::unordered_map<int, int> getCellsType() ;
		/**
		 * 获取点域。
		 * @return key为域ID，value为由点起始索引和结束索引组成的pair。
		 * @since  1.0.0
		 */
		std::unordered_map<int, std::pair<int, int>> getPointZones();
		/**
		 * 获取面域。
		 * @return key为域ID，value为由面起始索引和结束索引组成的pair。
		 * @since  1.0.0
		 */
		std::unordered_map<int, std::pair<int, int>> getFaceZones();
		/**
		 * 获取单元域。
		 * @return key为域ID，value为由单元起始索引和结束索引组成的pair。
		 * @since  1.0.0
		 */
		std::unordered_map<int, std::pair<int, int>> getCellZones();
		/**
		 * 获取面ID与单元面索引的映射。
		 * @return key为面ID，value为由单元索引及其面索引组成的pair。索引均以1开始。
		 * @since  1.0.0
		 */
		std::unordered_map<int, std::pair<int, int>> getFaceIDtoCellIndex();
		/**
		 * 获取域名称。
		 * @return key为域ID，value为域名称。
		 * @since  1.0.0
		 */
		std::unordered_map<int, std::string> getZones();
	protected:
		bool read();

	private:
		std::string m_File{};
		bool readPoints();
		bool readFaces();
		bool readCellsType();
		bool readZones();
		void parseFaceAndCells(int, const std::vector<std::string>&, int);
		void stringSplit(const std::string str, const char split, std::vector<std::string> &vec);
		void stringTrimmed(std::string& str);
	private:
		std::string m_ByteArray{};
		double m_Progress;
		int m_PointNumber{0};
		int m_FaceNumber{0};
		std::vector<double> m_Points{};
		std::unordered_map<int, std::vector<int>> m_Faces{};
		std::unordered_map<int, std::vector<int>> m_Cells{};
		std::unordered_map<int, int> m_CellsType{};
		std::unordered_map<int, std::pair<int, int>> m_PointZones{};
		std::unordered_map<int, std::pair<int, int>> m_FaceZones{};
		std::unordered_map<int, std::pair<int, int>> m_CellZones{};
		std::unordered_map<int, std::pair<int, int>> m_FaceIdToCellFaceIndex{};
		std::unordered_map<int, std::string> m_Zones{};
	};

	class CFDMeshReader
	{
	public:
		CFDMeshReader() = default;
		~CFDMeshReader() = default;
		void setMeshFileParser(CFDMshFileParser* parser);
		bool readFile(const std::string& file);
		vtkSmartPointer<vtkUnstructuredGrid> getGrid();

	private:
		CFDMshFileParser* m_Parser{ nullptr };
		vtkSmartPointer<vtkUnstructuredGrid> m_ugrid{ nullptr };
	};
}
#endif 