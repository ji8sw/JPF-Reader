#include <iostream>
#include <format>
#include "JPFReader.h"

int main(int ArgumentCount, char* ArgumentValues[])
{
	const char* JPFFileName = "Package.jpf";

	if (ArgumentCount >= 2)
		JPFFileName = ArgumentValues[1];

	printf("Reading package file...\n");
	printf(JPFFileName);
	printf("\n");

	JPF::JPFData Package;
	Package.From(JPFFileName);

	if (Package.ReadResult != JPF::Success)
	{
		switch (Package.ReadResult)
		{
		case JPF::NoMagicHeader:
			printf("This is not a valid JPF...");
			return 1;
			break;
		case JPF::NoFiles:
			printf("No files in this package...");
			return 1;
			break;
		case JPF::CorruptionOccurred:
			printf("This JPF is corrupt...");
			return 1;
			break;
		}
	}

	printf(std::format("{} assets in this package...\n", Package.Assets.size()).c_str());

	for (int Index = 0; Index < Package.Assets.size(); Index++)
	{
		JPF::JPFAssetData Asset = Package.Assets[Index];
		printf(std::format("Reading Asset: #{}: \n", Asset.FileNameHash).c_str());

		switch (Asset.Type)
		{
		case JPF::PNG:
			printf("	Asset Type: Texture (PNG)\n");
			break;
		case JPF::OBJ:
			printf("	Asset Type: Model (OBJ)\n");
			break;
		case JPF::TXT:
			printf("	Asset Type: Raw Text (TXT)\n");
			break;
		case JPF::UNK:
			printf("	Asset Type: Unknown (UNK)\n");
			break;
		}

		switch (JPF::GetRecommendedSizeFromBytes(Asset.FileSize))
		{
		case JPF::Bytes:
			printf(std::format("	Asset Size: {}B\n", Asset.FileSize).c_str());
			break;
		case JPF::Kilobytes:
			printf(std::format("	Asset Size: {}KB\n", Asset.FileSize / 1024).c_str());
			break;
		case JPF::Megabytes:
			printf(std::format("	Asset Size: {}MB\n", Asset.FileSize / 1024 / 1024).c_str());
			break;
		case JPF::Gigabytes:
			printf(std::format("	Asset Size: {}GB\n", Asset.FileSize / 1024 / 1024 / 1024).c_str());
			break;
		}
	}

	system("pause");
	return 0;
}