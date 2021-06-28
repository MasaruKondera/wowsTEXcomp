#pragma once

#include "master.h"

ref class Waiter
{
public:
	Waiter();
	void ZipFiles(System::String^ path, System::String^ workingDir, System::String^ name);
	void DeleteDirectory(System::String^ path);
	ReturnValue::Zipper zipResult;
	ReturnValue::Delete deleteResult;
private:
	void Wait();
	void ZipIt();
	void DeleteIt();
	System::Collections::Generic::List<System::String^>^ spinner;
	bool wait;
	System::String^ path;
	System::String^ workingDir;
	System::String^ name;
	System::String^ message;

	static System::Threading::Mutex^ waitMutex = gcnew System::Threading::Mutex();
	bool isWaiting();

};

