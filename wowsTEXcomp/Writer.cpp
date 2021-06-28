#include "pch.h"
#include "Writer.h"

using namespace System;

void Writer::ProgramWarning()
{
	PrintColoredLocalizedText(Writer::lang->GetString("ProgramWarning"));
}

ReturnValue::Console Writer::Ask(String^ question)
{
	String^ input;
	while (1)
	{
		Console::ForegroundColor = ConsoleColor::Cyan;
		Console::WriteLine(Writer::lang->GetString(question));
		Console::ForegroundColor = ConsoleColor::Yellow;
		Console::WriteLine(L"[Y/N]");
		Console::ForegroundColor = ConsoleColor::White;
		input = Console::ReadLine();
		if (input == "y" || input == "Y")
			return ReturnValue::Console::CONTINUE;
		else if (input == "n" || input == "N")
			return ReturnValue::Console::ABORT;
		else
		{
			Console::ForegroundColor = ConsoleColor::Yellow;
			Console::WriteLine(Writer::lang->GetString("QuestionHint"));
		}
	}
}

void Writer::Usage()
{
	Console::WriteLine(Writer::lang->GetString("UsageLine0"));
	Console::WriteLine("");
	Console::WriteLine("");
	Console::WriteLine(Writer::lang->GetString("UsageLine1"));
	Console::WriteLine("");
	Console::WriteLine("");
	Console::WriteLine(Writer::lang->GetString("UsageLine2"));
	Console::WriteLine(Writer::lang->GetString("UsageLine3"));
	Console::WriteLine(Writer::lang->GetString("UsageLine4"));
	Console::WriteLine(Writer::lang->GetString("UsageLine5"));
	Console::WriteLine(Writer::lang->GetString("UsageLine6"));
	Console::WriteLine(Writer::lang->GetString("UsageLine7"));
	Console::WriteLine(Writer::lang->GetString("UsageLine8"));
	Console::WriteLine(Writer::lang->GetString("UsageLine9"));
	Console::WriteLine(Writer::lang->GetString("UsageLine10"));
	Console::WriteLine(Writer::lang->GetString("UsageLine11"));
	Console::WriteLine("");
	Console::WriteLine("");
	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::WriteLine(Writer::lang->GetString("UsageLine12"));
	Console::WriteLine(Writer::lang->GetString("UsageLine13"));
	Console::WriteLine(Writer::lang->GetString("UsageLine14"));
	Console::WriteLine(Writer::lang->GetString("UsageLine15"));
	Console::ForegroundColor = Writer::front;
}


void Writer::InitError(System::String^ text, DWORD error)
{
	Writer::InitError(text, HRESULT_FROM_WIN32(error));
}

void Writer::InitError(System::String^ text, HRESULT error)
{

	System::String^ message = Writer::HRESULTtoMessage(error);
	Console::ForegroundColor = ConsoleColor::Red;
	Console::WriteLine(Writer::lang->GetString("InitError") + Writer::lang->GetString(text));
	if (message->Length)
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
	Console::ForegroundColor = Writer::front;
}

void Writer::Error(System::String^ text, System::String^ system)
{
	Console::ForegroundColor = ConsoleColor::Red;
	Console::WriteLine(Writer::lang->GetString("Error") + text);
	Console::WriteLine(Writer::lang->GetString("SystemMessage") + system);
	Console::ForegroundColor = Writer::front;
}

void Writer::Error(System::String^ text)
{
	Console::ForegroundColor = ConsoleColor::Red;
	Console::WriteLine(Writer::lang->GetString("Error") + text);
	Console::ForegroundColor = Writer::front;
}

