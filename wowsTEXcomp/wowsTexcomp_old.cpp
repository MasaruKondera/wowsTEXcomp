#include "pch.h"

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include <string>
#include <Windows.h>
#include <filesystem>
#include <processthreadsapi.h>
#include "DirectXTex.h"
#include <algorithm>
#include <dxgiformat.h>
#include <comdef.h>
#include <libloaderapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vcclr.h>

using namespace System;
using namespace std::filesystem;
using namespace System::Threading;

#define PROCESS_BUFFER_SIZE 4096

#define OVERWRITE 2
#define ABORT 1
#define SUCCESS 0

#define ARGUMENTS_MISSING -1
#define INCORRECT_SCALE_VALUE -2
#define UNKNOWN_PREFIX -3
#define WOWS_EXE_MISSING -4
#define UNPACKER_EXE_MISSING -5
#define IDX_DIR_ERROR -6
#define IDX_BIN_ERROR -7
#define PKG_MISSING -8
#define PATH_EMPTY -9
#define NOT_A_DIR -10
#define FALSE_ZIP_EXTENSION -11
#define FALSE_ZIP_PATH -12
#define NO_ZIP_PATH_DEFINED -13
#define BUFFER_ERROR -14
#define MODULE_PATH_ERROR -15
#define PIPE_CHILD_PARENT -16
#define PIPE_PARENT_CHILD -17
#define PIPE_CHILD_PARENT_HANDLE -18
#define PIPE_PARENT_CHILD_HANDLE -19
#define PROCESS_START_FAIL -20
#define PROCESS_FAILURE -21
#define FAILED_COM_INIT -22
#define IMAGE_LOAD_FAIL -23
#define DECOMP_FAIL -24
#define RESIZE_FAIL -25
#define MIP_MAP_FAIL -26
#define COMPRESSION_FAIL -27
#define SAVE_FAILED -28


//Parent Side
HANDLE g_hChildStd_IN_Wr = NULL; //Output!
HANDLE g_hChildStd_OUT_Rd = NULL; //Input!

//CHild
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

static void getStdString(std::string&, System::String^);

void println(std::string);
void printUsage();
void printWarning(std::string);
void printQuestion(std::string);
void printLastError();
void resultToError(HRESULT);


HRESULT parseArguments(array<System::String^>^, double&, bool&, bool&, std::string&, std::string&, std::string&, std::string&, std::string&);
HRESULT checkGamePath(std::string*, std::string*, std::string*, std::string*);
HRESULT checkZip(bool, std::string*, std::string* );
HRESULT checkFolderPath(std::string*, std::string);
HRESULT getIDX(std::string, std::string*);
HRESULT getPKG(std::string, std::string*);
HRESULT overwriteDeleteWarning(std::string);
HRESULT start_process(std::string, std::string);
HRESULT clearFolder(std::string);

HRESULT resizeCompressedImage(DirectX::ScratchImage*, DirectX::ScratchImage*, size_t, size_t);
HRESULT resizeImage(DirectX::ScratchImage*, DirectX::ScratchImage*, size_t, size_t);
HRESULT generateMipMap(DirectX::ScratchImage*, DirectX::ScratchImage*, DirectX::TEX_DIMENSION);
HRESULT compressImage(DirectX::ScratchImage*, DirectX::ScratchImage*, DXGI_FORMAT);
HRESULT saveImage(DirectX::ScratchImage*, std::wstring);
HRESULT saveCompressedImage(DirectX::ScratchImage*, DXGI_FORMAT, std::wstring, ID3D11Device*);
HRESULT imagePipe(ID3D11Device*, std::string, std::string, std::wstring, double);

HRESULT printError(HRESULT);

bool isExePath(std::string, std::string);

HANDLE hConsole;


