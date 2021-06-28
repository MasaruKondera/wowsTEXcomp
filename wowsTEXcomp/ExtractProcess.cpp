#include "pch.h"
#include "ExtractProcess.h"

using namespace System;

void ExtractProcess::RecievedText(Object ^ source, Diagnostics::DataReceivedEventArgs ^ args)
{
	if (args->Data && args->Data->Length > 0)
	{
		Writer::WriteExtractOutput(args->Data);
		this->extracted->Add(args->Data);
	}
}

ExtractProcess::ExtractProcess(Parser^ parser)
{
	this->pCode = ReturnValue::Unpacker::WAITING_TO_START;
	this->parser = parser;
	ExtractProcess::process = gcnew Diagnostics::Process();
	ExtractProcess::process->StartInfo->FileName = "cmd";
	ExtractProcess::process->StartInfo->CreateNoWindow = true;
	ExtractProcess::process->StartInfo->UseShellExecute = false;
	ExtractProcess::process->StartInfo->RedirectStandardOutput = true;
	ExtractProcess::process->StartInfo->RedirectStandardError = true;
	ExtractProcess::process->EnableRaisingEvents = true;
	this->extracted = gcnew Collections::Generic::List<String^>();
	ExtractProcess::process->OutputDataReceived += gcnew Diagnostics::DataReceivedEventHandler(this, &ExtractProcess::RecievedText);
	ExtractProcess::process->ErrorDataReceived += gcnew System::Diagnostics::DataReceivedEventHandler(this, &ExtractProcess::OnErrorDataReceived);
	ExtractProcess::process->Exited += gcnew System::EventHandler(this, &ExtractProcess::OnExited);
}

bool ExtractProcess::Start(String^ path)
{
	String^ extension = IO::Path::GetExtension(path);
	Writer::WriteExtractWarning(path);
	if(extension->Length == 0)
		ExtractProcess::process->StartInfo->Arguments = "/C " + parser->unpackerExe + " " + parser->idx + " -p " + parser->pkg + " -l -x -I " + path + "/*.dds -o " + this->parser->unpackerTargetDirectory;
	else
		ExtractProcess::process->StartInfo->Arguments = "/C " + parser->unpackerExe + " " + parser->idx + " -p " + parser->pkg + " -l -x -I " + path + " -o " + this->parser->unpackerTargetDirectory;
	Console::ForegroundColor = ConsoleColor::DarkGreen;
	try
	{
		ExtractProcess::process->Start();
	}
	catch (ComponentModel::Win32Exception^ we)
	{
		Writer::Error("ExtractStartFail", we->Message);
		ExtractProcess::process = nullptr;
		return false;
	}
	
	this->pCode = ReturnValue::Unpacker::STARTED;
	ExtractProcess::process->BeginOutputReadLine();
	ExtractProcess::process->BeginErrorReadLine();
	return true;
}

bool ExtractProcess::Wait()
{
	try
	{
		ExtractProcess::process->WaitForExit();
	}
	catch (ComponentModel::Win32Exception^ we)
	{
		Writer::Error("ExtractWaitError", we->Message);
		return false;
	}	
	ExtractProcess::process = nullptr;
	return true;
}

Collections::Generic::List<String^>^ ExtractProcess::GetExtracted()
{
	return this->extracted;
}

void ExtractProcess::OnExited(Object^ source, EventArgs^ args)
{
	Writer::SetStandardColor();
	if (ExtractProcess::process->ExitCode != 0)
		this->pCode = ReturnValue::Unpacker::FAILURE;
	else
		this->pCode = ReturnValue::Unpacker::SUCCESS;
}

void ExtractProcess::OnErrorDataReceived(System::Object ^sender, System::Diagnostics::DataReceivedEventArgs ^args)
{
	Writer::WriteExtractError(args->Data);
}
