#include "FoamDataExchange.h"
#include "MeshData/meshSingleton.h"
#include "MeshData/meshKernal.h"
#include "MeshData/meshSet.h"
#include <QFileInfo>
#include <QTextStream>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <QFile>
#include <QMessageBox>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <QDir>

namespace MeshData {
	FoamDataExchange::FoamDataExchange(const QString& fileName, MeshOperation operation,
									   GUI::MainWindow* mw, int modelId)
		: //	_fileName(fileName),
		_operation(operation)
		, _meshData(MeshData::getInstance())
		, MeshThreadBase(fileName, operation, mw)
		, _modelId(modelId)
	{
		_fileName = fileName;
		_file	  = new QFile(_fileName);
	}

	FoamDataExchange::~FoamDataExchange()
	{
		if(_stream != nullptr)
			delete _stream;
		if(_file != nullptr)
			delete _file;
		_file	= nullptr;
		_stream = nullptr;
	}

	bool FoamDataExchange::readHeader()
	{
		QFileInfo tempinfo(_fileName);
		if(!tempinfo.exists())
			return false;
		QString name = tempinfo.fileName();
		QString path = tempinfo.filePath();
		QFile	tempfile(_fileName);
		if(!tempfile.open(QIODevice::ReadOnly))
			return false;
		// QTextStream* tempStream=new QTextStream(&tempfile);
		QTextStream tempStream(&tempfile);
		while(!tempStream.atEnd()) {
			if(!_threadRuning)
				return false;
			QString line = tempStream.readLine().simplified();
		}
		// delete tempStream;
		return false;
	}

	bool FoamDataExchange::write()
	{
		return _modelId >= 1;
	}

	void FoamDataExchange::run()
	{
		ModuleBase::ThreadTask::run();
		bool result = false;
		switch(_operation) {
			case MESH_READ:
				emit showInformation(tr("Import MSH Mesh File From \"%1\"").arg(_fileName));
				result = read();
				setReadResult(result);
				break;
			case MESH_WRITE:
				emit showInformation(tr("Export MSH Mesh File From \"%1\"").arg(_fileName));
				//			result = write();
				//			setWriteResult(result);
				break;
		}
		defaultMeshFinished();
	}

	bool FoamDataExchange::read()
	{
		_file = new QFile();
		QFileInfo info(_fileName);
		if(!info.exists())
			return false;
		_baseFileName = info.fileName();
		_filePath	  = info.filePath();
		_file->setFileName(_fileName);
		if(!_file->open(QIODevice::ReadOnly))
			return false;
		_stream						  = new QTextStream(_file);
		OpenFoamMeshReader*	   reader = new OpenFoamMeshReader;
		CFDOpenfoamMeshParser* parser = new CFDOpenfoamMeshParser;
		reader->setMeshFileParser(parser);
		bool ok = reader->readFile(_fileName.toStdString());
		if(!ok)
			return ok;
		vtkSmartPointer<vtkUnstructuredGrid> dataset = reader->getGrid();
		MeshKernal*							 k		 = new MeshKernal;
		k->setName(_baseFileName);
		k->setPath(_filePath);
		k->setMeshData((vtkDataSet*)dataset);
		_meshData->appendMeshKernal(k);
		int									 kid   = k->getID();
		std::unordered_map<int, std::string> zones = parser->getZones();
		QMap<int, std::string>				 zoneNames{};
		for(auto zone : zones) {
			zoneNames.insert(zone.first, zone.second);
		}
		std::unordered_map<int, std::pair<int, int>> zoneIndexes = parser->getCellZones();
		for(auto it : zoneIndexes) {
			int		 setid = it.first;
			MeshSet* set   = new MeshSet;
			set->setType(SetType::Element);
			if(zoneNames.contains(setid))
				set->setName(QString::fromStdString(zoneNames[setid]));
			else
				set->setName("ElementSet" + QString::number(it.first));
			MeshSet* s = _idsetList.value(setid);
			if(s != nullptr) {
				delete s;
				_idsetList.remove(setid);
			}
			_idsetList[setid] = set;
			int firstIndex	  = it.second.first;
			int lastIndex	  = it.second.second;
			for(int i = firstIndex; i <= lastIndex; ++i)
				set->appendTempMem(i - 1);
		}
		std::unordered_map<int, std::pair<int, int>> faceZones = parser->getPointZones();
		for(auto it : faceZones) {
			int		 setid = it.first;
			MeshSet* set   = new MeshSet;
			set->setType(SetType::Node);
			if(zoneNames.contains(setid))
				set->setName(QString::fromStdString(zoneNames[setid]));
			else
				set->setName("PointSet" + QString::number(it.first));
			MeshSet* s = _idsetList.value(setid);
			if(s != nullptr) {
				delete s;
				_idsetList.remove(setid);
			}
			_idsetList[setid] = set;
			int firstIndex	  = it.second.first;
			int lastIndex	  = it.second.second;
			for(int i = firstIndex; i <= lastIndex; ++i)
				set->appendTempMem(i - 1);
		}
		std::unordered_map<int, std::pair<int, int>> face2CellIndex =
			parser->getFaceIDtoCellIndex();
		for(auto faceZone : parser->getFaceZones()) {
			int zoneId = faceZone.first;
			if(zoneNames.contains(zoneId)) {
				BoundMeshSet* set = new BoundMeshSet;
				set->setName(QString::fromStdString(zoneNames[zoneId]));
				QMap<int, QVector<int>> cellFaces{};
				int						firstIndex = faceZone.second.first;
				int						lastIndex  = faceZone.second.second;
				for(int i = firstIndex; i <= lastIndex; ++i) {
					int cellIndex = face2CellIndex[i].first;
					int index	  = face2CellIndex[i].second;
					cellFaces[cellIndex - 1].push_back(index - 1);
				}
				MeshSet* s = _idsetList.value(zoneId);
				if(s != nullptr) {
					delete s;
					_idsetList.remove(zoneId);
				}
				_idsetList[zoneId] = set;
			}
		}
		QList<MeshSet*> setList = _idsetList.values();
		for(int i = 0; i < _idsetList.size(); ++i) {
			MeshSet* set = setList.at(i);
			set->setKeneralID(kid);
			_meshData->appendMeshSet(set);
		}
		delete parser;
		delete reader;
		return ok;
	}

