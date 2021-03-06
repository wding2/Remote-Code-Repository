// RepoCore.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "RepoCore.h"
#include "../CppNoSqlDb/PayLoad/PayLoad.h"


//test stub
#ifdef TEST_REPOCORE
int main()
{
	Utilities::StringHelper::title("check in Comm package");
	Utilities::StringHelper::title("check in Comm.h");
	DbCore<PayLoad> testDB_Repo;
	DbElement<PayLoad> temp;
	temp.name("123");
	temp.dateTime(DateTime().now());
	//if (!dependency.empty())
	//temp.children(dependency);
	temp.descrip("des");

	//set up payload
	PayLoad pl;
	//user can change state manually
	pl.value("path");
	pl.categories().push_back("test");
	temp.payLoad(pl);

	testDB_Repo["asd"] = temp;
	showDb(testDB_Repo);
	
	RepoCore<PayLoad> testCheckin = RepoCore<PayLoad>(testDB_Repo);
	
	vector<string> RepoCoreCpp_dpdcy;
	RepoCoreCpp_dpdcy.push_back("./../LocalStorage/Comm.cpp");

	testCheckin.RepoCheckin("./../LocalStorage/Comm.h", "descripTest", "userNameTest", RepoCoreCpp_dpdcy);
	testCheckin.closeCheckin("Comm.cpp");

	testCheckin.RepoCheckin("./../LocalStorage/Comm.cpp", "descripTest", "userNameTest");
	
	testCheckin.showRepoDB();

	
	
	//check in RepositoryCore.h
	//Utilities::StringHelper::title("check in RepositoryCore.h");
	
	//vector<string> RepoCoreH_dpdcy;
	//RepoCoreH_dpdcy.push_back("RepositoryCore.cpp");
	//RepoCoreH_dpdcy.push_back("DbCore.h");
	//testCheckin.checkinFiles(RepoCoreH, "RepositoryCore Package", RepoCoreH_dpdcy);
	////show db and version system
	//testCheckin.showRepoDB();
	//Version::showVersion();


    return 0;
}
#endif