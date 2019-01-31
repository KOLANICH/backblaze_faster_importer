#include <initializer_list>
#include <iostream>
#include <regex>
#include <iomanip>
#include <ctime> // chrono sucks, so does date. Fuck off incomplete C++ impls like MinGW
#include <cmath>
#include <unordered_map>

#include "generated/config.h"

#if defined(CPP_DIR_ITERATION_WORKS)
		#include <filesystem> // Fuck Off MinGW-w64 https://sourceforge.net/p/mingw-w64/bugs/737/
#else
	#if defined(DIRENT_PRESENT)
		#include <dirent.h>
	#endif
#endif

#include <CLI/CLI.hpp>

#include "csv.hpp"
#include <sqlite3.h>
#include "ProgressBar.hpp"

#define private public
	#include <SQLiteCpp/SQLiteCpp.h>
	#include <SQLiteCpp/VariadicBind.h>
#undef private

#include "FasterImporter.h"

using namespace std;

struct Row{
	PackedRowidT packedRowid;
	uint32_t failure;
	#include "generated/tableColumnsNamesVars.h"
};


struct DriveWithRecords{
	Drive d;
	bool alreadyInBase = false;
	std::vector<Row> r;
	DriveWithRecords(const Drive &d, bool alreadyInBase=false): d(d), alreadyInBase(alreadyInBase){}
	DriveWithRecords(Drive &&d, bool alreadyInBase=false): d(std::move(d)), alreadyInBase(alreadyInBase){}
};

struct ModelWithDrives{
	Model m;
	bool alreadyInBase = false;
	std::unordered_map<string, DriveWithRecords> serialDriveMap;
	
	ModelWithDrives(Model &&m, bool alreadyInBase=false):m(std::move(m)), alreadyInBase(alreadyInBase){}
	ModelWithDrives(Model &m, bool alreadyInBase=false):m(m), alreadyInBase(alreadyInBase){}
	ModelWithDrives(ModelWithDrives && ) = default;
};


uint32_t firstFreeDriveId=1;
uint32_t firstFreeModelId=1;
std::unordered_map<string, ModelWithDrives> modelMap;

time_t getEpoch(){
	tm tp = {};
	std::stringstream is{"1970-01-01"};
	is >> get_time(&tp, "%Y-%m-%d");
	return mktime(&tp);
}
auto unixEpoch=getEpoch();

template<typename ResT> ResT parseDate(string &dateStr){
	tm tp = {};
	std::stringstream is{dateStr};
	is >> get_time(&tp, "%Y-%m-%d");
	tp.tm_min=tp.tm_hour=0; // UTC
	//std::cerr << std::put_time(&tp, "%c") << std::endl;
	auto tsTT=mktime(&tp)-1;
	//auto ts=time(&tsTT);
	auto ts=std::lround(difftime(tsTT, unixEpoch));
	//cerr << "ts " << ts << std::endl;
	ts=static_cast<ResT>(ts / secondsInDay);
	return ts;
}


/*constexpr uint16_t getCountOfSMARTColumns() {
	constexpr const std::initializer_list<const char* const> il{
		#include "generated/tableColumnsNamesStrings.h"
	};
	return il.size();
}*/

struct ProcessReport{
	ModelIdT models=0u;
	DriveIdT drives=0u;
	DriveIdT records=0u;
};

