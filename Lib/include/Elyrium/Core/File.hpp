/*************************
 * @file File.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 * 
 * @brief Filesystem
 * 
 * @date 2024-06-26
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Elyrium/Core/Common.hpp>

#include <LSD/SharedPointer.h>
#include <LSD/Vector.h>
#include <LSD/String.h>
#include <LSD/UnorderedSparseMap.h>

namespace elyrium {

namespace filesys {

class FilesystemError : public std::runtime_error {
public:
	FilesystemError(const lsd::String& message) : std::runtime_error(message.cStr()) {
		m_message.append(message).pushBack('!');
	}
	FilesystemError(const char* message) : std::runtime_error(message) {
		m_message.append(message).pushBack('!');
	}
	FilesystemError(const FilesystemError&) = default;
	FilesystemError(FilesystemError&&) = default;

	FilesystemError& operator=(const FilesystemError&) = default;
	FilesystemError& operator=(FilesystemError&&) = default;

	const char* what() const noexcept override {
		return m_message.cStr();
	}

private:
	lsd::String m_message { "Program terminated with FilesystemError: " };
};


enum class OpenMode {
	read,
	write,
	append,
	extend = 0x04,
	binary = 0x08
};

enum class SeekDirection {
	begin = SEEK_SET,
	current = SEEK_CUR,
	end = SEEK_END
};

enum class FileState {
	good = 0x00000000,
	bad = 0x0000001,
	fail = 0x00000002,
	eof = 0x00000004
};


template <class> class BasicFile;

template <> class BasicFile<char> {
public: 
	using literal_type = char;
	using file_type = lsd::SharedPointer<std::FILE>;

	BasicFile() = default;
	BasicFile(const BasicFile&) = default;
	BasicFile(BasicFile&&) = default;
	BasicFile& operator=(const BasicFile&) = default;
	BasicFile& operator=(BasicFile&&) = default;

	~BasicFile();
	void close();

	void disableBuffering();
	void enableBuffering();

	int get();
	BasicFile& get(char& c);
	BasicFile& get(char* string, size_type count);
	BasicFile& putback(int c);
	BasicFile& unget();
	BasicFile& read(char* string, size_type count);
	BasicFile& read(void* string, size_type size, size_type count);

	BasicFile& put(char c);
	BasicFile& write(const void* string, size_type size, size_type count);
	BasicFile& write(const char* string, size_type count);

	BasicFile& flush();
	int sync();

	std::fpos_t tellg() const;
	std::fpos_t tellp() const;
	BasicFile& seekg(std::fpos_t pos);
	BasicFile& seekg(std::fpos_t off, SeekDirection dir);
	BasicFile& seekp(std::fpos_t pos);
	BasicFile& seekp(std::fpos_t off, SeekDirection dir);
	size_type size() const;

	bool good() const;
	bool eof() const;
	bool fail() const;
	bool operator!() const {
		return fail();
	}
	operator bool() const {
		return !fail();
	}
	void clear();

	void swap(BasicFile& file);

	void rename(lsd::StringView newPath);
	[[nodiscard]] lsd::String path() const noexcept {
		return m_path;
	}
	[[nodiscard]] bool buffered() const noexcept {
		return m_buffered;
	}
	[[nodiscard]] const file_type& stream() const noexcept {
		return m_stream;
	}
	[[nodiscard]] file_type& stream() noexcept {
		return m_stream;
	}

private:
	file_type m_stream = nullptr;
	char* m_buffer = nullptr;

	lsd::String m_path;

	bool m_buffered;

	BasicFile(file_type file, char* buffer, lsd::StringView path) : m_stream(file), m_buffer(buffer), m_path(path) { 
		if (m_buffer) enableBuffering();
	}

	friend class FileSystem;
};

template <> class BasicFile<wchar_t> {
public:
	using literal_type = wchar_t;
	using file_type = lsd::SharedPointer<std::FILE>;

	BasicFile() = default;
	BasicFile(const BasicFile&) = default;
	BasicFile(BasicFile&&) = default;
	BasicFile& operator=(const BasicFile&) = default;
	BasicFile& operator=(BasicFile&&) = default;

	~BasicFile();
	void close();

	void disableBuffering();
	void enableBuffering();

	int get();
	BasicFile& get(wchar_t& c);
	BasicFile& get(wchar_t* string, size_type count);
	BasicFile& putback(int c);
	BasicFile& unget();
	BasicFile& read(wchar_t* string, size_type count);
	BasicFile& read(void* string, size_type size, size_type count);

	BasicFile& put(wchar_t c);
	BasicFile& write(const void* string, size_type size, size_type count);
	BasicFile& write(const wchar_t* string, size_type count);

	BasicFile& flush();
	int sync();

	std::fpos_t tellg() const;
	std::fpos_t tellp() const;
	BasicFile& seekg(std::fpos_t pos);
	BasicFile& seekg(std::fpos_t off, SeekDirection dir);
	BasicFile& seekp(std::fpos_t pos);
	BasicFile& seekp(std::fpos_t off, SeekDirection dir);
	size_type size() const;
	
	bool good() const;
	bool eof() const;
	bool fail() const;
	bool operator!() const {
		return fail();
	}
	operator bool() const {
		return !fail();
	}
	void clear();

	void swap(BasicFile& file);

	void rename(lsd::StringView newPath);
	[[nodiscard]] lsd::String path() const noexcept {
		return m_path;
	}
	[[nodiscard]] bool buffered() const noexcept {
		return m_buffered;
	}
	[[nodiscard]] const file_type& stream() const noexcept {
		return m_stream;
	}
	[[nodiscard]] file_type& stream() noexcept {
		return m_stream;
	}

private:
	file_type m_stream = nullptr;
	char* m_buffer = nullptr;

	lsd::String m_path = { };

	bool m_buffered;

	BasicFile(file_type file, char* buffer, lsd::StringView path) : m_stream(file), m_buffer(buffer), m_path(path.data()) { 
		if (m_buffer) enableBuffering();
	}

	friend class FileSystem;
};

using File = BasicFile<char>;
using WFile = BasicFile<wchar_t>;


class FileSystem {
public:
	static constexpr size_type bufferSize = BUFSIZ;
	static constexpr size_type maxFiles = FOPEN_MAX;

	bool exists(lsd::StringView path) const;

	[[nodiscard]] File load(lsd::StringView path, OpenMode mode, bool buffered = true);
	[[nodiscard]] File tmpFile();

private:
	[[nodiscard]] char* unusedBuffer();
	void returnBuffer(char* buffer);

	lsd::Vector<char*> m_buffers;

	template <class> friend class BasicFile;
};

} // namespace filesys

} // namespace elyrium