	void CFDOpenfoamMeshParser::stringSplit(const std::string str, const char split,
											std::vector<std::string>& vec)
	{
		std::istringstream iss(str);
		std::string		   token;
		while(getline(iss, token, split)) {
			if(token.size() != 0)
				vec.push_back(token);
		}
	}

	void CFDOpenfoamMeshParser::stringTrimmed(std::string& str)
	{
		int first = str.find_first_not_of(" ");
		str.erase(0, first);
		int last = str.find_last_not_of(" ");
		if(last + 1 < str.size())
			str.erase(last + 1);
	}

	bool CFDOpenfoamMeshParser::setFile(const std::string& file)
	{
		m_File = file;
		return this->read();
	}

	std::vector<double> CFDOpenfoamMeshParser::getPoints()
	{
		return m_Points;
	}

	std::unordered_map<int, std::vector<int>> CFDOpenfoamMeshParser::getFaces()
	{
		return m_Faces;
	}

	std::unordered_map<int, std::vector<int>> CFDOpenfoamMeshParser::getCells()
	{
		return m_Cells;
	}

	std::unordered_map<int, int> CFDOpenfoamMeshParser::getCellsType()
	{
		return m_CellsType;
	}

	std::unordered_map<int, std::pair<int, int>> CFDOpenfoamMeshParser::getPointZones()
	{
		return m_PointZones;
	}

	std::unordered_map<int, std::pair<int, int>> CFDOpenfoamMeshParser::getFaceZones()
	{
		return m_FaceZones;
	}

	std::unordered_map<int, std::pair<int, int>> CFDOpenfoamMeshParser::getCellZones()
	{
		return m_CellZones;
	}

	std::unordered_map<int, std::pair<int, int>> CFDOpenfoamMeshParser::getFaceIDtoCellIndex()
	{
		return m_FaceIdToCellFaceIndex;
	}

	std::unordered_map<int, std::string> CFDOpenfoamMeshParser::getZones()
	{
		return m_Zones;
	}

