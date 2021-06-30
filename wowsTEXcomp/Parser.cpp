#include "pch.h"
#include "Parser.h"

using namespace System;
using namespace System::IO;

Parser::Parser()
{
	this->gameLocation = L"C:\\Games\\World_of_Warships";
	this->idx = L"";;
	this->pkg = L"";
	this->unpackerExe = L"C:\\Games\\World_of_Warships";
	this->unpackerTargetDirectory = L"";
	this->workingDir = L"";
	this->zipDirPath = L"";
	this->zipName = L"";
	this->scale = 0.25;
	this->zip = false;
	this->extract = true;
}

ReturnValue::Parser Parser::ParseArgs(array<String ^> ^args)
{
	std::string input;
	char modulePath[MAX_PATH];
	DWORD result = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
	HRESULT error = GetLastError();
	if (result > 0 && error == ERROR_INSUFFICIENT_BUFFER)
	{
		Writer::ParseError(ReturnValue::Parser::THIS_EXE_PATH_TOO_LONG);
		return ReturnValue::Parser::THIS_EXE_PATH_TOO_LONG;
	}
	if (!result)
	{
		Writer::ParseError(ReturnValue::Parser::THIS_EXE_PATH_ERROR);
		return ReturnValue::Parser::THIS_EXE_PATH_ERROR;
	}
	String^ thisExeDir = gcnew String(modulePath);
	try
	{
		thisExeDir = Path::GetDirectoryName(thisExeDir);
	}
	catch (PathTooLongException^)
	{
		Writer::ParseError(ReturnValue::Parser::THIS_EXE_PATH_ERROR);
		return ReturnValue::Parser::THIS_EXE_PATH_ERROR;
	}
	unpackerTargetDirectory = thisExeDir;
	this->unpackerTargetDirectory += L"\\tex_unpacker_output";
	if (this->unpackerTargetDirectory->Length > MAX_PATH)
	{
		Writer::ParseError(ReturnValue::Parser::UNPACKER_TARGET_TOO_LONG);
		return ReturnValue::Parser::UNPACKER_TARGET_TOO_LONG;
	}		
	this->workingDir = thisExeDir;
	this->workingDir += L"tex_working_dir";
	if (this->workingDir->Length > MAX_PATH)
	{
		Writer::ParseError(ReturnValue::Parser::WORKING_DIR_TOO_LONG);
		return ReturnValue::Parser::WORKING_DIR_TOO_LONG;
	}
	if (args->Length == 0)
		return ReturnValue::Parser::SUCCESS;
	 
	if (args->Length == 1)
	{
		if (args[0]->Length == 2 && args[0][0] == '-')
		{
			switch (args[0][1])
			{
			case 'h':
			case 'H':
			case '?':
				Writer::Usage();
				return ReturnValue::Parser::PRINT_USAGE;
			case 'n':
			case 'N':
				this->extract = false;
			default:
				Writer::ParseError(ReturnValue::Parser::PREFIX_INCORRECT);
				return ReturnValue::Parser::PREFIX_INCORRECT;
			}
		}
		else
		{
			Writer::ParseError(ReturnValue::Parser::PREFIX_INCORRECT);
			return ReturnValue::Parser::PREFIX_INCORRECT;
		}
	}
	else
	{
		int argsParsed = 0;
		wchar_t comp;
		do
		{
			if (args[argsParsed][0] == '-' && args[argsParsed]->Length == 2)
			{
				switch (args[argsParsed][1])
				{
				case 'h':
				case 'H':
					Writer::ParseError(ReturnValue::Parser::HELP_NOT_ALONE);
					return ReturnValue::Parser::HELP_NOT_ALONE;
				case 'n':
				case 'N':
					extract = false;
					argsParsed++;
					break;
				case 'o':
				case 'O':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
				case 's':
				case 'S':
				case 'w':
				case 'W':
				case 'z':
				case 'Z':
					comp = args[argsParsed][1];
					argsParsed++;
					if (argsParsed < args->Length)
					{
						switch (comp)
						{
						case 'o':
						case 'O':
							this->zipDirPath = args[argsParsed];
							this->zip = true;
							if (this->zipDirPath->EndsWith("\\"))
							    this->zipDirPath->Remove(this->zipDirPath->Length - 1);			
							break;
						case 'e':
						case 'E':
							this->unpackerTargetDirectory = args[argsParsed];
							if (this->unpackerTargetDirectory->EndsWith("\\"))
								this->unpackerTargetDirectory->Remove(this->unpackerTargetDirectory->Length - 1);
							break;
						case 'g':
						case 'G':
							this->gameLocation = args[argsParsed];
							if (this->gameLocation->EndsWith("\\"))
								this->gameLocation->Remove(this->gameLocation->Length - 1);
							break;
						case 's':
						case 'S':
							try
							{
								scale = Double::Parse(args[argsParsed], System::Globalization::CultureInfo::InvariantCulture);
							}
							catch (FormatException^)
							{
								Writer::ParseError(ReturnValue::Parser::FORMAT_EXCEPTION);
								return ReturnValue::Parser::FORMAT_EXCEPTION;
							}
							break;
						case 'w':
						case 'W':
							this->workingDir = args[argsParsed];
							if (this->workingDir->EndsWith("\\"))
								this->workingDir->Remove(this->workingDir->Length - 1);
							break;
						case 'z':
						case 'Z':
							this->zipName = args[argsParsed];
							break;
						default:
							Writer::ParseError(ReturnValue::Parser::PREFIX_INCORRECT);
							return ReturnValue::Parser::PREFIX_INCORRECT;
						}
						argsParsed++;
					}
					else
					{
						Writer::ParseError(ReturnValue::Parser::MISSING_ARGUMENT);
						return ReturnValue::Parser::MISSING_ARGUMENT;
					}
					break;
				default:
					Writer::ParseError(ReturnValue::Parser::PREFIX_INCORRECT);
					return ReturnValue::Parser::PREFIX_INCORRECT;
				}
			}
			else
			{
				Writer::ParseError(ReturnValue::Parser::PREFIX_INCORRECT);
				return ReturnValue::Parser::PREFIX_INCORRECT;
			}
		} while (argsParsed < args->Length);
	}

	if (!(this->scale < 1.00))
	{
		Writer::ParseError(ReturnValue::Parser::SCALE_TOO_BIG);
		return ReturnValue::Parser::SCALE_TOO_BIG;
	}

	ReturnValue::Parser parseCode = this->CheckGamePath();
	if (parseCode != ReturnValue::Parser::SUCCESS)
		return parseCode;

	Writer::ProgramWarning();
	if (Writer::Ask("ProgramUsage") == ReturnValue::Console::ABORT)
	{
		return ReturnValue::Parser::NO_USAGE;
	}

	if (this->zip)
	{
		if (this->zipDirPath->Length && this->zipDirPath->IndexOfAny(Path::GetInvalidPathChars()) >= 0)
		{
			return ReturnValue::Parser::ZIP_INVALID_PATH;
		}
				
		if (!this->GetStdString(input, this->zipDirPath))
		{
			Writer::ParseError(ReturnValue::Parser::STRING_CONVERSION_ERROR);
			return ReturnValue::Parser::STRING_CONVERSION_ERROR;
		}
		if (!std::filesystem::path(input).is_absolute())
		{
			Writer::ParseError(ReturnValue::Parser::ZIP_NOT_ABSOLUTE);
			return ReturnValue::Parser::ZIP_NOT_ABSOLUTE;
		}
		
		

		if (!Directory::Exists(this->zipDirPath))
		{
			if (!File::Exists(this->zipDirPath))
			{
				try
				{
					Directory::CreateDirectory(this->zipDirPath);
				}
				catch (ArgumentException^)
				{
					Writer::ParseError(ReturnValue::Parser::ZIP_ARGUMENT_EXCEPTION);
					return ReturnValue::Parser::ZIP_ARGUMENT_EXCEPTION;
				}
				catch (PathTooLongException^)
				{
					Writer::ParseError(ReturnValue::Parser::ZIP_PATH_TOO_LONG);
					return ReturnValue::Parser::ZIP_PATH_TOO_LONG;
				}
				catch (DirectoryNotFoundException^)
				{
					Writer::ParseError(ReturnValue::Parser::ZIP_PATH_NOT_FOUND);
					return ReturnValue::Parser::ZIP_PATH_NOT_FOUND;
				}
				catch (UnauthorizedAccessException^)
				{
					Writer::ParseError(ReturnValue::Parser::ZIP_ACCESS_FAILURE);
					return ReturnValue::Parser::ZIP_ACCESS_FAILURE;
				}	
			}
			else
			{
				Writer::ParseError(ReturnValue::Parser::ZIP_PATH_IS_FILE);
				return ReturnValue::Parser::ZIP_PATH_IS_FILE;
			}
		}				
		
		if (this->zipName->Length)
		{
			this->zipName = Path::GetFileNameWithoutExtension(zipName);
			if (this->zipName->IndexOfAny(Path::GetInvalidFileNameChars()) >= 0)
				return ReturnValue::Parser::ZIP_INVALID_NAME;
			if (File::Exists(this->zipDirPath + "\\" + this->zipName + ".zip"))
			{
				Writer::WarningFileOverwrite(this->zipDirPath + "\\" + this->zipName + ".zip");
				if (Writer::Ask("Continue") == ReturnValue::Console::ABORT)
					return ReturnValue::Parser::ZIP_PROTECTED;
			}
		}
	}
	else
	{
		if (this->zipName->Length)
		{
			Writer::ParseError(ReturnValue::Parser::ZIP_PATH_EMPTY);
			return ReturnValue::Parser::ZIP_PATH_EMPTY;
		}
	}

	if (!this->unpackerTargetDirectory->Length)
	{
		Writer::ParseError(ReturnValue::Parser::UNPACK_PATH_EMPTY);
		return ReturnValue::Parser::UNPACK_PATH_EMPTY;
	}

	if (this->unpackerTargetDirectory->IndexOfAny(Path::GetInvalidPathChars()) >= 0)
	{
		return ReturnValue::Parser::UNPACK_INVALID_PATH;
	}

	if (!this->GetStdString(input, this->unpackerTargetDirectory))
	{
		Writer::ParseError(ReturnValue::Parser::STRING_CONVERSION_ERROR);
		return ReturnValue::Parser::STRING_CONVERSION_ERROR;
	}
	if (!std::filesystem::path(input).is_absolute())
	{
		Writer::ParseError(ReturnValue::Parser::UNPACK_NOT_ABSOLUTE);
		return ReturnValue::Parser::UNPACK_NOT_ABSOLUTE;
	}
	if (!Directory::Exists(this->unpackerTargetDirectory))
	{
		if (!File::Exists(this->unpackerTargetDirectory))
		{
			try
			{
				Directory::CreateDirectory(this->unpackerTargetDirectory);
			}
			catch (ArgumentException^)
			{
				Writer::ParseError(ReturnValue::Parser::UNPACK_ARGUMENT_EXCEPTION);
				return ReturnValue::Parser::UNPACK_ARGUMENT_EXCEPTION;
			}
			catch (PathTooLongException^)
			{
				Writer::ParseError(ReturnValue::Parser::UNPACK_PATH_TOO_LONG);
				return ReturnValue::Parser::UNPACK_PATH_TOO_LONG;
			}
			catch (DirectoryNotFoundException^)
			{
				Writer::ParseError(ReturnValue::Parser::UNPACK_PATH_NOT_FOUND);
				return ReturnValue::Parser::UNPACK_PATH_NOT_FOUND;
			}
			catch (UnauthorizedAccessException^)
			{
				Writer::ParseError(ReturnValue::Parser::UNPACK_ACCESS_FAILURE);
				return ReturnValue::Parser::UNPACK_ACCESS_FAILURE;
			}
		}
		else
		{
			Writer::ParseError(ReturnValue::Parser::UNPACK_PATH_IS_FILE);
			return ReturnValue::Parser::UNPACK_PATH_IS_FILE;
		}
	}

	if (this->extract)
	{
		Writer::WarningFileDelete(this->unpackerTargetDirectory);
		if (Writer::Ask("Continue") == ReturnValue::Console::ABORT)
			return ReturnValue::Parser::UNPACK_PROTECTED;
	}

	if (!this->workingDir->Length)
	{
		Writer::ParseError(ReturnValue::Parser::WORK_DIR_PATH_EMPTY);
		return ReturnValue::Parser::WORK_DIR_PATH_EMPTY;
	}

	if (this->workingDir->IndexOfAny(Path::GetInvalidPathChars()) >= 0)
	{
		return ReturnValue::Parser::WORK_DIR_INVALID_PATH;
	}

	if (!this->GetStdString(input, this->workingDir))
	{
		Writer::ParseError(ReturnValue::Parser::STRING_CONVERSION_ERROR);
		return ReturnValue::Parser::STRING_CONVERSION_ERROR;
	}
	if (!std::filesystem::path(input).is_absolute())
	{
		Writer::ParseError(ReturnValue::Parser::WORK_DIR_NOT_ABSOLUTE);
		return ReturnValue::Parser::WORK_DIR_NOT_ABSOLUTE;
	}
	if (!Directory::Exists(this->workingDir))
	{
		if (!File::Exists(this->workingDir))
		{
			try
			{
				Directory::CreateDirectory(this->workingDir);
			}
			catch (ArgumentException^)
			{
				Writer::ParseError(ReturnValue::Parser::WORK_DIR_ARGUMENT_EXCEPTION);
				return ReturnValue::Parser::WORK_DIR_ARGUMENT_EXCEPTION;
			}
			catch (PathTooLongException^)
			{
				Writer::ParseError(ReturnValue::Parser::WORK_DIR_PATH_TOO_LONG);
				return ReturnValue::Parser::WORK_DIR_PATH_TOO_LONG;
			}
			catch (DirectoryNotFoundException^)
			{
				Writer::ParseError(ReturnValue::Parser::WORK_DIR_PATH_NOT_FOUND);
				return ReturnValue::Parser::WORK_DIR_PATH_NOT_FOUND;
			}
			catch (UnauthorizedAccessException^)
			{
				Writer::ParseError(ReturnValue::Parser::WORK_DIR_ACCESS_FAILURE);
				return ReturnValue::Parser::WORK_DIR_ACCESS_FAILURE;
			}
		}
		else
		{
			Writer::ParseError(ReturnValue::Parser::WORK_DIR_PATH_IS_FILE);
			return ReturnValue::Parser::WORK_DIR_PATH_IS_FILE;
		}
	}
	else
	{
		Writer::WarningFileDelete(this->workingDir);
		if (Writer::Ask("Continue") == ReturnValue::Console::ABORT)
			return ReturnValue::Parser::WORK_DIR_PROTECTED;
	}
	return ReturnValue::Parser::SUCCESS;
}

