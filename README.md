# JPF Reader
JPF (Ji9sw Package File) is a custom file format made to store a bundle of files to be read by a game engine

JPF is **not** something to be used by a user, and is instead made for game and engine developers to implement into their engines

## JPF File Format

JPF files can be identified using the first 3 bytes of a `.jpf` file, the first 3 bytes should always be a magic header reading `JPF`

After the magic header, the first few bytes will be:
- 8 bytes for file name hash (CityHash64)
- 1 byte for file type
- 4 bytes for file length
- [FILE_LENGTH] bytes for file data
- next file...

## C# JPF Creator

A C# program has been developed which can easily create JPF files that are readable by this application

[View Source Code](https://github.com/ji8sw/JPF-Creator)

## To-Do
- [ ] Add optional file compression
- [ ] Add optional file encryption
- [x] C# JPF Creator