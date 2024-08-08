/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#if __ANDROID__

#include "AndroidFileSystem.h"

#include <cctype>
#include <cstddef>

#include <filesystem>
#include <fstream>
#include <system_error>

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

#include "MemoryStream.h"

using namespace openblack::filesystem;

namespace
{
// Adapted from https://stackoverflow.com/a/13059195/10604387
//          and https://stackoverflow.com/a/46069245/10604387
struct membuf: std::streambuf
{
	membuf(char const* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
	std::streampos seekoff(off_type off, std::ios_base::seekdir way, [[maybe_unused]] std::ios_base::openmode which) override
	{
		if (way == std::ios_base::cur)
		{
			gbump(static_cast<int>(off));
		}
		else if (way == std::ios_base::end)
		{
			setg(eback(), egptr() + off, egptr());
		}
		else if (way == std::ios_base::beg)
		{
			setg(eback(), eback() + off, egptr());
		}
		return gptr() - eback();
	}

	std::streampos seekpos([[maybe_unused]] pos_type pos, [[maybe_unused]] std::ios_base::openmode which) override
	{
		return seekoff(pos - static_cast<off_type>(0), std::ios_base::beg, which);
	}
};
struct imemstream: virtual membuf, std::istream
{
	imemstream(std::vector<uint8_t> data)
	    : membuf(reinterpret_cast<const char*>(data.data()), data.size())
	    , std::istream(dynamic_cast<std::streambuf*>(this))
	    , _data(data)
	{
		auto p = reinterpret_cast<char*>(_data.data());
		this->setg(p, p, p + data.size());
	}

	imemstream(char const* base, size_t size)
	    : membuf(base, size)
	    , std::istream(dynamic_cast<std::streambuf*>(this))
	{
	}

private:
	std::vector<uint8_t> _data;
};
} // namespace

AndroidFileSystem::AndroidFileSystem()
    : _jniEnv(static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv()))
    , _jniActivity(static_cast<jobject>(SDL_AndroidGetActivity()))
    , _jniInteropClass((jclass)_jniEnv->NewGlobalRef(_jniEnv->FindClass("org/openblack/app/FileSystemInterop")))
{
	// You need to create a global reference to use it outside the method where it was created
	_jniReadFileFromPathMid = _jniEnv->GetStaticMethodID(_jniInteropClass, "readFileFromPath",
	                                                     "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)[B");
	_jniListFilesFromPathMid =
	    _jniEnv->GetStaticMethodID(_jniInteropClass, "listFilesFromPath",
	                               "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Z)[Ljava/lang/String;");
}

AndroidFileSystem::~AndroidFileSystem()
{
	// Remember to clean up the global reference when you're done
	_jniEnv->DeleteGlobalRef(_jniInteropClass);
}

std::filesystem::path AndroidFileSystem::FindPath(const std::filesystem::path& path) const
{
	if (path.empty())
	{
		throw std::invalid_argument("empty path");
	}

	return path;
}

bool AndroidFileSystem::IsPathValid(const std::filesystem::path& path)
{
	jstring jgamePath = _jniEnv->NewStringUTF(path.c_str());
	jstring jpath = _jniEnv->NewStringUTF("/");

	jmethodID midGetDirectoryFromPath =
	    _jniEnv->GetStaticMethodID(_jniInteropClass, "getDirectoryFromPath",
	                               "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;");
	if (midGetDirectoryFromPath == nullptr)
	{
		spdlog::error("Failed to find method: getDirectoryFromPath");
		return false;
	}

	jobject juri = _jniEnv->CallStaticObjectMethod(_jniInteropClass, midGetDirectoryFromPath, _jniActivity, jgamePath, jpath);

	bool isValid = juri != nullptr;

	if (isValid)
	{
		_jniEnv->DeleteLocalRef(juri);
	}

	_jniEnv->DeleteLocalRef(jgamePath);
	_jniEnv->DeleteLocalRef(jpath);

	return isValid;
}

std::unique_ptr<Stream> AndroidFileSystem::Open(const std::filesystem::path& path, Stream::Mode mode)
{
	jstring jpath = _jniEnv->NewStringUTF(path.c_str());
	jstring jgamePath = _jniEnv->NewStringUTF(_gamePath.c_str());

	// You need to create a global reference to use it outside the method where it was created
	jbyteArray jbytes =
	    (jbyteArray)_jniEnv->CallStaticObjectMethod(_jniInteropClass, _jniReadFileFromPathMid, _jniActivity, jgamePath, jpath);

	jsize length = _jniEnv->GetArrayLength(jbytes);
	jbyte* jbytesPtr = _jniEnv->GetByteArrayElements(jbytes, nullptr);

	std::vector<uint8_t> bytes(jbytesPtr, jbytesPtr + length);
	auto value = std::unique_ptr<Stream>(new MemoryStream(std::move(bytes)));

	_jniEnv->ReleaseByteArrayElements(jbytes, jbytesPtr, 0);
	_jniEnv->DeleteLocalRef(jbytes);
	return value;
}

bool AndroidFileSystem::Exists(const std::filesystem::path& path) const
{
	try
	{
		[[maybe_unused]] auto realPath = FindPath(path);
		return true;
	}
	catch (std::exception&)
	{
		return false;
	}
}

std::vector<uint8_t> AndroidFileSystem::ReadAll(const std::filesystem::path& path)
{
	auto file = Open(path, Stream::Mode::Read);
	std::size_t size = file->Size();

	std::vector<uint8_t> data(size);
	file->Read(data.data(), size);

	return data;
}

void AndroidFileSystem::Iterate(const std::filesystem::path& path, bool recursive,
                                const std::function<void(const std::filesystem::path&)>& function) const
{
	// Converting C++ string to Java string
	jstring jgamePath = _jniEnv->NewStringUTF(_gamePath.c_str());
	jstring jpath = _jniEnv->NewStringUTF(path.c_str());
	jboolean jrecursive = (jboolean)recursive;

	// Calling Java method
	jobjectArray jfilePaths = (jobjectArray)_jniEnv->CallStaticObjectMethod(_jniInteropClass, _jniListFilesFromPathMid,
	                                                                        _jniActivity, jgamePath, jpath, jrecursive);

	// Processing returned string array
	int stringCount = _jniEnv->GetArrayLength(jfilePaths);

	for (int i = 0; i < stringCount; i++)
	{
		jstring filePath = (jstring)(_jniEnv->GetObjectArrayElement(jfilePaths, i));
		const char* rawString = _jniEnv->GetStringUTFChars(filePath, nullptr);

		// Calling provided function
		function(path / rawString);

		// Don't forget to release the string
		_jniEnv->ReleaseStringUTFChars(filePath, rawString);
	}
}

std::unique_ptr<std::istream> AndroidFileSystem::GetData(const std::filesystem::path& path)
{
	return std::make_unique<imemstream>(ReadAll(path));
}

#endif
