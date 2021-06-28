#include "pch.h"
#include "TextureProcessing.h"

using namespace System;

TextureProcessing::TextureProcessing(Collections::Generic::List<String^>^ files, Parser^ parser, ID3D11Device* pDevice)
{
	this->files = files;
	this->parser = parser;
	this->pDevice = pDevice;
	this->abort = false;
	this->amountThreads = Environment::ProcessorCount;
	this->threads = gcnew Collections::Generic::List<Threading::Thread^>();
	this->standard = gcnew Collections::Concurrent::ConcurrentQueue<String^>();
	this->current = 0;

	Console::ForegroundColor = ConsoleColor::Green;

	for each(String^ file in files)
	{
		this->standard->Enqueue(file);
	}

}

TextureProcessing::TextureProcessing(Parser^ parser, ID3D11Device* pDevice)
{
	this->files = gcnew Collections::Generic::List<String^>();	
	for each(String^ file in IO::Directory::GetFiles(parser->unpackerTargetDirectory, "*.dds", IO::SearchOption::AllDirectories))
	{
		this->files->Add(file);
	}
	this->parser = parser;
	this->pDevice = pDevice;
	this->abort = false;
	this->amountThreads = Environment::ProcessorCount;
	this->threads = gcnew Collections::Generic::List<Threading::Thread^>();
	this->standard = gcnew Collections::Concurrent::ConcurrentQueue<String^>();

	this->current = 0;
	Console::ForegroundColor = ConsoleColor::Green;

	for each(String^ file in files)
	{
		this->standard->Enqueue(file);
	}
}

ReturnValue::Texture TextureProcessing::StartImagePipe()
{
	for (unsigned int i = 0; i < amountThreads; i++)
	{
		try
		{
			Threading::Thread^ thread = gcnew Threading::Thread(gcnew Threading::ThreadStart(this, &TextureProcessing::ProcessImage));
			thread->Start();
			this->threads->Add(thread);
		}
		catch (Threading::ThreadStateException^ tse)
		{
			Writer::WriteTextureError(ReturnValue::Texture::THREAD_STATE, tse->Message);
			this->abort = true;
			break;
		}
		catch (OutOfMemoryException^ tm)
		{
			Writer::WriteTextureError(ReturnValue::Texture::THREAD_MEMORY, tm->Message);
			this->abort = true;
			break;
		}		
	}
	if (this->abort)
		return ReturnValue::Texture::ABORT;
	for each (Threading::Thread^ thread in threads)
	{
		try
		{
			thread->Join();
		}
		catch (Threading::ThreadStateException^ tse)
		{
			Writer::WriteTextureError(ReturnValue::Texture::THREAD_STATE, tse->Message);
			this->abort = true;
		}
		catch (Threading::ThreadInterruptedException^ tie)
		{
			Writer::WriteTextureError(ReturnValue::Texture::THREAD_INTERRUPTED, tie->Message);
			this->abort = true;
		}
	}
	if (this->abort)
		return ReturnValue::Texture::ABORT;
	return ReturnValue::Texture::SUCCESS;
}

void TextureProcessing::ProcessImage()
{
	String^ file;
	pin_ptr<const wchar_t> pathToLoad;
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage source;
	HRESULT result;
	size_t width;
	size_t height;
	unsigned int counterThread;

	while (this->standard->Count)
	{
		if (this->abort)
			return;
		if (!this->standard->TryDequeue(file))
			continue;
		if (parser->extract)
			file = parser->unpackerTargetDirectory + "\\" + file;
		pathToLoad = PtrToStringChars(file);
		if (this->abort)
			return;
		result = DirectX::LoadFromDDSFile(pathToLoad, DirectX::DDS_FLAGS_NONE, &metadata, source);
		if (result != S_OK)
		{
			Writer::WriteTextureError(ReturnValue::Texture::LOAD_FAIL, file);
			continue;
		}
		width = (size_t)round(metadata.width * this->parser->scale);
		height = (size_t)round(metadata.height * this->parser->scale);
		if (width == 0)
			width = 1;
		if (height == 0)
			height = 1;
		if (this->abort)
			return;
		if (DirectX::IsCompressed(metadata.format))
			CompressedWorkflow(file, &source, &metadata, width, height);
		else
			NormalWorkflow(file, &source, &metadata, width, height);
		this->counter->WaitOne();
		this->current++;
		counterThread = this->current;
		this->counter->ReleaseMutex();
		Writer::WriteTexture(counterThread, file);
	}
	return;
}