	bool CFDOpenfoamMeshParser::read()
	{
		QFileInfo	  f{ QString::fromStdString(m_File) };
		QString		  path			= f.absoluteDir().absolutePath() + "/constant/polyMesh/";
		std::string	  pointsFile	= path.toStdString() + "/points";
		std::string	  facesFile		= path.toStdString() + "/faces";
		std::string	  ownerFile		= path.toStdString() + "/owner";
		std::string	  neighbourFile = path.toStdString() + "/neighbour";
		std::string	  boundaryFile	= path.toStdString() + "/boundary";

		std::ifstream inPoints{};
		inPoints.open(pointsFile, std::ios_base::in);
		if(!inPoints.is_open())
			return false;
		std::stringstream buffer;
		buffer << inPoints.rdbuf();
		m_PointsString = std::string(buffer.str());
		inPoints.close();
		bool p = this->readPoints();
		if(!p)
			return p;

		std::ifstream inFaces{};
		inFaces.open(facesFile, std::ios_base::in);
		if(!inFaces.is_open())
			return false;
		std::stringstream buffer1;
		buffer1 << inFaces.rdbuf();
		m_FacesString = std::string(buffer1.str());
		inFaces.close();
		bool fa = this->readFaces();
		if(!fa)
			return fa;

		std::ifstream inOwner{};
		inOwner.open(ownerFile, std::ios_base::in);
		if(!inOwner.is_open())
			return false;
		std::stringstream buffer2;
		buffer2 << inOwner.rdbuf();
		m_OwnerString = std::string(buffer2.str());
		inOwner.close();
		bool c = this->readOwner();
		if(!c)
			return c;

		std::ifstream inNeigh{};
		inNeigh.open(neighbourFile, std::ios_base::in);
		if(!inNeigh.is_open())
			return false;
		std::stringstream buffer3;
		buffer3 << inNeigh.rdbuf();
		m_NeighbourString = std::string(buffer3.str());
		inNeigh.close();
		bool n = this->readNeighbour();
		if(!n)
			return n;

		std::ifstream inBoundary{};
		inBoundary.open(boundaryFile, std::ios_base::in);
		if(!inBoundary.is_open())
			return false;
		std::stringstream buffer4;
		buffer4 << inBoundary.rdbuf();
		m_NeighbourString = std::string(buffer4.str());
		inBoundary.close();
		bool b = this->readBoundary();
		if(!b)
			return b;
		return true;
	}

	bool CFDOpenfoamMeshParser::readPoints()
	{
		int index = m_PointsString.find(m_FoamLabel);
		if(index == -1)
			return false;
		int end = m_PointsString.find("}", index);
		if(end == -1)
			return false;
		int			start  = m_PointsString.find("(", end);
		int			finish = m_PointsString.find_last_of(")", start);
		std::string ps	   = m_PointsString.substr(start + 1, end - start - 1);
		// std::vector<std::string> strList{};
		// CFDMeshFileParserBase::stringSplit(m_PointsString.substr(end + 1), '\n', strList);
		QStringList strList =
			QString::fromStdString(ps).split(QRegExp("[()]+"), QString::SkipEmptyParts);
		for(auto str : strList) {
			str = str.trimmed();
			if(str.size() == 0)
				continue;
			QStringList coords = str.split(" ", Qt::SkipEmptyParts);
			if(coords.size() == 3) {
				for(auto& coord : coords) {
					double x = coord.toDouble();
					m_Points.push_back(x);
				}
			}
		}
		return true;
	}

	bool CFDOpenfoamMeshParser::readFaces()
	{
		int index = m_FacesString.find(m_FoamLabel);
		if(index == -1)
			return false;
		int end = m_FacesString.find("}", index);
		if(end == -1)
			return false;
		std::vector<std::string> strList{};
		this->stringSplit(m_FacesString.substr(end + 1), '\n', strList);
		int faceID{ 1 };
		for(auto& str : strList) {
			this->stringTrimmed(str);
			if(str.size() == 0)
				continue;
			if(isdigit(str[0])) {
				int start = str.find_first_of('(');
				if(start == -1)
					continue;
				int end = str.find_last_of(')');
				if(end == -1)
					continue;
				std::string numStr = str.substr(0, start);
				int			num{};
				sscanf(numStr.c_str(), "%d", &num);
				std::string				 coordsStr = str.substr(start + 1, end - start - 1);
				std::vector<std::string> coords{};
				this->stringSplit(coordsStr, ' ', coords);
				std::vector<int> pointIdS{};
				if(coords.size() >= 3) {
					for(auto& coord : coords) {
						int id{ 0 };
						sscanf(coord.c_str(), "%d", &id);
						pointIdS.push_back(id + 1);
					}
					m_Faces[faceID++] = pointIdS;
				}
			}
		}
		return true;
	}