int main(array<System::String ^> ^args)
{
	HRESULT result;

	//Semaphore^ pool = gcnew Semaphore(0, Environment::ProcessorCount);
	
	//ThreadStart^ tStart = gcnew ThreadStart();
	//Thread^ imageProcessing = gcnew Thread();

	
	ID3D11Device* pDevice;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL fLevels[6] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT, fLevels, 6, D3D11_SDK_VERSION, &pDevice, &featureLevel, NULL);

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	printWarning("This program may overwrite some files. The writer of this program takes no responsibility for any damage done by this software!");

	std::string game_location = "";
	std::string idx = "";
	std::string pkg = "";
	std::string unpacker = "";
	std::string unpacker_out_directory = "";
	std::string working_dir = "";
	std::string compressed_path = "";
	std::string zipName = "";
	
	bool extract = true;
	
	bool compress = false;
	double scale = 0.25;
	

	result = parseArguments(args, scale, compress, extract, game_location, unpacker_out_directory, working_dir, compressed_path, zipName);
	if (result != SUCCESS)
		return printError(result);
	
	result = checkGamePath(&game_location, &unpacker, &idx, &pkg);
	if (result != SUCCESS)
		return printError(result);

	result = checkZip(compress, &compressed_path, &zipName);
	if (result == ABORT)
		return result;
	else if (result != SUCCESS)
		return printError(result);
	
	result = checkFolderPath(&unpacker_out_directory, "tex_unpacker_output");
	if (result == OVERWRITE && extract)
	{
		if (overwriteDeleteWarning("All subdirectories and files in \"" + unpacker_out_directory + "\" will get deleted!") == ABORT)
			return ABORT;
	}
		
	result = checkFolderPath(&working_dir, "tex_working_dir");
	if (result == OVERWRITE)
	{
		if (overwriteDeleteWarning("All subdirectories and files in \"" + working_dir + "\" will get deleted!") == ABORT)
			return ABORT;
	}

	//ImageProcessing ^processing = gcnew ImageProcessing(Environment::ProcessorCount, pDevice, working_dir, unpacker_out_directory, scale);

	if (extract)
		clearFolder(unpacker_out_directory);
	clearFolder(working_dir);
	
	printWarning("Starting extracting files! This will take a while!");

	if (extract)
	{
		std::vector<std::string> dds_folders;
		std::vector<std::string> dds_files;

		dds_folders.push_back("content");
		dds_folders.push_back("dyndecals");
		dds_folders.push_back("gui/ui_render");
		dds_folders.push_back("helpers");
		dds_folders.push_back("maps");
		dds_folders.push_back("resources");
		dds_folders.push_back("spaces");
		dds_folders.push_back("system");

		dds_files.push_back("default_ao");

		for (std::string item : dds_folders)
		{
			std::cout << "Extrating Directory " + item + "from WoWs PKG System!" << std::endl;
			result = start_process("C:\\WINDOWS\\system32\\cmd.exe", "cmd /C \"" + unpacker + "\" -x " + idx + " -p " + pkg + " -I " + item + "/*.dds -o " + unpacker_out_directory);
		}

		// Extract files first
		for (std::string item : dds_files)
		{
			std::cout << "Extracting File " + item + "from WoWs PKG System!" << std::endl;
			result = start_process("C:\\WINDOWS\\system32\\cmd.exe", "cmd /C \"" + unpacker + "\" -x " + idx + " -p " + pkg + " -I " + item + ".dds -o " + unpacker_out_directory);
		}
	}
	
	//Get all extracted files
	std::vector<std::wstring> files;

	for (const auto &file : recursive_directory_iterator(unpacker_out_directory))
	{
		if (is_regular_file(file.path()))
			files.push_back(file.path().wstring());
	}

	// Resize dds files

	


	for (std::wstring file : files)
	{
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage srcImage;
		DirectX::ScratchImage resizedImage;
		DirectX::ScratchImage mipmapedImage;

		SetConsoleTextAttribute(hConsole, 10);
		std::wcout << L"Loading File: " << file << std::endl;
		result = DirectX::LoadFromDDSFile(file.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, srcImage);
		if (result != S_OK)
		{
			resultToError(HRESULT_FROM_WIN32(result));
			return IMAGE_LOAD_FAIL;
		}

		file.erase(0, unpacker_out_directory.size());
		file = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(working_dir) + file;

		size_t width = (size_t)round(metadata.width * scale);
		size_t height = (size_t)round(metadata.width * scale);
		if (width == 0)
			width = 1;
		if (height == 0)
			height = 1;

		if (DirectX::IsCompressed(metadata.format))
			result = resizeCompressedImage(&srcImage, &resizedImage, width, height);
		else
			result = resizeImage(&srcImage, &resizedImage, width, height);
		if (result != SUCCESS)
			return result;

		if (height > 1 && width > 1)
		{
			result = generateMipMap(&resizedImage, &mipmapedImage, metadata.dimension);
			if (result != SUCCESS)
				return result;

			if (DirectX::IsCompressed(metadata.format))
				result = saveCompressedImage(&mipmapedImage, metadata.format, file, pDevice);
			else
				result = saveImage(&mipmapedImage, file);
			if (result != SUCCESS)
				return result;

		}
		else
		{
			if (DirectX::IsCompressed(metadata.format))
				result = saveCompressedImage(&resizedImage, metadata.format, file, pDevice);
			else
				result = saveImage(&resizedImage, file);
			if (result != SUCCESS)
				return result;
		}
		
	}

	// If it is needed, compress the files into a Zip file

	return 0;
}