ProcessReport parseTheStuff(const char * fileName){
	ProcessReport report;
	
	//constexpr auto countOfColumns=getCountOfSMARTColumns();
	constexpr const auto countOfSMARTColumns=
	#include "generated/countOfSMARTColumns.h"
	;
	
	io::CSVReader<countOfSMARTColumns+5> in(fileName);
	in.read_header(io::ignore_extra_column, "date", "serial_number", "model", "capacity_bytes", "failure", 
	#include "generated/tableColumnsNamesStrings.h"
	);
	
	string dateStr;

	
	for(;;){
		Model model;
		Drive d;
		Row row;
		
		string modelName;
		string driveSerial;
		
		
		auto res=in.read_row(
			dateStr, driveSerial, modelName, model.capacity_bytes, row.failure,
			#include "generated/tableColumnsNamesVars1.h"
		);
		
		if(!res)
			break;
		
		ModelWithDrives *mWD = nullptr;
		{
			//cerr << modelName << " sn " << d.serial << " " << mWD << std::endl;
			auto it = modelMap.find(modelName);
			if(it != end(modelMap)){
				mWD = &(it->second);
				//cerr << "found model " << modelName << " " << mWD << " " << &mWD->serialDriveMap << " " << mWD->serialDriveMap.size() <<  std::endl;
			} else {
				model.id=firstFreeModelId;
				++firstFreeModelId;
				//cerr << "Adding model: " << modelName << " id: " << model.id << std::endl;
				mWD = &(modelMap.emplace(std::move(modelName), std::move(ModelWithDrives(model, false))).first->second);
				++report.models;
			}
		}
		
		DriveWithRecords *dWR;
		{
			auto it = mWD->serialDriveMap.find(driveSerial);
			if(it != end(mWD->serialDriveMap)){
				dWR = &(it->second);
				//cerr << "Found drive: " << driveSerial << " " << dWR->d.id << std::endl;
			} else {
				d.id = firstFreeDriveId;
				++firstFreeDriveId;
				//cerr << "Adding drive: " << driveSerial << " " << d.id << std::endl;
				
				auto iP = mWD->serialDriveMap.emplace(std::move(driveSerial), DriveWithRecords(d, false)).first;
				dWR = &(iP->second);
				++report.drives;
			}
		}
		
		row.packedRowid=packRowid(dWR->d.id, parseDate<uint32_t>(dateStr));
		//cerr << "Adding row: " << driveSerial << " " << dateStr << " " << dWR->d.id << " "  << parseDate(dateStr) << row.packedRowid << std::endl;
		dWR->r.emplace_back(row);
		++report.records;
	}
	return report;
}

