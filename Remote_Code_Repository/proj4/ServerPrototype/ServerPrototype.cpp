/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 1.2                                                             //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018           //
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Process/Process/Process.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

//----< return name of every file on path >----------------------------

Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}
//----< return name of every subdirectory on path >--------------------

Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}


ServerProc Server::wrappedLambda_Checkin() {
	return  [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.attribute("checkinFile", msg.value("checkinFile"));
		reply.command("reply_checkin");
		//reply.show();
		string filName = "./SaveFiles/" + msg.value("checkinFile");
		string userName = msg.value("userName");
		string dependencyFiles_Str = msg.value("dependencyFiles");
		string descrip = msg.value("descrip");
		string cate_Str = msg.value("category");
		vector<string> dependencyFiles_Vec;
		string delimiter = "/";
		size_t pos = 0;
		string token;
		while ((pos = dependencyFiles_Str.find(delimiter)) != std::string::npos) {
			token = dependencyFiles_Str.substr(0, pos);
			dependencyFiles_Vec.push_back("./SaveFiles/" + token);
			//std::cout << token << std::endl;
			dependencyFiles_Str.erase(0, pos + delimiter.length());
		}
		vector<string> cate_Vec;
		pos = 0;
		while ((pos = cate_Str.find(delimiter)) != std::string::npos) {
			token = cate_Str.substr(0, pos);
			cate_Vec.push_back(token);
			//std::cout << token << std::endl;
			cate_Str.erase(0, pos + delimiter.length());
		}

		this->repoCore->RepoCheckin(filName, descrip, userName, cate_Vec, dependencyFiles_Vec);

		//let client to run next test
		if (msg.containsKey("demo")) {
			reply.attribute("demo", "next_testCheckout");
		}
		this->repoCore->showRepoDB();
		return reply;
	};
}


ServerProc Server::wrappedLambda_closeCheckin() {
	return  [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.attribute("fileName", msg.value("fileName"));
		reply.command("reply_closeCheckin");
		
		string temp = this->repoCore->closeCheckin(msg.value("userName"),msg.value("fileName"));
		reply.attribute("closeResult", temp);
		this->repoCore->showRepoDB();
		return reply;
	};
}

ServerProc Server::wrappedLambda_Checkout() {
	return  [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		//reply.attribute("fileName", msg.value("fileName"));
		reply.command("reply_checkoutSending");
		//reply.show();
		set<string> names = this->repoCore->RepoCheckout(msg.value("fileName"));
		for (auto item : names) {
			reply.attribute("sendingFile", item);
			reply.show();
			this->postMessage(reply);
		}
		reply.remove("sendingFile");
		reply.attribute("fileName", msg.value("fileName"));
		reply.command("reply_checkoutFinished");
		return reply;
	};
}

ServerProc Server::wrappedLambda_displayCate() {
	return  [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		//reply.attribute("fileName", msg.value("fileName"));
		reply.command("reply_displayCate");
		//reply.show();
		vector<string> names = this->repoCore->displayCate(msg.value("fileName"));
		string temp;
		for (auto item : names) {
			temp += item + "/";		
		}
		reply.attribute("fileName", temp);		
		return reply;
	};
}

ServerProc  Server::wrappedLambda_query() {
	return  [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		//reply.attribute("fileName", msg.value("fileName"));
		reply.command("reply_query");
		//reply.show();
		Query<PayLoad> q(db_);
		Conditions<PayLoad> conds;
		conds.categories(msg.value("category"));
		conds.name(msg.value("fileName"));
		conds.version(msg.value("version"));
		conds.children(msg.value("dependency"));
		q.select(conds).show();
		set<string> names = q.select(conds).getResult();
		//set<string> names = this->repoCore->getQueryResult(msg.value("fileName"), msg.value("version"), msg.value("category"), msg.value("dependency"));
		string temp;
		for (auto item : names) {
			temp += item + "/";
		}
		reply.attribute("fileName", temp);
		return reply;
	};




}






namespace MsgPassingCommunication
{
	// These paths, global to MsgPassingCommunication, are needed by 
	// several of the ServerProcs, below.
	// - should make them const and make copies for ServerProc usage

	std::string sendFilePath;
	std::string saveFilePath;


	//----< show message contents >--------------------------------------

	template<typename T>
	void show(const T& t, const std::string& msg)
	{
		std::cout << "\n  " << msg.c_str();
		for (auto item : t)
		{
			std::cout << "\n    " << item.c_str();
		}
	}
	//----< test ServerProc simply echos message back to sender >--------

