#include <Elyrium/Core/File.hpp>

#include <LSD/Array.h>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#ifdef ELYRIUM_POSIX
#include <unistd.h>
#elif defined(ELYRIUM_WINDOWS)
#include <io.h>
#endif

namespace elyrium {

namespace filesys {

namespace globals {

FileSystem* fileSystem;

}

// Utility

namespace {

constexpr const char* enumToOpenMode(OpenMode m) {
	constexpr lsd::Array<const char*, 15> openModes {
		"rt",
		"wt",
		"at",
		"",
		"r+t",
		"w+t",
		"a+t",
		"",
		"rb",
		"wb",
		"ab",
		"",
		"r+b",
		"w+b",
		"a+b"
	};

	return openModes[static_cast<size_type>(m)];
}

}


// File system

bool FileSystem::exists(lsd::StringView path) const {
#ifdef ELYRIUM_POSIX
	return access(path.data(), F_OK) == 0;
#elif defined(ELYRIUM_WINDOWS)
	return _access(path.data(), 0) == 0;
#else
	auto file = std::fopen(path.data(), "r");
	if (file) {
		std::fclose(file);
		return true;
	}

	return false;
#endif
}

File FileSystem::load(lsd::StringView path, OpenMode mode, bool buffered) {
	// @todo implement as multithreaded
	// return signal (bool mutex?)
	// run on different thread and set bool to true when done
	auto file = std::fopen(path.data(), enumToOpenMode(mode));
	if (!file) throw FilesystemError("Failed to load file");

	return File(file, (buffered ? unusedBuffer() : nullptr), path.data());
}

char* FileSystem::unusedBuffer() {
	if (m_buffers.empty()) m_buffers.pushBack(new char[bufferSize]);

	auto r = m_buffers.back();
	m_buffers.popBack();
	return r;
}
	
void FileSystem::returnBuffer(char* buffer) {
	memset(buffer, '\0', bufferSize);
	m_buffers.pushBack(buffer);
}

File FileSystem::tmpFile() {
	return File(std::tmpfile(), nullptr, { });
}


// File

BasicFile<char>::~BasicFile<char>() {
	if (m_buffer) globals::fileSystem->returnBuffer(m_buffer);
}
void BasicFile<char>::close() {
	if (m_buffer) globals::fileSystem->returnBuffer(m_buffer);
}

void BasicFile<char>::disableBuffering() {
	if (m_buffered) {
		std::fflush(m_stream.get());
		std::setbuf(m_stream.get(), nullptr);
		globals::fileSystem->returnBuffer(m_buffer);
	}
}
void BasicFile<char>::enableBuffering() {
	if (!m_buffered) {
		std::setvbuf(m_stream.get(), globals::fileSystem->unusedBuffer(), _IOFBF, FileSystem::bufferSize);
	}
}

int BasicFile<char>::get() {
	return std::fgetc(m_stream.get());
}
BasicFile<char>& BasicFile<char>::get(char& c) {
	c = std::fgetc(m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::get(char* string, size_type count) {
	std::fgets(string, static_cast<std::uint32_t>(count), m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::putback(int c) {
	std::ungetc(c, m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::unget() {
	std::fseek(m_stream.get(), -1, 0);
	return *this;
}
BasicFile<char>& BasicFile<char>::read(char* string, size_type count) {
	std::fread(string, sizeof(char), count, m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::read(void* string, size_type size, size_type count) {
	std::fread(string, size, count, m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::put(char c) {
	std::fputc(c, m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::write(const void* string, size_type size, size_type count) {
	std::fwrite(string, size, count, m_stream.get());
	return *this;
}
BasicFile<char>& BasicFile<char>::write(const char* string, size_type count) {
	std::fwrite(string, sizeof(char), count, m_stream.get());
	return *this;
}

std::fpos_t BasicFile<char>::tellg() const {
	return std::ftell(m_stream.get());
}
std::fpos_t BasicFile<char>::tellp() const {
	return std::ftell(m_stream.get());
}
BasicFile<char>& BasicFile<char>::seekg(std::fpos_t pos) {
	std::fseek(m_stream.get(), pos, SEEK_SET);
	return *this;
}
BasicFile<char>& BasicFile<char>::seekg(std::fpos_t off, SeekDirection dir) {
	std::fseek(m_stream.get(), off, static_cast<int>(dir));
	return *this;
}
BasicFile<char>& BasicFile<char>::seekp(std::fpos_t pos) {
	std::fseek(m_stream.get(), pos, SEEK_SET);
	return *this;
}
BasicFile<char>& BasicFile<char>::seekp(std::fpos_t off, SeekDirection dir) {
	std::fseek(m_stream.get(), off, static_cast<int>(dir));
	return *this;
}
size_type BasicFile<char>::size() const {
	auto p = std::ftell(m_stream.get());
	std::fseek(m_stream.get(), 0, SEEK_END);
	auto r = std::ftell(m_stream.get());
	std::fseek(m_stream.get(), p, SEEK_SET);
	return r;
}

BasicFile<char>& BasicFile<char>::flush() {
	std::fflush(m_stream.get());
	return *this;
}
int BasicFile<char>::sync() {
	return std::fflush(m_stream.get());
}

bool BasicFile<char>::good() const {
	return m_stream.get();
}
bool BasicFile<char>::eof() const {
	return std::feof(m_stream.get()) != 0;
}
void BasicFile<char>::clear() {
	std::clearerr(m_stream.get());
}

void BasicFile<char>::swap(BasicFile<char>& file) {
	std::swap(flush().m_stream, file.flush().m_stream);
	std::swap(m_buffer, file.m_buffer);
	m_path.swap(file.m_path);
	std::swap(m_buffered, file.m_buffered);
}

void BasicFile<char>::rename(lsd::StringView newPath) {
	std::rename(m_path.data(), newPath.data());
	m_path = newPath;
}


// WFile

BasicFile<wchar_t>::~BasicFile() {
	if (m_buffer) globals::fileSystem->returnBuffer(m_buffer);
}
void BasicFile<wchar_t>::close() {
	if (m_buffer) globals::fileSystem->returnBuffer(m_buffer);
}

void BasicFile<wchar_t>::disableBuffering() {
	if (m_buffered) {
		std::fflush(m_stream.get());
		std::setbuf(m_stream.get(), nullptr);
	}
}
void BasicFile<wchar_t>::enableBuffering() {
	if (!m_buffered) {
		std::setvbuf(m_stream.get(), globals::fileSystem->unusedBuffer(), _IOFBF, FileSystem::bufferSize);
	}
}

int BasicFile<wchar_t>::get() {
	return std::fgetwc(m_stream.get());
}
BasicFile<wchar_t>& BasicFile<wchar_t>::get(wchar_t& c) {
	c = std::fgetwc(m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::get(wchar_t* string, size_type count) {
	std::fgetws(string, static_cast<std::uint32_t>(count), m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::putback(int c) {
	std::ungetwc(c, m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::unget() {
	std::fseek(m_stream.get(), -1, 0);
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::read(wchar_t* string, size_type count) {
	std::fread(string, sizeof(wchar_t), count, m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::read(void* string, size_type size, size_type count) {
	std::fread(string, size, count, m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::put(wchar_t c) {
	std::fputwc(c, m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::write(const void* string, size_type size, size_type count) {
	std::fwrite(string, size, count, m_stream.get());
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::write(const wchar_t* string, size_type count) {
	std::fwrite(string, sizeof(wchar_t), count, m_stream.get());
	return *this;
}

std::fpos_t BasicFile<wchar_t>::tellg() const {
	return std::ftell(m_stream.get());
}
std::fpos_t BasicFile<wchar_t>::tellp() const {
	return std::ftell(m_stream.get());
}
BasicFile<wchar_t>& BasicFile<wchar_t>::seekg(std::fpos_t pos) {
	std::fseek(m_stream.get(), pos, SEEK_SET);
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::seekg(std::fpos_t off, SeekDirection dir) {
	std::fseek(m_stream.get(), off, static_cast<int>(dir));
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::seekp(std::fpos_t pos) {
	std::fseek(m_stream.get(), pos, SEEK_SET);
	return *this;
}
BasicFile<wchar_t>& BasicFile<wchar_t>::seekp(std::fpos_t off, SeekDirection dir) {
	std::fseek(m_stream.get(), off, static_cast<int>(dir));
	return *this;
}
size_type BasicFile<wchar_t>::size() const {
	auto p = std::ftell(m_stream.get());
	std::fseek(m_stream.get(), 0, SEEK_END);
	auto r = std::ftell(m_stream.get());
	std::fseek(m_stream.get(), p, SEEK_SET);
	return r;
}

BasicFile<wchar_t>& BasicFile<wchar_t>::flush() {
	std::fflush(m_stream.get());
	return *this;
}
int BasicFile<wchar_t>::sync() {
	return std::fflush(m_stream.get());
}

bool BasicFile<wchar_t>::good() const {
	return (std::ferror(m_stream.get()) != 0) && (m_stream);
}
bool BasicFile<wchar_t>::eof() const {
	return std::feof(m_stream.get()) != 0;
}
void BasicFile<wchar_t>::clear() {
	std::clearerr(m_stream.get());
}

void BasicFile<wchar_t>::swap(BasicFile& file) {
	std::swap(flush().m_stream, file.flush().m_stream);
	std::swap(m_buffer, file.m_buffer);
	m_path.swap(file.m_path);
	std::swap(m_buffered, file.m_buffered);
}

void BasicFile<wchar_t>::rename(lsd::StringView newPath) {
	std::rename(m_path.data(), newPath.data());
	m_path = newPath;
}

} // namespace filesys

} // namespace elyrium