void saveTheStuff(const char* dbFileName){
	SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
	SQLite::Statement modelsQuery(db, "INSERT INTO `models` (id, name, brand_id) VALUES(:id, :modelName, :brandId);");
	SQLite::Statement drivesQuery(db, "INSERT INTO `drives` (id, model_id, serial_number) VALUES(:id, :model_id, :serial_number);");
	SQLite::Statement recordsQuery(
		db, 
		
		"INSERT INTO `drive_stats` (`packed_rowid`, `failure`, "
		#include "generated/tableColumnsNames.h"
		") VALUES ( :packed_rowid, :failure, "
		#include "generated/tableColumnsPlaceholders.h"
		")"
	);
	
	std::vector<ModelWithDrives> serialModelTempVec;
	//std::vector<std::unordered_map<string, DriveWithRecords>::value_type> serialDriveTempVec;
	std::vector<std::pair<string, DriveWithRecords> > serialDriveTempVec;
	
	auto modelIdIdx=sqlite3_bind_parameter_index(modelsQuery.mStmtPtr, ":id");
	auto modelNameIdx=sqlite3_bind_parameter_index(modelsQuery.mStmtPtr, ":modelName");
	auto modelBrandIdIdx=sqlite3_bind_parameter_index(modelsQuery.mStmtPtr, ":brandId");
	auto capacityIdIdx=sqlite3_bind_parameter_index(modelsQuery.mStmtPtr, ":capacity_bytes");
	
	auto driveIdIdx=sqlite3_bind_parameter_index(drivesQuery.mStmtPtr, ":id");
	auto driveModelIdIdx=sqlite3_bind_parameter_index(drivesQuery.mStmtPtr, ":model_id");
	auto driveSerialIdx=sqlite3_bind_parameter_index(drivesQuery.mStmtPtr, ":serial_number");
	
	auto packedRowidIdx=sqlite3_bind_parameter_index(recordsQuery.mStmtPtr, ":packed_rowid");
	auto failureIdx=sqlite3_bind_parameter_index(recordsQuery.mStmtPtr, ":failure");
	#include "generated/variablesIndices.h"

	ProgressBar modelPb(modelMap.size(), 130);
	for(auto &mp: modelMap){
		auto &modelName=mp.first;
		auto &mWD=mp.second;
		//cerr << modelName << " " << &mWD << " " << &mWD.serialDriveMap << std::endl;
		
		SQLite::Transaction transaction(db);
		if(!mWD.alreadyInBase){
			//cout << "Inserting model id: " << mWD.m.id << " name: " << modelName << std::endl;
			
			modelsQuery.bind(modelIdIdx, mWD.m.id);
			modelsQuery.bindNoCopy(modelNameIdx, modelName);
			modelsQuery.bind(modelBrandIdIdx, 0);
			
			//modelsQuery.bind(capacityIdIdx, capacity_bytes);
			
			modelsQuery.executeStep();
			mWD.alreadyInBase=true;
			modelsQuery.reset(); // do I need it ?
		}
		
		serialDriveTempVec.clear();
		
		{
			//ProgressBar pb(mWD.serialDriveMap.size(), 130);
			//for(auto dPIt=begin(mWD.serialDriveMap), dPItEnd=end(mWD.serialDriveMap); dPIt!=dPItEnd; ++dPIt){
			for(auto &&dp: mWD.serialDriveMap){
				auto &&driveSerial=dp.first;
				auto &&dWR=dp.second;
		
				if(!dWR.alreadyInBase){
					//cerr << "adding into the list: id: "<< dWR.d.id << " serial: " << driveSerial << " model_id: " << mWD.m.id << " records: " << dWR.r.size() << std::endl;
					serialDriveTempVec.emplace_back(dp);
				}
				
				//cerr << "processing drive: id: "<< dWR.d.id << " serial: " << driveSerial << " model_id: " << mWD.m.id << " records: " << dWR.r.size() << " " <<  &dWR.r[0] << " " <<  &dWR.r << std::endl;
				
				for(auto &r: dWR.r){
					recordsQuery.bind(packedRowidIdx, r.packedRowid);
					recordsQuery.bind(failureIdx, r.failure);
					#include "generated/variablesBind.h"
					
					recordsQuery.executeStep();
				}
				dWR.r.clear();
				
				//++pb;
				//pb.display();
			}
		}
		
		std::sort(begin(serialDriveTempVec), end(serialDriveTempVec), [](std::pair<string, DriveWithRecords> a, std::pair<string, DriveWithRecords> b) -> bool { return a.second.d.id < b.second.d.id; });
		
		{
			//ProgressBar pb(serialDriveTempVec.size(), 130);
			for(auto &dWRNotInBasePair: serialDriveTempVec){
				auto &driveSerial=dWRNotInBasePair.first;
				auto &dWRNotInBase=dWRNotInBasePair.second;
				
				drivesQuery.bind(driveIdIdx, dWRNotInBase.d.id);
				drivesQuery.bind(driveModelIdIdx, mWD.m.id);
				drivesQuery.bindNoCopy(driveSerialIdx, driveSerial);
				//cerr << "id: " << dWRNotInBase.d.id << " serial: " << driveSerial << " model id: " << mWD.m.id << " records: " << dWRNotInBase.r.size() << std::endl;
				
				drivesQuery.executeStep();
				drivesQuery.reset(); // do I need it ?
				
				//++pb;
				//pb.display();
			}
		}
		
		transaction.commit();
		++modelPb;
		modelPb.display();
	}
}


template <typename T, typename IntType=uint16_t> std::unordered_map<std::string, IntType> getTableInfo(SQLite::Database &db, T &tableName){
	stringstream s;
	s<<"PRAGMA table_info(";
	s<<tableName;
	s<<");";
	
	SQLite::Statement pragmaQuery(db, s.str());
	std::unordered_map<std::string, uint16_t> pragmaMap;
	
	while (pragmaQuery.executeStep()) {
		std::string colN=pragmaQuery.getColumn(1);
		IntType colIdx=static_cast<IntType>(uint32_t(pragmaQuery.getColumn(0)));
		//cout << "colName: " << colN << " idx: " << colIdx << std::endl;
		pragmaMap.emplace(colN, colIdx);
	}
	return pragmaMap;
}


struct DrivesLoader{
	SQLite::Statement drivesQuery;
	uint8_t drivesQueryModelIdIdx, serialIdx, idIdx;
	
	DrivesLoader(SQLite::Database &db):
		drivesQuery(SQLite::Statement(db, "select * from `drives` where `model_id` = :modelId;")),
		drivesQueryModelIdIdx(static_cast<uint8_t>(sqlite3_bind_parameter_index(drivesQuery.mStmtPtr, ":modelId")))
	{
		auto modelsInfo = getTableInfo(db, "drives");
		serialIdx = static_cast<uint8_t>(modelsInfo["serial_number"]);
		idIdx = static_cast<uint8_t>(modelsInfo["id"]);
	}
	