	std::function<Msg(Msg)> echo = [](Msg msg) {
		Msg reply = msg;
		reply.to(msg.from());
		reply.from(msg.to());
		return reply;
	};
	//----< getFiles ServerProc returns list of files on path >----------

	std::function<Msg(Msg)> getFiles = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getFiles");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != ".")
				searchPath = searchPath + "\\" + path;
			Files files = Server::getFiles(searchPath);
			size_t count = 0;
			for (auto item : files)
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("file" + countStr, item);
			}
		}
		else
		{
			std::cout << "\n  getFiles message did not define a path attribute";
		}
		return reply;
	};
	//----< getDirs ServerProc returns list of directories on path >-----

	std::function<Msg(Msg)> getDirs = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getDirs");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != ".")
				searchPath = searchPath + "\\" + path;
			Files dirs = Server::getDirs(searchPath);
			size_t count = 0;
			for (auto item : dirs)
			{
				if (item != ".." && item != ".")
				{
					std::string countStr = Utilities::Converter<size_t>::toString(++count);
					reply.attribute("dir" + countStr, item);
				}
			}
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};

	//----< sendFile ServerProc sends file to requester >----------------
	/*
	*  - Comm sends bodies of messages with sendingFile attribute >------
	*/
	std::function<Msg(Msg)> sendFile = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("sendFile");
		reply.attribute("sendingFile", msg.value("fileName"));
		reply.attribute("fileName", msg.value("fileName"));
		reply.attribute("verbose", "blah blah");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath))
			{
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			std::string filePath = searchPath + "/" + msg.value("fileName");
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = sendFilePath;
			if (!FileSystem::Directory::exists(fullDstPath))
			{
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += "/" + msg.value("fileName");
			FileSystem::File::copy(fullSrcPath, fullDstPath);
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};

	//----< analyze code on current server path >--------------------------
	/*
	*  - Creates process to run CodeAnalyzer on specified path
	*  - Won't return until analysis is done and logfile.txt
	*    is copied to sendFiles directory
	*/
	std::function<Msg(Msg)> codeAnalyze = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("sendFile");
		reply.attribute("sendingFile", "logfile.txt");
		reply.attribute("fileName", "logfile.txt");
		reply.attribute("verbose", "blah blah");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath))
			{
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			// run Analyzer using Process class

			Process p;
			p.title("test application");
			//std::string appPath = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe";
			std::string appPath = "CodeAnalyzer.exe";
			p.application(appPath);

			//std::string cmdLine = "c:/su/temp/project4Sample/debug/CodeAnalyzer.exe ";
			std::string cmdLine = "CodeAnalyzer.exe ";
			cmdLine += searchPath + " ";
			cmdLine += "*.h *.cpp /m /r /f";
			//std::string cmdLine = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe ../Storage/path *.h *.cpp /m /r /f";
			p.commandLine(cmdLine);

			std::cout << "\n  starting process: \"" << appPath << "\"";
			std::cout << "\n  with this cmdlne: \"" << cmdLine << "\"";

			CBP callback = []() { std::cout << "\n  --- child process exited ---"; };
			p.setCallBackProcessing(callback);

			if (!p.create())
			{
				std::cout << "\n  can't start process";
			}
			p.registerCallback();

			std::string filePath = searchPath + "\\" + /*msg.value("codeAnalysis")*/ "logfile.txt";
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = sendFilePath;
			if (!FileSystem::Directory::exists(fullDstPath))
			{
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += std::string("\\") + /*msg.value("codeAnalysis")*/ "logfile.txt";
			FileSystem::File::copy(fullSrcPath, fullDstPath);
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};


	//reply create subfolder msg coming from client
	std::function<Msg(Msg)> descripFolderReply = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("reply_descrip");
		//let client to run next test
		if (msg.containsKey("demo")) {
			reply.attribute("demo", "next_testCheckin");
		}
		std::string tempFolderName = msg.value("descripFolder");
		std::string tempFolderPath = storageRoot + "/" + tempFolderName;
		std::cout << "\n try to create dir, path is " + tempFolderPath << std::endl;
		FileSystem::Directory::create(tempFolderPath);
		if (tempFolderName != "") {
			if (FileSystem::Directory::exists(tempFolderPath)) {
				std::cout << "\n create dir successfully" << std::endl;
				reply.attribute("createFolder", "true");
				reply.attribute("descripFolder", tempFolderName);
				reply.attribute("descripFolderPath", tempFolderPath);
			}
			else {
				reply.attribute("createFolder", "false");
				reply.attribute("descripFolderPath", storageRoot);
				std::cout << "\n try to create dir, fails" << std::endl;
			}
		}
		else {
			std::cout << "\n try to create dir, result is root" << std::endl;
			reply.attribute("createFolder", "root");
		}
		return reply;
	};


	//reply check in msg coming from client



	//std::function<Msg(Msg)> checkinReply = [&](Msg msg) {
	   // Msg reply;
	   // reply.to(msg.from());
	   // reply.from(msg.to());
	   // reply.attribute("checkinFile", msg.value("checkinFile"));
	   // reply.command("reply_checkin");
	   // string filName = "./SaveFiles/" + msg.value("checkinFile");
	   // string userName = msg.value("userName");
	   // string dependencyFiles_Str = msg.value("dependencyFiles");
	   // vector<string> dependencyFiles_Vec;
	   // string delimiter = ",";
	   // size_t pos = 0;
	   // string token;
	   // while ((pos = dependencyFiles_Str.find(delimiter)) != std::string::npos) {
		  //  token = dependencyFiles_Str.substr(0, pos);
		  //  dependencyFiles_Vec.push_back(token);
		  //  //std::cout << token << std::endl;
		  //  dependencyFiles_Str.erase(0, pos + delimiter.length());
	   // }
	   // //for(msg.value("dependencyFiles"))
	   // string descrip = msg.value("descrip");

	   // //repoCore->RepoCheckin(filName, dependencyFiles_Vec, descrip, userName);
	   //// this->testLambda = "123";
	   // 
	   // 
	   // //let client to run next test
	   // if (msg.containsKey("demo")) {
		  //  reply.attribute("demo", "next_testCheckout");
	   // }
	   // return reply;
	//};


	std::function<Msg(Msg)> preCheckingReply = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("reply_preChecking");
		reply.attribute("fileName", msg.value("sendingFile"));
		return reply;
	};
}