void Writer::ParseError(ReturnValue::Parser code)
{
	Console::ForegroundColor = ConsoleColor::Red;
	switch (code)
	{
	case ReturnValue::Parser::SUCCESS:
		break;
	case ReturnValue::Parser::PRINT_USAGE:
		break;
	case ReturnValue::Parser::NO_USAGE:
		break;
	case ReturnValue::Parser::THIS_EXE_PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThisExePathTooLong"));
		break;
	case ReturnValue::Parser::THIS_EXE_PATH_ERROR:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThisExePathError"));
		break;
	case ReturnValue::Parser::UNPACKER_TARGET_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackerTargetTooLong"));
		break;
	case ReturnValue::Parser::WORKING_DIR_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirTooLong"));
		break;
	case ReturnValue::Parser::STRING_CONVERSION_ERROR:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("StringConversionError"));
		break;
	case ReturnValue::Parser::PREFIX_INCORRECT:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("PrefixIncorrect"));
		break;
	case ReturnValue::Parser::FORMAT_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("FormatException"));
		break;
	case ReturnValue::Parser::HELP_NOT_ALONE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("HelpNotAlone"));
		break;
	case ReturnValue::Parser::MISSING_ARGUMENT:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("MissingArgument"));
		break;
	case ReturnValue::Parser::GAME_EXE_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("GameExeNotFound"));
		break;
	case ReturnValue::Parser::UNPACKER_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackerNotFound"));
		break;
	case ReturnValue::Parser::BIN_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BinNotFound"));
		break;
	case ReturnValue::Parser::BIN_FORMAT:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BinFormat"));
		break;
	case ReturnValue::Parser::BIN_OVERFLOW:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BinOverflow"));
		break;
	case ReturnValue::Parser::BIN_PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BinPathTooLong"));
		break;
	case ReturnValue::Parser::BIN_NO_ACCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BinNoAccess"));
		break;
	case ReturnValue::Parser::IDX_MISSING:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("IDXMissing"));
		break;
	case ReturnValue::Parser::BIN_EMPTY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("BINEmpty"));
		break;
	case ReturnValue::Parser::MISSING_PKG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("MissingPKG"));
		break;
	case ReturnValue::Parser::EMPTY_GAME_LOCATION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("EmptyGameLocation"));
		break;
	case ReturnValue::Parser::GAME_LOCATION_NOT_ABSOLUTE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("GameLocationNotAbsolute"));
		break;
	case ReturnValue::Parser::SCALE_TOO_BIG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ScaleTooBig"));
		break;
	case ReturnValue::Parser::ZIP_NOT_ABSOLUTE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipNotAbsolute"));
		break;
	case ReturnValue::Parser::ZIP_ARGUMENT_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipArgumentException"));
		break;
	case ReturnValue::Parser::ZIP_PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipPathTooLong"));
		break;
	case ReturnValue::Parser::ZIP_PATH_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipPathNotFound"));
		break;
	case ReturnValue::Parser::ZIP_ACCESS_FAILURE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipAccessFailure"));
		break;
	case ReturnValue::Parser::ZIP_PATH_IS_FILE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipPathIsFile"));
		break;
	case ReturnValue::Parser::ZIP_PROTECTED:
		break;
	case ReturnValue::Parser::ZIP_PATH_EMPTY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipPathEmpty"));
		break;
	case ReturnValue::Parser::UNPACK_NOT_ABSOLUTE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackNotAbsolute"));
		break;
	case ReturnValue::Parser::UNPACK_ARGUMENT_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackArgumentException"));
		break;
	case ReturnValue::Parser::UNPACK_PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackPathTooLong"));
		break;
	case ReturnValue::Parser::UNPACK_PATH_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackPathNotFound"));
		break;
	case ReturnValue::Parser::UNPACK_ACCESS_FAILURE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackAccessFailure"));
		break;
	case ReturnValue::Parser::UNPACK_PATH_IS_FILE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackPathIsFile"));
		break;
	case ReturnValue::Parser::UNPACK_PROTECTED:
		break;
	case ReturnValue::Parser::UNPACK_PATH_EMPTY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackPathEmpty"));
		break;
	case ReturnValue::Parser::WORK_DIR_NOT_ABSOLUTE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirNotAbsolute"));
		break;
	case ReturnValue::Parser::WORK_DIR_ARGUMENT_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirArgumentException"));
		break;
	case ReturnValue::Parser::WORK_DIR_PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirPathTooLong"));
		break;
	case ReturnValue::Parser::WORK_DIR_PATH_NOT_FOUND:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirPathNotFound"));
		break;
	case ReturnValue::Parser::WORK_DIR_ACCESS_FAILURE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirAccessFailure"));
		break;
	case ReturnValue::Parser::WORK_DIR_PATH_IS_FILE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirPathIsFile"));
		break;
	case ReturnValue::Parser::WORK_DIR_PROTECTED:
		break;
	case ReturnValue::Parser::WORK_DIR_PATH_EMPTY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirPathEmpty"));
		break;
	case ReturnValue::Parser::ZIP_INVALID_NAME:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipInvalidName"));
		break;
	case ReturnValue::Parser::ZIP_INVALID_PATH:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipInvalidPath"));
		break;
	case ReturnValue::Parser::UNPACK_INVALID_PATH:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnpackInvalidPath"));
		break;
	case ReturnValue::Parser::WORK_DIR_INVALID_PATH:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("WorkDirInvalidPath"));
		break;
	case ReturnValue::Parser::GAME_LOCATION_INVALID_PATH:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("GameLocationInvalidPath"));
		break;
	default:
		break;
	}
	Console::ForegroundColor = Writer::front;
}

