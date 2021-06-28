#include "pch.h"
#include "master.h"

using namespace System;
using namespace Collections::Generic;
using namespace System::Management;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	if (ExtractProcess::process != nullptr)
	{
		int cmdID = ExtractProcess::process->Id;
		ManagementObjectSearcher^ objectSearcher = gcnew ManagementObjectSearcher(L"SELECT * FROM Win32_Process WHERE ParentProcessId=" + cmdID.ToString());
		for each(ManagementObject^ obj in objectSearcher->Get())
		{
			try
			{
				System::Diagnostics::Process^ child = System::Diagnostics::Process::GetProcessById(safe_cast<unsigned int>(obj->Properties["ProcessId"]->Value));
				child->Kill();
			}
			catch (ArgumentException^ ae)
			{
				Writer::Error("ChildProcessKillError", ae->Message);
			}
			catch (ComponentModel::Win32Exception^ win)
			{
				Writer::Error("ChildProcessKillError", win->Message);
			}
		}
		try
		{
			ExtractProcess::process->Kill();
		}
		catch (ComponentModel::Win32Exception^ win)
		{
			Writer::Error("ChildProcessKillError", win->Message);
		}
	}
	return FALSE;
}

int main(array<String ^> ^args)
{	
	bool ctrl = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!ctrl)
	{
		Writer::InitError("SetCtrlHandler", GetLastError());
		return -1;
	}
	
	//Initialize D3D11 Device for GPU Tex Compression
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
	HRESULT device = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT, fLevels, 6, D3D11_SDK_VERSION, &pDevice, &featureLevel, NULL);
	if (device != S_OK)
	{
		Writer::InitError("GPUDevice", device);
		return -1;
	}
	
	
	Parser^ parser = gcnew Parser();

	ReturnValue::Parser result = parser->ParseArgs(args);
	if (result == ReturnValue::Parser::PRINT_USAGE)
		return 0;
	if (result == ReturnValue::Parser::NO_USAGE || result == ReturnValue::Parser::UNPACK_PROTECTED || result == ReturnValue::Parser::WORK_DIR_PROTECTED)
	{
		return 1;
	}
	if (result != ReturnValue::Parser::SUCCESS)
	{
		Writer::ParseError(result);
		Writer::Usage();
		return -1;
	}
	
	Waiter^ waiter = gcnew Waiter();

	if (parser->extract)
	{
		waiter->DeleteDirectory(parser->unpackerTargetDirectory);
		if (waiter->deleteResult != ReturnValue::Delete::SUCCESS)
			return -1;
	}
	waiter->DeleteDirectory(parser->workingDir);
	if (waiter->deleteResult != ReturnValue::Delete::SUCCESS)
		return -1;
	
	TextureProcessing^ textureProcessor;

	if (parser->extract)
	{
		List<String^>^ dds = gcnew List<String^>();
		
		dds->Add("content");
		dds->Add("dyndecals");
		dds->Add("gui/ui_render");
		dds->Add("helpers");
		dds->Add("maps");
		dds->Add("resources");
		dds->Add("spaces");
		dds->Add("system");
		dds->Add("default_ao.dds");

		List<String^>^ extracted = gcnew List<String^>();
				
		for each(String^ ddsPath in dds)
		{
			ExtractProcess^ extractor = gcnew ExtractProcess(parser);
			if (!extractor->Start(ddsPath))
				return -1;
			if (!extractor->Wait())
				return -1;
			if (extractor->pCode != ReturnValue::Unpacker::SUCCESS)
				return -1;
			for each(String^ path in extractor->GetExtracted())
			{
				extracted->Add(path->Replace("/", "\\"));
			}
		}

		textureProcessor = gcnew TextureProcessing(extracted, parser, pDevice);
	}
	else
	{
		textureProcessor = gcnew TextureProcessing(parser, pDevice);
	}
	textureProcessor->StartImagePipe();
	Console::ForegroundColor = ConsoleColor::White;

	//Add Zipping everything here

	if (parser->zip)
	{
		waiter->ZipFiles(parser->zipDirPath, parser->workingDir, parser->zipName);
		
		if (waiter->zipResult != ReturnValue::Zipper::SUCCESS)
			return -1;
	}

	return 0;
}