HRESULT parseArguments(array<System::String^> ^args, double &scale, bool &compress, bool &extract, std::string &game_location, std::string &unpacker_output_directory, std::string &working_dir, std::string &compressed_path, std::string &zipName)
{
	if (args->Length == 0)
		return SUCCESS;

	if (args->Length == 1)
	{
		std::string prefix;
		getStdString(prefix, args[0]);

		if (prefix.size() == 2 && prefix[0] == '-')
		{
			switch (prefix[1])
			{
			case 'h':
			case 'H':
			case '?':
				printUsage();
				break;
			case 'n':
			case 'N':
				extract = false;
				break;
			default:
				return UNKNOWN_PREFIX;
				break;
			}
			return SUCCESS;
		}
		else
			return UNKNOWN_PREFIX;
	}

	int argsParsed = 0;
	do
	{
		std::string input;
		getStdString(input, args[argsParsed]);
		if (input[0] == '-' && input.size() == 2)
		{
			argsParsed++;
			switch (input[1])
			{
			case 'n':
			case 'N':
				extract = false;
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
				if (argsParsed < args->Length)
				{
					std::string value;
					getStdString(value, args[argsParsed]);
					switch (input[1])
					{
					case 'o':
					case 'O':
						compressed_path = value;
						compress = true;
						break;
					case 'e':
					case 'E':
						unpacker_output_directory = value;
						break;
					case 'g':
					case 'G':
						game_location = value;
						break;
					case 's':
					case 'S':
						try
						{
							scale = stod(value);
						}
						catch (const std::invalid_argument &ia)
						{
							return INCORRECT_SCALE_VALUE;
						}
						break;
					case 'w':
					case 'W':
						working_dir = value;
						break;
					case 'z':
					case 'Z':
						zipName = value;
						break;
					default:
						return UNKNOWN_PREFIX;
					}
					argsParsed++;
				}
				else
					return ARGUMENTS_MISSING;
				break;
			default:
				return UNKNOWN_PREFIX;
			}
		}
		else
			return UNKNOWN_PREFIX;
	} while (argsParsed < args->Length);
	return SUCCESS;
}

void printUsage()
{
	println("This is a usage thingy!");
	return;
}

HRESULT clearFolder(std::string path)
{
	for (auto& path : directory_iterator(path))
	{
		remove_all(path);
	}
	return SUCCESS;
}