void Writer::WriteExtractOutput(String^ text)
{
	Console::WriteLine(Writer::lang->GetString("Extracting") + text);
}

void Writer::WriteExtractWarning(String^ path)
{
	Writer::PrintColoredLocalizedText(Writer::lang->GetString("ExtractingWarning"), path);
}

void Writer::WriteExtractError(String ^ text)
{
	Console::ForegroundColor = ConsoleColor::Red;
	Console::Write(text);
	Console::ForegroundColor = ConsoleColor::DarkGreen;
}

void Writer::SetStandardColor()
{
	Console::ForegroundColor = Writer::front;
}


static Writer::Writer() 
{
	Console::ForegroundColor = ConsoleColor::White;
	Console::BackgroundColor = ConsoleColor::Black;
	Writer::front = Console::ForegroundColor;
	Writer::back = Console::BackgroundColor;
	Writer::assembly	= Reflection::Assembly::GetExecutingAssembly();
#ifdef LANG_RU
	String^ resourceName = assembly->GetName()->Name + ".second";
	Resources::ResourceManager^ lang = gcnew Resources::ResourceManager(resourceName, assembly);
#else
	String^ resourceName = assembly->GetName()->Name + ".main";
	Writer::lang = gcnew Resources::ResourceManager(resourceName, assembly);
#endif // LANG_RU
}



System::String^ Writer::HRESULTtoMessage(HRESULT error)
{
	_com_error comError(error);
	LPCTSTR text = comError.ErrorMessage();
	return gcnew System::String(text);
}


void Writer::WriteTextureError(ReturnValue::Texture code, String^ extraInfo)
{
	Writer::colorMutex->WaitOne();
	Console::ForegroundColor = ConsoleColor::Red;
	switch (code)
	{
	case ReturnValue::Texture::SUCCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("NotHere") + "\"" + extraInfo + "\"");
		break;
	case ReturnValue::Texture::LOAD_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("LoadFail"), extraInfo));
		break;
	case ReturnValue::Texture::DECOMPRESS_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("DecompressFail"), extraInfo));
		break;
	case ReturnValue::Texture::RESIZE_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("ResizeFail"), extraInfo));
		break;
	case ReturnValue::Texture::MIPMAP_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("MipmapFail"), extraInfo));
		break;
	case ReturnValue::Texture::COMPRESS_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("CompressFail"), extraInfo));
		break;
	case ReturnValue::Texture::SAVE_FAIL:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("SaveFail"), extraInfo));
		break;
	case ReturnValue::Texture::ABORT:
		Console::WriteLine(Writer::lang->GetString("Error") + String::Format(Writer::lang->GetString("Abort"), extraInfo));
		break;
	case ReturnValue::Texture::THREAD_STATE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadState"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + extraInfo);
		break;
	case ReturnValue::Texture::THREAD_MEMORY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadMemory") + extraInfo);
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + extraInfo);
		break;
	case ReturnValue::Texture::THREAD_INTERRUPTED:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadInterrupted") + extraInfo);
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + extraInfo);
		break;
	default:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnknownError"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + extraInfo);
		break;
	}
	Console::ForegroundColor = ConsoleColor::Green;
	Writer::colorMutex->ReleaseMutex();
}

void Writer::WriteTexture(unsigned int counter, String ^ file)
{
	Writer::colorMutex->WaitOne();
	Console::WriteLine(Writer::lang->GetString("Finished") + "[#" + counter.ToString() + "] \"" + file + "\"");
	Writer::colorMutex->ReleaseMutex();
}

void Writer::WarningFileDelete(System::String^ file)
{
	Writer::PrintColoredLocalizedText(Writer::lang->GetString("FileDirDelete"), file);
}

void Writer::WarningFileOverwrite(System::String ^ file)
{
	Writer::PrintColoredLocalizedText(Writer::lang->GetString("FileDirOverwrite"), file);
}

