#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
using byte = unsigned char;

namespace JPF
{
	const int NameHashLength = 8; // 8 bytes to store the file name as a CityHash64
	const int FileTypeLength = 1; // 1 byte to store the file type
	const int FileSizeLength = 4; // 4 bytes to store the file size in bytes
	const int TotalFileDescriptionSize = NameHashLength + FileTypeLength + FileSizeLength;

	// Must be 0-255 to fit inside of a single byte
	enum FileType : byte
	{
		PNG,
		JPG, // Or, JPEG
		OBJ, // Wavefront OBJ model file
		FBX, // Filmbox model file
		TXT,
		HLSL, // DirectX Shader code
		VERT, // Vertex Shader Code
		FRAG, // Fragment Shader Code
		VERTC, // Vertex Shader (Compiled)
		FRAGC, // Fragment Shader (Compiled)
		FX, // Shader Effect Code
		FXC, // Shader Effect (Compiled)
		EXE, // Executable (Compiled)
		DLL, // Dynamic-linky library (Compiled)
		JSON, // JavaScript Object Notation (Raw text, Formatted)
		JPF, // Ji9sw Package File (Compiled)
		// Add More
		UNK = 255
	};

	enum ReadResults
	{
		Success,
		NoMagicHeader,
		NoFiles,
		CorruptionOccurred
	};

	enum FileSizes
	{
		Bytes,
		Kilobytes,
		Megabytes,
		Gigabytes
	};

	FileSizes GetRecommendedSizeFromBytes(int Bytes)
	{
		if (Bytes / 1024 > 1)
		{
			if (Bytes / 1024 / 1024 > 1)
			{
				if (Bytes / 1024 / 1024 / 1024 > 1)
				{
					return FileSizes::Gigabytes;
				}
				else return FileSizes::Megabytes;
			}
			else return FileSizes::Kilobytes;
		}
		else return FileSizes::Bytes;
	}

	template <typename T>
	T BytesToType(std::vector<byte>& Data, size_t Start, bool LittleEndian = true)
	{
		T Value = 0;
		for (std::size_t Index = 0; Index < sizeof(T); ++Index)
		{
			if (LittleEndian)
				Value |= static_cast<T>(static_cast<unsigned char>(Data[Start + Index])) << (Index * 8);
			else
				Value |= static_cast<T>(static_cast<unsigned char>(Data[Start + Index])) << ((sizeof(T) - 1 - Index) * 8);
		}
		return Value;
	}

	std::vector<byte> ReadFileAsByteArray(const char* FileName)
	{
		std::ifstream File(FileName, std::ios::binary | std::ios::ate);

		if (File)
		{
			std::streamsize FileSize = File.tellg();
			File.seekg(0, std::ios::beg);

			std::vector<byte> Buffer(FileSize);
			if (File.read(reinterpret_cast<char*>(Buffer.data()), FileSize))
				return Buffer;

			File.close();
		}

		return std::vector<byte>();
	}

	// Checks if the first 3 bytes of the data is "JPF"
	bool FileHasMagicHeader(std::vector<byte>& Data)
	{
		return Data.size() >= 3 &&
			Data[0] == (byte)'J' &&
			Data[1] == (byte)'P' &&
			Data[2] == (byte)'F';
	}

	// A JPF contains a list of Asset Data
	struct JPFAssetData
	{
		uint64_t FileNameHash;
		FileType Type;
		int FileSize;
		std::vector<byte> FileData;
	};

	struct JPFData
	{
		std::vector<JPFAssetData> Assets;
		bool Valid = false;
		ReadResults ReadResult;

		template <typename T>
		T BytesToTypeWithOffset(const std::vector<byte>& Data, size_t& Offset, bool LittleEndian = true)
		{
			if (Offset + sizeof(T) > Data.size()) return T{};

			T Value = 0;
			for (std::size_t Index = 0; Index < sizeof(T); ++Index)
			{
				if (LittleEndian)
					Value |= static_cast<T>(static_cast<unsigned char>(Data[Offset + Index])) << (Index * 8);
				else
					Value |= static_cast<T>(static_cast<unsigned char>(Data[Offset + Index])) << ((sizeof(T) - 1 - Index) * 8);
			}
			Offset += sizeof(T);
			return Value;
		}

		void From(const char* FileName)
		{
			std::vector<byte> Data = ReadFileAsByteArray(FileName);
			if (FileHasMagicHeader(Data))
			{
				Valid = true;
				size_t Offset = 3;

				if (!Data.empty())
				{
					while (Data.size() - Offset >= TotalFileDescriptionSize) // this file contains another file
					{
						JPFAssetData NewAsset;

						NewAsset.FileNameHash = BytesToTypeWithOffset<uint64_t>(Data, Offset);
						NewAsset.Type = (FileType)BytesToTypeWithOffset<byte>(Data, Offset);
						NewAsset.FileSize = BytesToTypeWithOffset<int>(Data, Offset);

						if (Data.size() - Offset >= NewAsset.FileSize)
						{
							NewAsset.FileData.assign(Data.begin() + Offset, Data.begin() + Offset + NewAsset.FileSize);
							Offset += NewAsset.FileSize;
						}
						else
						{
							// Handle corrupted file cases where declared file size is larger than remaining data
							ReadResult = CorruptionOccurred;
							return;
						}

						Assets.push_back(NewAsset);
					}
					ReadResult = Success;
				}
				else
					ReadResult = NoFiles;
			}
			else
				ReadResult = NoMagicHeader;
		}
	};
}