HRESULT checkFolderPath(std::string* folderPath, std::string standard)
{
	char modulePath[MAX_PATH];
	if (folderPath->empty())
	{
		int result = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
		HRESULT error = GetLastError();
		if (result > 0 && error == ERROR_INSUFFICIENT_BUFFER)
			return BUFFER_ERROR;
		if (!result)
			return MODULE_PATH_ERROR;
		std::string moduleStringPath(modulePath);
		*folderPath = path(moduleStringPath).parent_path().string() + "\\" + standard;
	}
	if (!exists(*folderPath))
		create_directories(*folderPath);
	else
	{
		if (is_directory(*folderPath))
			return OVERWRITE;
		else
		{
			return NOT_A_DIR;
		}
	}
	return SUCCESS;
}

HRESULT checkGamePath(std::string* game_location, std::string* unpacker, std::string* idx, std::string* pkg)
{
	//TODO: Add Error messages!
	if (game_location->empty())
		*game_location = "C:\\Games\\World_of_Warships";
	if (unpacker->empty())
		*unpacker = *game_location;
	
	if (!isExePath(*game_location, "WorldOfWarships"))
		return WOWS_EXE_MISSING;
	if (!isExePath(*unpacker, "wowsunpack"))
		return UNPACKER_EXE_MISSING;

	int result;
	result = getIDX(*game_location, idx);
	if (result != SUCCESS)
		return result;
	result = getPKG(*game_location, pkg);
	if (result != SUCCESS)
		return result;

	*unpacker = *game_location + "\\" + "wowsunpack.exe";

	return SUCCESS;
}

HRESULT checkZip(bool compress, std::string* compressPath, std::string* zipName)
{
	if (compress)
	{
		if (compressPath->empty())
			return PATH_EMPTY;
		
		if(!exists(*compressPath))
			create_directories(*compressPath);
		else
		{
			if (!is_directory(*compressPath))
				return NOT_A_DIR;
		}
		
		if (!zipName->empty())
		{
			path zip = path(*zipName);
			if (zip.parent_path().empty())
			{
				if (zip.has_extension() && zip.extension() != ".zip")
					return FALSE_ZIP_EXTENSION;
				else
					*zipName = *zipName + ".zip";
			}
			else
				return FALSE_ZIP_PATH;
		}
		else 
			*zipName = "wowsTexComp.zip";
		//PRINT WARNING FOR OVERWRITE HERE!
		if(exists(*compressPath + "\\" + *zipName))
			return overwriteDeleteWarning("Zip File already exists. Old file will get deleted!");
	}
	else
	{
		if (!zipName->empty())
			return NO_ZIP_PATH_DEFINED;
	}
	return SUCCESS;
}

HRESULT getIDX(std::string path, std::string* idx)
{
	std::string bin_path = path + "\\bin";
	if (exists(bin_path))
	{
		std::vector<std::string> folders;
		for (const auto &file : directory_iterator(bin_path))
		{
			if (file.is_directory())
				folders.push_back(file.path().filename().string());
		}
		if (folders.size() > 0)
		{
			int folder_int = 0;
			for (std::string folder : folders)
			{
				int conv = stoi(folder);
				if (conv > folder_int)
					folder_int = conv;
			}
			std::string idx_path = bin_path + "\\" + std::to_string(folder_int) + "\\idx";
			if (!exists(idx_path))
				return IDX_DIR_ERROR;
				
			*idx = idx_path;
			return SUCCESS;
		}
	}
	return IDX_BIN_ERROR;
}

HRESULT getPKG(std::string path, std::string* pkg)
{
	std::string package = path + "\\res_packages";
	if (!exists(package))
		return PKG_MISSING;
	*pkg = package;
	return SUCCESS;
}


void println(std::string text)
{
	Console::WriteLine(gcnew String(text.c_str()));
}