using namespace MsgPassingCommunication;

int main()
{
	SetConsoleTitleA("Project4Sample Server Console");

	std::cout << "\n  Testing Server Prototype";
	std::cout << "\n ==========================";
	std::cout << "\n";

	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();

	sendFilePath = FileSystem::Directory::createOnPath("./SendFiles");
	saveFilePath = FileSystem::Directory::createOnPath("./SaveFiles");

	Server server(serverEndPoint, "ServerPrototype");

	// may decide to remove Context
	MsgPassingCommunication::Context* pCtx = server.getContext();
	pCtx->saveFilePath = saveFilePath;
	pCtx->sendFilePath = sendFilePath;

	server.start();

	std::cout << "\n  testing getFiles and getDirs methods";
	std::cout << "\n --------------------------------------";
	Files files = server.getFiles();
	show(files, "Files:");
	Dirs dirs = server.getDirs();
	show(dirs, "Dirs:");
	std::cout << "\n";

	std::cout << "\n  testing message processing";
	std::cout << "\n ----------------------------";
	server.addMsgProc("echo", echo);
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("sendFile", sendFile);
	server.addMsgProc("codeAnalyze", codeAnalyze);
	server.addMsgProc("descripFolder", descripFolderReply);
	server.addMsgProc("preCheckingin", preCheckingReply);

	server.addMsgProc("checkin", server.wrappedLambda_Checkin());
	server.addMsgProc("closeCheckin", server.wrappedLambda_closeCheckin());
	server.addMsgProc("checkout", server.wrappedLambda_Checkout());
	server.addMsgProc("displayCate", server.wrappedLambda_displayCate());
	server.addMsgProc("query", server.wrappedLambda_query());

	server.addMsgProc("serverQuit", echo);

	server.processMessages();

	Msg msg(serverEndPoint, serverEndPoint);  // send to self
	msg.name("msgToSelf");

	/////////////////////////////////////////////////////////////////////
	// Additional tests here, used during development
	//
	//msg.command("echo");
	//msg.attribute("verbose", "show me");
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//msg.command("getFiles");
	//msg.remove("verbose");
	//msg.attributes()["path"] = storageRoot;
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//msg.command("getDirs");
	//msg.attributes()["path"] = storageRoot;
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	std::cout << "\n  press enter to exit\n";
	std::cin.get();
	std::cout << "\n";

	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

