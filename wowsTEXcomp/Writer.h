#pragma once

#include "master.h"

//#define LANG_RU

ref class TextureProcessing;
ref class Parser;



public ref class Writer
{	
public:
	static void ProgramWarning();
	static ReturnValue::Console Ask(System::String^ question);

	static void Usage();

	static void InitError(System::String^ text, DWORD error);
	static void InitError(System::String^ text, HRESULT error);
	static void Error(System::String^ text, System::String^ system);
	static void Error(System::String^ text);
	static void ParseError(ReturnValue::Parser code);
	static void WriteExtractOutput(System::String^ text);
	static void WriteExtractWarning(System::String^ path);
	static void WriteExtractError(System::String^ text);
	static void WriteTexture(unsigned int counter, System::String^ file);
	static void WriteTextureError(ReturnValue::Texture code, System::String^ extraInfo);
	static void WarningFileDelete(System::String^ file);
	static void WarningFileOverwrite(System::String^ file);
	static void PrintColoredLocalizedText(System::String^ text, System::String^ format);
	static void PrintColoredLocalizedText(System::String^ text);
	static void WriteZip();
	static void FinishedZip();
	static void WriteDeleteWarning(System::String^ path);
	static void WriteZipperError(ReturnValue::Zipper code, System::String^ message);
	static void WriteDeleteError(ReturnValue::Delete code, System::String^ message);
	
	static void SetStandardColor();

	static System::Threading::Mutex^ colorMutex = gcnew System::Threading::Mutex();
	static initonly System::Resources::ResourceManager^ lang;

private:
	static System::String^ HRESULTtoMessage(HRESULT error);
	static Writer();
	static initonly System::ConsoleColor front;
	static initonly System::ConsoleColor back;
	static initonly System::Reflection::Assembly^ assembly;
	static System::ConsoleColor GetColor(wchar_t symbol);
	
};