HRESULT printError(HRESULT errorCode)
{
	std::string errorText = "[ERROR] ";
	SetConsoleTextAttribute(hConsole, 12);
	switch (errorCode)
	{
	case ARGUMENTS_MISSING:
		println(errorText + "Can not parse given arguments. Maybe something is missing? Try -h for help!");
		break;
	case INCORRECT_SCALE_VALUE:
		println(errorText + "Could not parse given scale value. Please provide a decimal number.");
		break;
	case UNKNOWN_PREFIX:
		println(errorText + "Could not parse given prefix. Try -h for help!");
		break;
	case WOWS_EXE_MISSING:
		println(errorText + "Could not find World_of_Warships.exe!");
		break;
	case UNPACKER_EXE_MISSING:
		println(errorText + "Could not find wowsunpack.exe!");
		break;
	case IDX_BIN_ERROR:
		println(errorText + "Could not find the bin directory!");
		break;
	case IDX_DIR_ERROR:
		println(errorText + "Could not find the id directory!");
		break;
	case PKG_MISSING:
		println(errorText + "Could not find the directory res_packages!");
		break;
	case PATH_EMPTY:
		println(errorText + "Something went horrible wrong! You should not be able to be here!");
		break;
	case NOT_A_DIR:
		println(errorText + "A provided path does not point to a directory!");
		break;
	case FALSE_ZIP_EXTENSION:
		println(errorText + "ZIP filename contains an invalid file extension!");
		break;
	case FALSE_ZIP_PATH:
		println("Please provide a filename for the zip file and not a path!");
		break;
	case NO_ZIP_PATH_DEFINED:
		println("Please provide a location for your zip file via [-o]!");
		break;
	case BUFFER_ERROR:
		println(errorText + "Please move this program to a location whith less than " + std::to_string(MAX_PATH) + " characters!");
		break;
	case MODULE_PATH_ERROR:
		println(errorText + "Could not find the running directory of this program!");
		break;
	case PIPE_CHILD_PARENT:
		println(errorText + "Could not create pipe from child to parent!");
		printLastError();
		break;
	case PIPE_PARENT_CHILD:
		break;
	default:
		println(errorText + "Unknown error occured with error code " + std::to_string(errorCode));
		break;
	}
	SetConsoleTextAttribute(hConsole, 7);
	return errorCode;
}

HRESULT overwriteDeleteWarning(std::string text)
{
	std::string input;
	printWarning(text);
	while (1)
	{
		printQuestion("Continue? [y/n]");
		std::cin >> input;
		if (input == "y" || input == "Y")
			return SUCCESS;
		else if (input == "n" || input == "N")
			return ABORT;
	}
}

void printWarning(std::string text)
{
	SetConsoleTextAttribute(hConsole, 14);
	println("[WARNING] " + text);
	SetConsoleTextAttribute(hConsole, 7);
	return;
}

void printQuestion(std::string text)
{
	SetConsoleTextAttribute(hConsole, 11);
	println(text);
	SetConsoleTextAttribute(hConsole, 7);
	return;
}

static void getStdString(std::string &outStr, String ^str)
{
	IntPtr ansiStr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
	outStr = (const char*)ansiStr.ToPointer();
	System::Runtime::InteropServices::Marshal::FreeHGlobal(ansiStr);
}

bool isExePath(std::string path, std::string exe)
{
	std::string path_exe = path + "\\" + exe + ".exe";
	if (!exists(path_exe))
		return false;
	return true;
}

