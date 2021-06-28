#pragma once

#include "master.h"

ref class Parser;
ref class ChildProcessTracker;

public ref class ExtractProcess
{
private:
	void RecievedText(System::Object^ source, System::Diagnostics::DataReceivedEventArgs^ args);
	void OnExited(System::Object^ source, System::EventArgs^ args);
	Parser^ parser;
	System::Collections::Generic::List<System::String^>^ extracted;
	void OnErrorDataReceived(System::Object ^sender, System::Diagnostics::DataReceivedEventArgs ^args);


public:
	ExtractProcess(Parser^ parser);
	bool Start(System::String^ path);
	bool Wait();
	System::Collections::Generic::List<System::String^>^ GetExtracted();
	ReturnValue::Unpacker pCode;
	static System::Diagnostics::Process^ process = nullptr;
};

