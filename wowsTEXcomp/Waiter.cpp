#include "pch.h"
#include "Waiter.h"

using namespace System;

Waiter::Waiter()
{
	spinner = gcnew Collections::Generic::List<String^>();
	spinner->Add("[    ]");
	spinner->Add("[=   ]");
	spinner->Add("[==  ]");
	spinner->Add("[=== ]");
	spinner->Add("[ ===]");
	spinner->Add("[  ==]");
	spinner->Add("[   =]");
	spinner->Add("[    ]");
	spinner->Add("[   =]");
	spinner->Add("[  ==]");
	spinner->Add("[ ===]");
	spinner->Add("[=== ]");
	spinner->Add("[==  ]");
	spinner->Add("[=   ]");
	spinner->Add("[    ]");
}

void Waiter::ZipFiles(String^ path, String^ workingDir, String^ name)
{
	this->wait = true;
	this->path = path;
	this->name = name;
	this->workingDir = workingDir;
	this->message = "";
	this->zipResult = ReturnValue::Zipper::NO_RESULT;
	Writer::WriteZip();
	try
	{
		Threading::Thread^ thread = gcnew Threading::Thread(gcnew Threading::ThreadStart(this, &Waiter::ZipIt));
		thread->Start();
	}
	catch (Threading::ThreadStateException^ tse)
	{
		this->message = tse->Message;
		this->zipResult = ReturnValue::Zipper::THREAD_STATE;
		this->wait = false;
	}
	catch (OutOfMemoryException^ tm)
	{
		this->message = tm->Message;
		this->zipResult = ReturnValue::Zipper::THREAD_MEMORY;
		this->wait = false;
	}
	this->Wait();
}

void Waiter::DeleteDirectory(String^ path)
{
	this->wait = true;
	this->path = path;
	this->message = "";
	this->deleteResult = ReturnValue::Delete::NO_RESULT;
	Writer::WriteDeleteWarning(path);
	try
	{
		Threading::Thread^ thread = gcnew Threading::Thread(gcnew Threading::ThreadStart(this, &Waiter::DeleteIt));
		thread->Start();
	}
	catch (Threading::ThreadStateException^ tse)
	{
		this->message = tse->Message;
		this->deleteResult = ReturnValue::Delete::THREAD_STATE;
		this->wait = false;
	}
	catch (OutOfMemoryException^ tm)
	{
		this->message = tm->Message;
		this->deleteResult = ReturnValue::Delete::THREAD_MEMORY;
		this->wait = false;
	}
	this->Wait();
}

void Waiter::Wait()
{
	int left = 0;
	int top = Console::CursorTop;
	Console::ForegroundColor = ConsoleColor::Cyan;
	if (isWaiting())
	{
		Console::CursorVisible = false;
		int index = 0;
		while (isWaiting())
		{
			Console::SetCursorPosition(left, top);
			Console::Write(spinner[index]);
			index++;
			if (index == spinner->Count)
				index = 0;
			Sleep(100);
		}
		Console::CursorVisible = true;
	}
	Console::SetCursorPosition(left, top);
	Console::WriteLine("[====]");
	Console::ForegroundColor = ConsoleColor::White;
	if (this->deleteResult != ReturnValue::Delete::NO_RESULT && this->deleteResult != ReturnValue::Delete::SUCCESS)
			Writer::WriteDeleteError(this->deleteResult, this->message);
	else if (this->zipResult != ReturnValue::Zipper::NO_RESULT && this->zipResult != ReturnValue::Zipper::SUCCESS)
		Writer::WriteZipperError(this->zipResult, this->message);
}

void Waiter::ZipIt()
{
	
	try
	{
		if (IO::File::Exists(this->path + "\\" + this->name + ".zip"))
		{
			IO::File::Delete(this->path + "\\" + this->name + ".zip");
		}
		if (this->name->Length)
			IO::Compression::ZipFile::CreateFromDirectory(this->workingDir, this->path + "\\" + this->name + ".zip", IO::Compression::CompressionLevel::Optimal, false);
		else
		{
			std::time_t time = std::time(nullptr);
			IO::Compression::ZipFile::CreateFromDirectory(this->workingDir, this->path + "\\wows_tex_" + gcnew String(std::to_string(time).c_str()) + ".zip", IO::Compression::CompressionLevel::Optimal, false);
		}
	}
	catch (IO::PathTooLongException^ ptle)
	{
		this->message = ptle->Message;
		this->zipResult = ReturnValue::Zipper::PATH_TOO_LONG;
		this->waitMutex->WaitOne();
		this->wait = false;
		this->waitMutex->ReleaseMutex();
		return;
	}
	catch (IO::IOException^ io)
	{
		this->message = io->Message; 
		this->zipResult = ReturnValue::Zipper::IO_EXCEPTION;
		this->waitMutex->WaitOne();
		this->wait = false;
		this->waitMutex->ReleaseMutex();
		return;
	}
	catch (UnauthorizedAccessException^ uae)
	{
		this->message = uae->Message; 
		this->zipResult = ReturnValue::Zipper::NO_ACCESS;
		this->waitMutex->WaitOne();
		this->wait = false;
		this->waitMutex->ReleaseMutex();
		return;
	}
	this->zipResult = ReturnValue::Zipper::SUCCESS;
	this->waitMutex->WaitOne();
	this->wait = false;
	this->waitMutex->ReleaseMutex();
}

void Waiter::DeleteIt()
{
	try
	{
		IO::DirectoryInfo^ dir = gcnew IO::DirectoryInfo(this->path);
		for each(IO::FileInfo^ info in dir->GetFiles())
			info->Delete();
		for each(IO::DirectoryInfo^ info in dir->GetDirectories())
			info->Delete(true);
	}
	catch (IO::IOException^ io)
	{
		this->message = io->Message;
		this->deleteResult = ReturnValue::Delete::IO_EXCEPTION;
		this->waitMutex->WaitOne();
		this->wait = false;
		this->waitMutex->ReleaseMutex();
		return;
	}
	catch (UnauthorizedAccessException^ uae)
	{
		this->message = uae->Message;
		this->deleteResult = ReturnValue::Delete::NO_ACCESS;
		this->waitMutex->WaitOne();
		this->wait = false;
		this->waitMutex->ReleaseMutex();
		return;
	}
	this->deleteResult = ReturnValue::Delete::SUCCESS;
	this->waitMutex->WaitOne();
	this->wait = false;
	this->waitMutex->ReleaseMutex();
}

bool Waiter::isWaiting()
{
	Waiter::waitMutex->WaitOne();
	if (this->wait)
	{
		Waiter::waitMutex->ReleaseMutex();
		return true;
	}
	Waiter::waitMutex->ReleaseMutex();
	return false;

}