void Writer::PrintColoredLocalizedText(System::String^ text, System::String^ format)
{
	Collections::Generic::List<Tuple<String^, ConsoleColor>^>^ list = gcnew Collections::Generic::List<Tuple<String^, ConsoleColor>^>();
	ConsoleColor color;
	int start = 3;
	int end = 0;
	color = Writer::GetColor(text[1]);
	for (int i = 3; i < text->Length; i++)
	{
		if (text[i] == '<' && (i + 2) < text->Length && text[i + 2] == '>')
		{
			end = i - start;
			Tuple<String^, ConsoleColor>^ pair = gcnew Tuple<String^, ConsoleColor>(text->Substring(start, end), color);
			list->Add(pair);
			color = Writer::GetColor(text[i + 1]);
			start = i + 3;
		}
	}
	end = text->Length - start;
	Tuple<String^, ConsoleColor>^ pair = gcnew Tuple<String^, ConsoleColor>(text->Substring(start, end), color);
	list->Add(pair);
	for each(Tuple<String^, ConsoleColor>^ pair in list)
	{
		Console::ForegroundColor = pair->Item2;
		Console::Write(String::Format(pair->Item1, format));
	}
	Console::ForegroundColor = Writer::front;
	Console::WriteLine("");
}

void Writer::PrintColoredLocalizedText(System::String^ text)
{
	Collections::Generic::List<Tuple<String^, ConsoleColor>^>^ list = gcnew Collections::Generic::List<Tuple<String^, ConsoleColor>^>();
	ConsoleColor color;
	int start = 3;
	int end = 0;
	color = Writer::GetColor(text[1]);
	for (int i = 3; i < text->Length; i++)
	{
		if (text[i] == '<' && (i + 2) < text->Length && text[i + 2] == '>')
		{
			end = i - start;
			Tuple<String^, ConsoleColor>^ pair = gcnew Tuple<String^, ConsoleColor>(text->Substring(start, end), color);
			list->Add(pair);
			color = Writer::GetColor(text[i + 1]);
			start = i + 3;
		}
	}
	end = text->Length - start;
	Tuple<String^, ConsoleColor>^ pair = gcnew Tuple<String^, ConsoleColor>(text->Substring(start, end), color);
	list->Add(pair);
	for each(Tuple<String^, ConsoleColor>^ pair in list)
	{
		Console::ForegroundColor = pair->Item2;
		Console::Write(pair->Item1);
	}
	Console::ForegroundColor = Writer::front;
	Console::WriteLine("");
}

ConsoleColor Writer::GetColor(wchar_t symbol)
{
	switch (symbol)
	{
	case 'y':
		return ConsoleColor::Yellow;
	case 'r':
		return ConsoleColor::Red;
	case 'm':
		return ConsoleColor::Magenta;
	case 'Y':
		return ConsoleColor::DarkYellow;
	default:
		return ConsoleColor::White;
	}
}

void Writer::WriteZip()
{
	Console::ForegroundColor = ConsoleColor::Yellow;
	Console::WriteLine(Writer::lang->GetString("Warning") + Writer::lang->GetString("ZipWarning"));
	Console::ForegroundColor = Writer::front;
}

void Writer::WriteDeleteWarning(String^ path)
{
	Writer::PrintColoredLocalizedText(Writer::lang->GetString("DeleteWarning"), path);
}

void Writer::FinishedZip()
{
	Console::ForegroundColor = ConsoleColor::Green;
	Console::WriteLine(Writer::lang->GetString("ZipFinished"));
	Console::ForegroundColor = Writer::front;
}

void Writer::WriteZipperError(ReturnValue::Zipper code, String^ message)
{
	switch (code)
	{
	case ReturnValue::Zipper::SUCCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("NotHere"));
		break;
	case ReturnValue::Zipper::THREAD_STATE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadState"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Zipper::THREAD_MEMORY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadMemory"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Zipper::PATH_TOO_LONG:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipNamePathTooLong"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Zipper::IO_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipIOExe"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Zipper::NO_ACCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ZipAccessFailure"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Zipper::NO_RESULT:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("NotHere"));
		break;
	default:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnknownError"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	}
}

void Writer::WriteDeleteError(ReturnValue::Delete code, String^ message)
{
	switch (code)
	{
	case ReturnValue::Delete::SUCCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("NotHere"));
		break;
	case ReturnValue::Delete::THREAD_STATE:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadState"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Delete::THREAD_MEMORY:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("ThreadMemory"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Delete::IO_EXCEPTION:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("DeleteIOExe"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Delete::NO_ACCESS:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("DeleteNoAccess"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	case ReturnValue::Delete::NO_RESULT:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("NotHere"));
		break;
	default:
		Console::WriteLine(Writer::lang->GetString("Error") + Writer::lang->GetString("UnknownError"));
		Console::WriteLine(Writer::lang->GetString("SystemMessage") + message);
		break;
	}
}