HRESULT start_process(std::string process_name, std::string process_arguments)
{
	STARTUPINFOA start_info;
	PROCESS_INFORMATION process_info;
	SECURITY_ATTRIBUTES security_attribute;

	ZeroMemory(&security_attribute, sizeof(security_attribute));
	security_attribute.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attribute.bInheritHandle = TRUE;
	security_attribute.lpSecurityDescriptor = NULL;

	//Setting up Pipe from Child to Parent
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &security_attribute, 0))
		return PIPE_CHILD_PARENT;

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return PIPE_CHILD_PARENT_HANDLE;

	//Setting up Pipe from Parent to Child
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &security_attribute, 0))
		return PIPE_PARENT_CHILD;

	if (!SetHandleInformation(g_hChildStd_IN_Rd, HANDLE_FLAG_INHERIT, 0))
		return PIPE_PARENT_CHILD_HANDLE;

	ZeroMemory(&start_info, sizeof(start_info));
	start_info.cb = sizeof(start_info);
	start_info.hStdError = g_hChildStd_OUT_Wr;
	start_info.hStdInput = g_hChildStd_IN_Rd;
	start_info.hStdOutput = g_hChildStd_OUT_Wr;
	start_info.dwFlags |= STARTF_USESTDHANDLES;

	ZeroMemory(&process_info, sizeof(process_info));


	std::vector<char>commandLineBuffer(process_arguments.begin(), process_arguments.end());
	if (!CreateProcessA(NULL, &process_arguments.front(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &start_info, &process_info))
		return PROCESS_START_FAIL;

	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);

	CloseHandle(g_hChildStd_OUT_Wr);
	g_hChildStd_OUT_Wr = 0;
	CloseHandle(g_hChildStd_IN_Rd);
	g_hChildStd_IN_Rd = 0;

	CHAR aBuf[PROCESS_BUFFER_SIZE + 1];
	ZeroMemory(&aBuf, PROCESS_BUFFER_SIZE + 1);
	DWORD dwRead;

	while (ReadFile(g_hChildStd_OUT_Rd, aBuf, PROCESS_BUFFER_SIZE, &dwRead, NULL)) {
		std::string str(aBuf);
		std::cout << str;
	}



	//ReadFile will either tell us that the pipe has closed, or give us an error
	DWORD le = GetLastError();

	//And finally cleanup
	CloseHandle(g_hChildStd_IN_Wr);
	g_hChildStd_IN_Wr = 0;
	CloseHandle(g_hChildStd_OUT_Rd);
	g_hChildStd_OUT_Rd = 0;


	if (le != ERROR_BROKEN_PIPE) //"The pipe has been ended."
	{
		resultToError(HRESULT_FROM_WIN32(le));
		return PROCESS_FAILURE;
	}
	return SUCCESS;
}

void printLastError()
{
	resultToError(HRESULT_FROM_WIN32(GetLastError()));
}

void resultToError(HRESULT errorCode)
{
	SetConsoleTextAttribute(hConsole, 12);
	_com_error error(errorCode);
	LPCTSTR text = error.ErrorMessage();
	std::wcout << std::endl << std::endl << std::endl << text;
	SetConsoleTextAttribute(hConsole, 7);
}


HRESULT resizeCompressedImage(DirectX::ScratchImage* srcImage, DirectX::ScratchImage* resizedImage, size_t width, size_t height)
{
	DirectX::ScratchImage decompImage;
	HRESULT result = DirectX::Decompress(srcImage->GetImages(), srcImage->GetImageCount(), srcImage->GetMetadata(), DXGI_FORMAT_UNKNOWN, decompImage);
	if (result != S_OK)
	{
		resultToError(result);
		return DECOMP_FAIL;
	}
	std::cout << "Decompressed...";
	return resizeImage(&decompImage, resizedImage, width, height);
}

HRESULT resizeImage(DirectX::ScratchImage* srcImage, DirectX::ScratchImage* resizedImage, size_t width, size_t height)
{
	
	HRESULT result = DirectX::Resize(srcImage->GetImages(), srcImage->GetImageCount(), srcImage->GetMetadata(), width, height, DirectX::TEX_FILTER_DEFAULT, *resizedImage);
	if (result != S_OK)
	{
		resultToError(result);
		return RESIZE_FAIL;
	}

	std::cout << "Resized...";
	
	return SUCCESS;
}

HRESULT generateMipMap(DirectX::ScratchImage* resizedImage, DirectX::ScratchImage* mipMapImage, DirectX::TEX_DIMENSION dimension)
{
	HRESULT result;
	if (dimension == DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE3D)
		result = DirectX::GenerateMipMaps3D(resizedImage->GetImages(), resizedImage->GetImageCount(), resizedImage->GetMetadata(), DirectX::TEX_FILTER_LINEAR, 0, *mipMapImage);
	else
		result = DirectX::GenerateMipMaps(resizedImage->GetImages(), resizedImage->GetImageCount(), resizedImage->GetMetadata(), DirectX::TEX_FILTER_LINEAR, 0, *mipMapImage);
	if (result != S_OK)
	{
		resultToError(result);
		return MIP_MAP_FAIL;
	}
	std::cout << "Mipmap Generated...";
	return SUCCESS;
}

