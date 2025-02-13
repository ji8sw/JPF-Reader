#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
using byte = char;

namespace JPF
{
	const int NameHashLength = 4; // 4 bytes to store the file name as a CityHash32
	const int FileTypeLength = 1; // 1 byte to store the file type
	const int FileSizeLength = 4; // 4 bytes to store the file size in bytes
	const int TotalFileDescriptionSize = NameHashLength + FileTypeLength + FileSizeLength;

	// Must be 0-255 to fit inside of a single byte
	enum FileType : byte
	{
		PNG,
		OBJ,
		TXT,
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
					if (Bytes / 1024 / 1024 / 1024 / 1024 > 1)
					{
						return FileSizes::Gigabytes;
					}
					else return FileSizes::Gigabytes;
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
		int FileNameHash;
		FileType Type;
		int FileSize;
		std::vector<byte> FileData;
	};

	struct JPFData
	{
		std::vector<JPFAssetData> Assets;
		bool Valid = false;
		ReadResults ReadResult;

		// JPF specific BytesToType which erases data it goes over
		template <typename T>
		T BytesToTypeErase(std::vector<byte>& Data, bool LittleEndian = true)
		{
			T Value = BytesToType<T>(Data, 0, LittleEndian);
			Data.erase(Data.begin(), Data.begin() + sizeof(T));
			return Value;
		}

		void From(const char* FileName)
		{
			std::vector<byte> Data = ReadFileAsByteArray(FileName);
			if (FileHasMagicHeader(Data))
			{
				Valid = true;
				Data.erase(Data.begin(), Data.begin() + 3); // Delete magic header bytes

				if (!Data.empty())
				{
					while (Data.size() > TotalFileDescriptionSize) // this file contains another file
					{
						JPFAssetData NewAsset;
						int TotalOffset = 0;

						NewAsset.FileNameHash = BytesToTypeErase<int>(Data);
						NewAsset.Type = (FileType)BytesToTypeErase<byte>(Data);
						NewAsset.FileSize = BytesToTypeErase<int>(Data);

						if (Data.size() >= NewAsset.FileSize)
						{
							NewAsset.FileData.assign(Data.begin(), Data.begin() + NewAsset.FileSize);
							Data.erase(Data.begin(), Data.begin() + NewAsset.FileSize);
						}
						else
						{
							// Handle corrupted file cases where declared file size is larger than remaining data
							ReadResult = CorruptionOccurred;
							break;
						}

						Assets.push_back(NewAsset);
						ReadResult = Success;
					}
				}
				else
					ReadResult = NoFiles;
			}
			else
				ReadResult = NoMagicHeader;
		}
	};
}