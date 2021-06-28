#pragma once

#include "master.h"

ref class Writer;

public ref class Parser
{
public:
	System::String^ gameLocation; //
	System::String^ idx; //
	System::String^ pkg; //
	System::String^ unpackerExe; //
	System::String^ unpackerTargetDirectory;//
	System::String^ workingDir;//
	System::String^ zipDirPath;
	System::String^ zipName;
	
	double scale;
	bool zip;
	bool extract;

	Parser();

	ReturnValue::Parser ParseArgs(array<System::String ^> ^args);

private:
	bool GetStdString(std::string &output, System::String^ toConvert);
	ReturnValue::Parser Parser::CheckGamePath();	
};