	DriveIdT importModelDrives(ModelWithDrives &mWD){
		DriveIdT drivesImported=0u;
		drivesQuery.bind(drivesQueryModelIdIdx, mWD.m.id);
		while (drivesQuery.executeStep()) {
			Drive d;
			d.id = drivesQuery.getColumn(idIdx);
			string serial = drivesQuery.getColumn(serialIdx);
			
			if(d.id > firstFreeDriveId)
				firstFreeDriveId = d.id;
			
			//cout << "importing " << serial << " id: "<< d.id << std::endl;
			mWD.serialDriveMap.emplace(serial, DriveWithRecords(d, true));
			++drivesImported;
		}
		drivesQuery.reset();
		++firstFreeDriveId;
		return drivesImported;
	}
};

ProcessReport importModelsFromBase(SQLite::Database &db){
	ProcessReport report;
	SQLite::Statement modelsQuery(db, "select * from `models`;");
	
	auto modelsInfo = getTableInfo(db, "models");
	auto nameIdx=modelsInfo["name"];
	auto idIdx=modelsInfo["id"];
	auto brandIdIdx=modelsInfo["brand_id"];

	Model m;
	m.capacity_bytes = 0;
	DrivesLoader dI(db);
	while (modelsQuery.executeStep()) {
		m.id = modelsQuery.getColumn(idIdx);
		if(m.id > firstFreeModelId)
			firstFreeModelId = m.id;
		m.brandId = static_cast<decltype(m.brandId)>((unsigned int)(modelsQuery.getColumn(brandIdIdx)));
		string modelName = modelsQuery.getColumn(nameIdx);
		//cout << "importing " << modelName << std::endl;
		
		auto &mWD = modelMap.emplace(modelName, ModelWithDrives(m, true)).first->second;
		++report.models;
		report.drives+=dI.importModelDrives(mWD);
	}
	++firstFreeModelId;
	modelsQuery.reset();
	return report;
}

void init(const char *dbFileName){
	auto mmapPageLim=8u*1024u*1024u;
	sqlite3_config(SQLITE_CONFIG_MMAP_SIZE, mmapPageLim, mmapPageLim);
	sqlite3_initialize();
	
	SQLite::Database db(dbFileName);
	importModelsFromBase(db);
}

const std::regex csvFileNameRx("\\d{4}-\\d{2}-\\d{2}\\.csv$");


int main(int argc, char* argv[]){
	std::ios::sync_with_stdio(false);
	
	CLI::App app{"An importer which is much faster than the one implemented on python + SQLite"};
	std::string filename = "";
	
	std::vector<std::string> fileNames{};
	std::string dbFileName{"./a_test.sqlite"};
	uint8_t sizeOfBatch = 5;
	
	app.add_option("files", fileNames)->check(CLI::ExistingFile);
	app.add_option("--DB-file-name", dbFileName, "The file name of the database file")->check(CLI::ExistingFile);
	app.add_option("--batch-size", sizeOfBatch, "If you imported all the files into memory at once, you will run out of it");
	
	
	CLI11_PARSE(app, argc, argv);
	
	if(fileNames.size() == 0){
		std::smatch match;
		auto emplacer=[&match, &fileNames](string &&fileNameStr) -> void{
			if(std::regex_match(fileNameStr, match, csvFileNameRx)){
				fileNames.emplace_back(fileNameStr);
			}
		};
		
		const char defaultDirName[]=".";
		#if defined(CPP_DIR_ITERATION_WORKS)
				for(auto& f: fs::directory_iterator(defaultDirName)){
					emplacer(f.path());
				}
		#else
			#if defined(DIRENT_PRESENT)
				DIR *curD = opendir(defaultDirName);
				dirent *dirEntry;
				while ((dirEntry = readdir(curD))) {
					emplacer(string(dirEntry->d_name));
				}
				closedir(curD);
			#endif
		#endif
	}
	

	try{
		init(dbFileName.c_str());
		cout << "Imported models: " << modelMap.size() << std::endl;
		
		for(auto& fn: fileNames){
			cout << "parsing " << fn << std::endl;
			parseTheStuff(fn.c_str());
		}
		
		saveTheStuff(dbFileName.c_str());
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}