	bool CFDOpenfoamMeshParser::readOwner()
	{
		int index = m_OwnerString.find(m_FoamLabel);
		if(index == -1)
			return false;
		int end = m_OwnerString.find("}", index);
		if(end == -1)
			return false;
		int start = m_OwnerString.find("(", end);
		if(start == -1)
			return false;
		end = m_OwnerString.find(")", start);
		if(end == -1)
			return false;
		QString owners = QString::fromStdString(m_OwnerString.substr(start + 1, end - start - 1));
		QStringList strList = owners.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
		int			faceID{ 1 };
		for(auto& str : strList) {
			int cellID = str.toInt();
			m_Cells[cellID].push_back(faceID);
			m_FaceIdToCellFaceIndex[faceID] = std::make_pair(cellID, m_Cells[cellID].size());
			++faceID;
		}
		return true;
	}

	bool CFDOpenfoamMeshParser::readNeighbour()
	{
		int index = m_NeighbourString.find(m_FoamLabel);
		if(index == -1)
			return false;
		int end = m_NeighbourString.find("}", index);
		if(end == -1)
			return false;
		int start = m_NeighbourString.find("(", end);
		if(start == -1)
			return false;
		end = m_NeighbourString.find(")", start);
		if(end == -1)
			return false;
		QString neighbours =
			QString::fromStdString(m_NeighbourString.substr(start + 1, end - start - 1));
		QStringList strList = neighbours.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
		int			faceID{ 1 };
		for(auto& str : strList) {
			int cellID = str.toInt();
			if(cellID == -1)
				continue;
			m_Cells[cellID].push_back(faceID);
			m_FaceIdToCellFaceIndex[faceID] = std::make_pair(cellID, m_Cells[cellID].size());
			++faceID;
		}
		return true;
	}

	bool CFDOpenfoamMeshParser::readBoundary()
	{
		return true;
	}

	void OpenFoamMeshReader::setMeshFileParser(CFDOpenfoamMeshParser* parser)
	{
		m_Parser = parser;
	}

	bool OpenFoamMeshReader::readFile(const std::string& file)
	{
		if(m_Parser == nullptr)
			return false;
		bool ok = m_Parser->setFile(file);
		if(!ok)
			return ok;
		m_ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
		vtkSmartPointer<vtkPoints>						points = vtkSmartPointer<vtkPoints>::New();
		const std::vector<double>						ps	   = m_Parser->getPoints();
		const std::unordered_map<int, std::vector<int>> cells  = m_Parser->getCells();
		const std::unordered_map<int, std::vector<int>> faces  = m_Parser->getFaces();
		const std::unordered_map<int, int>				cellTyps = m_Parser->getCellsType();
		std::set<int>									usedFaces{};
		int												num = ps.size() / 3;
		for(int i = 0; i < num; ++i) {
			double coord[3]{ ps[i * 3], ps[i * 3 + 1], ps[i * 3 + 2] };
			points->InsertNextPoint(coord);
		}
		m_ugrid->SetPoints(points);
		std::unordered_map<int, std::vector<int>>::const_iterator it = cells.cbegin();
		while(it != cells.cend()) {
			std::vector<vtkIdType>	   pointIds{};
			vtkSmartPointer<vtkIdList> facesIDS = vtkSmartPointer<vtkIdList>::New();
			std::vector<int>		   faceids	= it->second;
			int						   index	= it->first;
			for(int id : faceids) {
				auto it = faces.find(id);
				if(it == faces.cend())
					continue;
				const std::vector<int> pids = it->second;
				usedFaces.insert(it->first);
				facesIDS->InsertNextId(pids.size());
				for(int pid : pids) {
					pointIds.push_back(pid - 1);
					facesIDS->InsertNextId(pid - 1);
				}
			}

			std::sort(pointIds.begin(), pointIds.end());
			auto ii = std::unique(pointIds.begin(), pointIds.end());
			pointIds.erase(ii, pointIds.end());
			m_ugrid->InsertNextCell(VTK_POLYHEDRON, pointIds.size(), pointIds.data(),
									faceids.size(), facesIDS->GetPointer(0));
			++it;
		}
		return true;
	}

	vtkSmartPointer<vtkUnstructuredGrid> OpenFoamMeshReader::getGrid()
	{
		return m_ugrid;
	}

} // namespace MeshData