bool Parser::GetStdString(std::string &output, String^ toConvert)
{
	try
	{
		IntPtr ansiStr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(toConvert);
		output = (const char*)ansiStr.ToPointer();
		Runtime::InteropServices::Marshal::FreeHGlobal(ansiStr);
	}
	catch (OutOfMemoryException^)
	{
		return false;
	}
	catch (ArgumentOutOfRangeException^)
	{
		return false;
	}
	return true;
}

ReturnValue::Parser Parser::CheckGamePath()
{
	if (gameLocation->Length > 0)
	{
		if (this->gameLocation->IndexOfAny(Path::GetInvalidPathChars()) >= 0)
		{
			return ReturnValue::Parser::GAME_LOCATION_INVALID_PATH;
		}
		std::string input;
		if (!this->GetStdString(input, this->gameLocation))
		{
			Writer::ParseError(ReturnValue::Parser::STRING_CONVERSION_ERROR);
			return ReturnValue::Parser::STRING_CONVERSION_ERROR;
		}
		if (!std::filesystem::path(input).is_absolute())
		{
			Writer::ParseError(ReturnValue::Parser::GAME_LOCATION_NOT_ABSOLUTE);
			return ReturnValue::Parser::GAME_LOCATION_NOT_ABSOLUTE;
		}
		if (!File::Exists(this->gameLocation + L"\\WorldofWarships.exe"))
		{
			Writer::ParseError(ReturnValue::Parser::GAME_EXE_NOT_FOUND);
			return ReturnValue::Parser::GAME_EXE_NOT_FOUND;
		}
		this->unpackerExe = this->gameLocation + L"\\wowsunpack.exe";

		if (!this->GetStdString(input, this->unpackerExe))
		{
			Writer::ParseError(ReturnValue::Parser::STRING_CONVERSION_ERROR);
			return ReturnValue::Parser::STRING_CONVERSION_ERROR;
		}
		if (!std::filesystem::path(input).is_absolute())
		{
			Writer::ParseError(ReturnValue::Parser::UNPACK_NOT_ABSOLUTE);
			return ReturnValue::Parser::UNPACK_NOT_ABSOLUTE;
		}
		if (!File::Exists(this->unpackerExe))
		{
			Writer::ParseError(ReturnValue::Parser::UNPACKER_NOT_FOUND);
			return ReturnValue::Parser::UNPACKER_NOT_FOUND;
		}
				
		
		if (!Directory::Exists(this->gameLocation + L"\\bin"))
		{
			Writer::ParseError(ReturnValue::Parser::BIN_NOT_FOUND);
			return ReturnValue::Parser::BIN_NOT_FOUND;
		}
			
		int dirInt = 0;
		String^ tempBinDir = L"";
		try
		{
			for each (String^ dirName in Directory::EnumerateDirectories(this->gameLocation + L"\\bin"))
			{
				try
				{
					int temp = int::Parse(Path::GetFileName(dirName));
					if (temp > dirInt)
					{
						dirInt = temp;
						tempBinDir = Path::GetFileName(dirName);
					}
				}
				catch (FormatException^)
				{
					Writer::ParseError(ReturnValue::Parser::BIN_FORMAT);
				}
				catch (OverflowException^)
				{
					Writer::ParseError(ReturnValue::Parser::BIN_OVERFLOW);
					return ReturnValue::Parser::BIN_OVERFLOW;
				}
			}
		}
		catch (PathTooLongException^)
		{
			Writer::ParseError(ReturnValue::Parser::BIN_PATH_TOO_LONG);
			return ReturnValue::Parser::BIN_PATH_TOO_LONG;
		}
		catch (UnauthorizedAccessException^)
		{
			Writer::ParseError(ReturnValue::Parser::BIN_NO_ACCESS);
			return ReturnValue::Parser::BIN_NO_ACCESS;
		}
		
		if (!dirInt)
		{
			Writer::ParseError(ReturnValue::Parser::BIN_EMPTY);
			return ReturnValue::Parser::BIN_EMPTY;
		}

		this->idx = this->gameLocation + L"\\bin\\" + tempBinDir + "\\idx";
		if (!Directory::Exists(this->idx))
		{
			Writer::ParseError(ReturnValue::Parser::IDX_MISSING);
			return ReturnValue::Parser::IDX_MISSING;
		}
		this->pkg = this->gameLocation + "\\res_packages";
		if (!Directory::Exists(this->pkg))
		{
			Writer::ParseError(ReturnValue::Parser::MISSING_PKG);
			return ReturnValue::Parser::MISSING_PKG;
		}
		return ReturnValue::Parser::SUCCESS;
	}
	else
		return ReturnValue::Parser::EMPTY_GAME_LOCATION;
}
