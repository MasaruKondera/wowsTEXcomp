#pragma once
#include "master.h"

ref class Parser;
ref class Writer;

ref struct NoDDSFiles : public System::Exception {
	NoDDSFiles(System::String^ message) : System::Exception(message) {}
};

ref class TextureProcessing
{

private:
	unsigned int amountThreads;
	Parser^ parser;
	System::Collections::Generic::List<System::String^>^ files;
	ID3D11Device* pDevice;
	bool abort;
	System::Collections::Generic::List<System::Threading::Thread^>^ threads;
	System::Collections::Concurrent::ConcurrentQueue<System::String^>^ standard;
	void TextureProcessing::ProcessImage();
	void CompressedWorkflow(System::String^ file, DirectX::ScratchImage* source, DirectX::TexMetadata* metadata, size_t width, size_t height);
	void NormalWorkflow(System::String^ file, DirectX::ScratchImage* source, DirectX::TexMetadata* metadata, size_t width, size_t height);
	
	unsigned int current;

	ReturnValue::Texture ResizeImage(DirectX::ScratchImage* input, DirectX::ScratchImage* output, size_t width, size_t height);
	ReturnValue::Texture GenerateMipMap(DirectX::ScratchImage* input, DirectX::ScratchImage* output, DirectX::TexMetadata* metadata);
	ReturnValue::Texture CompressImage(DirectX::ScratchImage* input, DirectX::ScratchImage* output, DirectX::TexMetadata* metadata);
	ReturnValue::Texture SaveImage(DirectX::ScratchImage* input, System::String^ file);

	static System::Threading::Mutex^ gpu = gcnew System::Threading::Mutex();
	static System::Threading::Mutex^ counter = gcnew System::Threading::Mutex();

public:
	TextureProcessing(System::Collections::Generic::List<System::String^>^ files, Parser^ parser, ID3D11Device* pDevice);
	TextureProcessing(Parser^ parser, ID3D11Device* pDevice);
	ReturnValue::Texture StartImagePipe();
};