HRESULT saveCompressedImage(DirectX::ScratchImage* mipmapedImage, DXGI_FORMAT format, std::wstring file, ID3D11Device* pDevice)
{
	DirectX::ScratchImage finalImage;
	HRESULT result = S_OK;
	if (format == DXGI_FORMAT_BC6H_UF16 || format == DXGI_FORMAT_BC6H_SF16 || format == DXGI_FORMAT_BC7_UNORM || format == DXGI_FORMAT_BC7_UNORM_SRGB)
	{
		result = DirectX::Compress(pDevice, mipmapedImage->GetImages(), mipmapedImage->GetImageCount(), mipmapedImage->GetMetadata(), format, DirectX::TEX_COMPRESS_DEFAULT, 0.5, finalImage);
	}
		
	else
		result = DirectX::Compress(mipmapedImage->GetImages(), mipmapedImage->GetImageCount(), mipmapedImage->GetMetadata(), format, DirectX::TEX_COMPRESS_DEFAULT, 0.5, finalImage);
	if (result != S_OK)
	{
		resultToError(result);
		return COMPRESSION_FAIL;
	}
	std::cout << "Image compressed...";
	
	saveImage(&finalImage, file);
	return SUCCESS;
}

HRESULT saveImage(DirectX::ScratchImage* imageToSave, std::wstring file)
{
	//save image here
	path parent = path(file).parent_path();
	if (!exists(parent))
	{
		create_directories(parent);
	}
	HRESULT result = DirectX::SaveToDDSFile(imageToSave->GetImages(), imageToSave->GetImageCount(), imageToSave->GetMetadata(), DirectX::DDS_FLAGS_NONE, file.c_str());
	if (result != S_OK)
	{
		resultToError(result);
		return SAVE_FAILED;
	}
	std::cout << "Image saved!" << std::endl;
	
	return SUCCESS;
}

HRESULT imagePipe(ID3D11Device* pDevice, std::string unpacker_out_directory, std::string working_dir, std::wstring file, double scale)
{
	HRESULT result;
	
	

	SetConsoleTextAttribute(hConsole, 10);
	std::wcout << L"Loading File: " << file << std::endl;
	result = DirectX::LoadFromDDSFile(file.c_str(), );
	if (result != S_OK)
	{
		resultToError(HRESULT_FROM_WIN32(result));
		return IMAGE_LOAD_FAIL;
	}

	file.erase(0, unpacker_out_directory.size());
	file = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(working_dir) + file;

	size_t width = (size_t)round(metadata.width * scale);
	size_t height = (size_t)round(metadata.width * scale);
	if (width == 0)
		width = 1;
	if (height == 0)
		height = 1;

	if (DirectX::IsCompressed(metadata.format))
		result = resizeCompressedImage(&srcImage, &resizedImage, width, height);
	else
		result = resizeImage(&srcImage, &resizedImage, width, height);
	if (result != SUCCESS)
		return result;

	if (height > 1 && width > 1)
	{
		result = generateMipMap(&resizedImage, &mipmapedImage, metadata.dimension);
		if (result != SUCCESS)
			return result;

		if (DirectX::IsCompressed(metadata.format))
			result = saveCompressedImage(&mipmapedImage, metadata.format, file, pDevice);
		else
			result = saveImage(&mipmapedImage, file);
		if (result != SUCCESS)
			return result;

	}
	else
	{
		if (DirectX::IsCompressed(metadata.format))
			result = saveCompressedImage(&resizedImage, metadata.format, file, pDevice);
		else
			result = saveImage(&resizedImage, file);
		if (result != SUCCESS)
			return result;
	}
	return SUCCESS;
}