void TextureProcessing::CompressedWorkflow(String^ file, DirectX::ScratchImage* source, DirectX::TexMetadata* metadata, size_t width, size_t height)
{
	DirectX::ScratchImage temp1;
	DirectX::ScratchImage temp2;
	HRESULT result;
	result = DirectX::Decompress(source->GetImages(), source->GetImageCount(), source->GetMetadata(), DXGI_FORMAT_UNKNOWN, temp1);
	if (result != S_OK)
	{
		Writer::WriteTextureError(ReturnValue::Texture::DECOMPRESS_FAIL, file);
		temp1.Release();
		return;
	}
	if (this->abort)
		return;
	if (ResizeImage(&temp1, &temp2, width, height) != ReturnValue::Texture::SUCCESS)
	{
		Writer::WriteTextureError(ReturnValue::Texture::RESIZE_FAIL, file);
		temp1.Release();
		temp2.Release();
		return;
	}
	if (this->abort)
		return;
	if (height > 1 && width > 1)
	{
		if (GenerateMipMap(&temp2, &temp1, metadata) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::MIPMAP_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
		if (this->abort)
			return;
		if (CompressImage(&temp1, &temp2, metadata) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::COMPRESS_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
		if (this->abort)
			return;
		if (SaveImage(&temp2, file->Replace(parser->unpackerTargetDirectory, parser->workingDir)) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::SAVE_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
	}
	else
	{
		if (CompressImage(&temp2, &temp1, metadata) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::COMPRESS_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
		if (this->abort)
			return;
		if (SaveImage(&temp1, file->Replace(parser->unpackerTargetDirectory, parser->workingDir)) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::SAVE_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
	}
	if (this->abort)
		return;
	temp1.Release();
	temp2.Release();
	return;
}

void TextureProcessing::NormalWorkflow(String^ file, DirectX::ScratchImage* source, DirectX::TexMetadata* metadata, size_t width, size_t height)
{
	DirectX::ScratchImage temp1;
	DirectX::ScratchImage temp2;
	
	if (ResizeImage(source, &temp1, width, height) != ReturnValue::Texture::SUCCESS)
	{
		Writer::WriteTextureError(ReturnValue::Texture::RESIZE_FAIL, file);
		temp1.Release();
		return;
	}
	if (this->abort)
		return;
	if (height > 1 && width > 1)
	{
		if (GenerateMipMap(&temp1, &temp2, metadata) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::MIPMAP_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
		if (this->abort)
			return;
		if (SaveImage(&temp2, file->Replace(parser->unpackerTargetDirectory, parser->workingDir)) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::SAVE_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
	}
	else
	{
		if (SaveImage(&temp1, file->Replace(parser->unpackerTargetDirectory, parser->workingDir)) != ReturnValue::Texture::SUCCESS)
		{
			Writer::WriteTextureError(ReturnValue::Texture::SAVE_FAIL, file);
			temp1.Release();
			temp2.Release();
			return;
		}
	}
	if (this->abort)
		return;
	temp1.Release();
	temp2.Release();
	return;

}

ReturnValue::Texture TextureProcessing::ResizeImage(DirectX::ScratchImage* input, DirectX::ScratchImage* output, size_t width, size_t height)
{
	HRESULT result = DirectX::Resize(input->GetImages(), input->GetImageCount(), input->GetMetadata(), width, height, DirectX::TEX_FILTER_DEFAULT, *output);
	if (result != S_OK)
		return ReturnValue::Texture::RESIZE_FAIL;
	return ReturnValue::Texture::SUCCESS;
}

ReturnValue::Texture TextureProcessing::GenerateMipMap(DirectX::ScratchImage* input, DirectX::ScratchImage* output, DirectX::TexMetadata* metadata)
{
	HRESULT result;
	if (metadata->dimension == DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE3D)
		result = DirectX::GenerateMipMaps3D(input->GetImages(), input->GetImageCount(), input->GetMetadata(), DirectX::TEX_FILTER_LINEAR, 0, *output);
	else
		result = DirectX::GenerateMipMaps(input->GetImages(), input->GetImageCount(), input->GetMetadata(), DirectX::TEX_FILTER_LINEAR, 0, *output);
	if(result != S_OK)
		return ReturnValue::Texture::MIPMAP_FAIL;
	return ReturnValue::Texture::SUCCESS;
}

ReturnValue::Texture TextureProcessing::CompressImage(DirectX::ScratchImage* input, DirectX::ScratchImage* output, DirectX::TexMetadata* metadata)
{
	HRESULT result;
	if (metadata->format == DXGI_FORMAT_BC6H_UF16 || metadata->format == DXGI_FORMAT_BC6H_SF16 ||
		metadata->format == DXGI_FORMAT_BC7_UNORM || metadata->format == DXGI_FORMAT_BC7_UNORM_SRGB)
	{
		this->gpu->WaitOne();
		result = DirectX::Compress(this->pDevice, input->GetImages(), input->GetImageCount(), input->GetMetadata(), metadata->format, DirectX::TEX_COMPRESS_DEFAULT, 0.5, *output);
		this->gpu->ReleaseMutex();
	}
	else
		result = DirectX::Compress(input->GetImages(), input->GetImageCount(), input->GetMetadata(), metadata->format, DirectX::TEX_COMPRESS_DEFAULT, 0.5, *output);
	if (result != S_OK)
		return ReturnValue::Texture::COMPRESS_FAIL;
	return ReturnValue::Texture::SUCCESS;
}

ReturnValue::Texture TextureProcessing::SaveImage(DirectX::ScratchImage* input, String^ file)
{
	pin_ptr<const wchar_t> pathToSave = PtrToStringChars(file);
	
	String^ dir = IO::Path::GetDirectoryName(file);
	IO::Directory::CreateDirectory(dir);
	HRESULT result = DirectX::SaveToDDSFile(input->GetImages(), input->GetImageCount(), input->GetMetadata(), DirectX::DDS_FLAGS_NONE, pathToSave);
	if (result != S_OK)
		return ReturnValue::Texture::SAVE_FAIL;
	return ReturnValue::Texture::SUCCESS;
}
