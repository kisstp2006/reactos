
#include <iostream>
#include <fstream>
#include <string>

#include "devcpp.h"

using namespace std;

static class DevCppFactory : public Backend::Factory
{
	public:

		DevCppFactory() : Factory("devcpp") {}
		Backend *operator() (Project &project)
		{
			return new DevCppBackend(project);
		}
		
} factory;


DevCppBackend::DevCppBackend(Project &project) : Backend(project)
{
	m_unitCount = 0;
}

void DevCppBackend::Process()
{
	string filename = ProjectNode.name + ".dev";
	
	cout << "Creating Dev-C++ project: " << filename << endl;

	ProcessModules();

	m_devFile.open(filename.c_str());

	if(!m_devFile.is_open())
	{
		cout << "Could not open file." << endl;
		return;
	}

	m_devFile << "[Project]" << endl;
	
	m_devFile	<< "FileName="				<< filename 		<< endl
				<< "Name="					<< ProjectNode.name	<< endl
				<< "UnitCount="				<< m_unitCount		<< endl
				<< "Type=1"					<< endl
				<< "Ver=1"					<< endl
				<< "ObjFiles="				<< endl
				<< "Includes="				<< endl
				<< "Libs="					<< endl
				<< "PrivateResource="		<< endl
				<< "ResourceIncludes="		<< endl
				<< "MakeIncludes="			<< endl
				<< "Compiler="				<< endl
				<< "CppCompiler="			<< endl
				<< "Linker="				<< endl
				<< "IsCpp=1"				<< endl
				<< "Icon="					<< endl
				<< "ExeOutput="				<< endl
				<< "ObjectOutput="			<< endl
				<< "OverrideOutput=0"		<< endl
				<< "OverrideOutputName="	<< endl
				<< "HostApplication="		<< endl
				<< "CommandLine="			<< endl
				<< "UseCustomMakefile=1"	<< endl
				<< "CustomMakefile="		<< ProjectNode.makefile << endl
				<< "IncludeVersionInto=0"	<< endl
				<< "SupportXPThemes=0"		<< endl
				<< "CompilerSet=0"			<< endl
				
				<< "CompilerSettings=0000000000000000000000" << endl;
	
	OutputFolders();

	m_devFile << endl << endl;

	OutputFileUnits();

	m_devFile.close();
	
	// Dev-C++ needs a makefile, so use the MinGW backend to create one.
	
	cout << "Creating Makefile: " << ProjectNode.makefile << endl;
	
	Backend *backend = Backend::Factory::Create("mingw", ProjectNode);
	backend->Process();
	delete backend;

	cout << "Done." << endl << endl;

	cout	<< "You may want to disable Class browsing (see below) before you open this project in Dev-C++, as the "
			<< "parsing required for large projects can take quite awhile."
			<< endl << endl
			<< "(Tools->Editor Options->Class browsing->Enable class browsing check box)"
			<< endl << endl;
}

void DevCppBackend::ProcessModules()
{
	for(size_t i = 0; i < ProjectNode.modules.size(); i++)
	{
		Module &module = *ProjectNode.modules[i];

		for(size_t k = 0; k < module.files.size(); k++)
		{
			File &file = *module.files[k];
			
			ProcessFile(file.name);
		}
	}
}

void DevCppBackend::ProcessFile(string &filepath)
{
	// Remove the .\ at the start of the filenames
	filepath.erase(0, 2);

	// Change the \ to /
	for(size_t i = 0; i < filepath.length(); i++)
	{
		if(filepath[i] == '\\')
			filepath[i] = '/';
	}

	// Remove the filename from the path
	string folder = "";

	size_t pos = filepath.rfind(string("/"), filepath.length() - 1);

	if(pos != string::npos)
	{
		folder = filepath;
		folder.erase(pos, folder.length() - pos);
	}
	
	FileUnit fileUnit;
	fileUnit.filename = filepath;
	fileUnit.folder = folder;

	m_fileUnits.push_back(fileUnit);

	if(folder != "")
		AddFolders(folder);

	m_unitCount++;
}

bool DevCppBackend::CheckFolderAdded(string &folder)
{
	for(size_t i = 0; i < m_folders.size(); i++)
	{
		if(m_folders[i] == folder)
			return true;
	}

	return false;
}

void DevCppBackend::AddFolders(string &folder)
{
	// Check if this folder was already added. true if it was, false otherwise.
	if(CheckFolderAdded(folder))
		return;
	
	m_folders.push_back(folder);
	
	size_t pos = folder.rfind(string("/"), folder.length() - 1);

	if(pos == string::npos)
		return;

	folder.erase(pos, folder.length() - pos);
	AddFolders(folder);
}

void DevCppBackend::OutputFolders()
{
	m_devFile << "Folders=";

	for(size_t i = 0; i < m_folders.size(); i++)
	{
		if(i > 0)
			m_devFile << ",";

		m_devFile << m_folders[i];
	}
}

void DevCppBackend::OutputFileUnits()
{
	for(size_t i = 0; i < m_fileUnits.size(); i++)
	{
		m_devFile << "[Unit" << i + 1 << "]" << endl;
		
		m_devFile << "FileName="			<< m_fileUnits[i].filename << endl;
		m_devFile << "CompileCpp=1" 		<< endl;
		m_devFile << "Folder=" 				<< m_fileUnits[i].folder << endl;
		m_devFile << "Compile=1"			<< endl;
		m_devFile << "Link=1" 				<< endl;
		m_devFile << "Priority=1000"		<< endl;
		m_devFile << "OverrideBuildCmd=0"	<< endl;
		m_devFile << "BuildCmd="			<< endl << endl;;
	}
}

