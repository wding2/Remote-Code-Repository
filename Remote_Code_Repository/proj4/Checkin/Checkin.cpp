// Checkin.cpp : Defines the entry point for the console application.


#include "Checkin.h"
#include "../CppNoSqlDb/PayLoad/PayLoad.h"
#include "../CppCommWithFileXfer/Utilities/Utilities.h"


#ifdef TEST_CHECKIN
int main()
{  //create db instance
	DbCore<PayLoad> testDB_Repo;
	Checkin<PayLoad> testCheckin = Checkin<PayLoad>(testDB_Repo);
	testCheckin.showRepoDB();
	Version::showVersion();
	testCheckin.showRepoDB();
	Version::showVersion();
    return 0;
}
#endif
