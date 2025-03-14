#include "fetch.hpp"
#include <spdlog/spdlog.h>
#include <assimp/Importer.hpp>
#include <stb_image.h>
#include <filesystem>

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#else
#include <fstream>
#endif

std::string joinPath(std::string a, std::string b) {
	return (std::filesystem::path(a) / std::filesystem::path(b)).string();
}

void fetch_image(std::string root, std::string path, std::function<void(stbi_uc*, int, int, int)> handler) {
	std::string fullPath = joinPath(root, path);
	fetch_data(root, path, [fullPath=std::move(fullPath), handler=std::move(handler)](unsigned int size, unsigned char* data) {
		int x, y, channels;
		stbi_uc* image = stbi_load_from_memory(data, size, &x, &y, &channels, 0);
		delete data;
		if (image == nullptr) {
			spdlog::critical("Failed to load image {}: {}", fullPath, stbi_failure_reason());
			return;
		}
		handler(image, x, y, channels);
	});
}

void fetch_assimp_scene(std::string root, std::string path, unsigned int postprocessingFlags, std::function<void(std::string, const aiScene*)> handler) {
	std::string fullPath = joinPath(root, path);
	fetch_data(root, path, std::move([postprocessingFlags, fullPath=std::move(fullPath), handler=std::move(handler)](unsigned int size, unsigned char* data) {
		Assimp::Importer loader;
		const aiScene* scene = loader.ReadFileFromMemory(data, size, postprocessingFlags, fullPath.c_str());
		delete data;
		if (!scene) {
			spdlog::critical("Couldn't load model file! {}", loader.GetErrorString());
			return;
		}

		auto index = fullPath.find_last_of("/");
		std::string assetPath = index == -1 ? fullPath : fullPath.substr(0, index);
		handler(assetPath, scene);
	}));
}

#ifdef __EMSCRIPTEN__

struct MyFetchData {
	FetchDataHandler handler;
};

void downloadSucceeded(emscripten_fetch_t* fetch) {
	// Steal the data object
	unsigned char* data = (unsigned char*)fetch->data;
	fetch->data = nullptr;

	static_cast<MyFetchData*>(fetch->userData)->handler(fetch->numBytes, std::move(data));
	
	delete fetch->userData;
	emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t* fetch) {
	spdlog::critical("Downloading {} failed, HTTP failure status code: {}.\n", fetch->url, fetch->status);
	delete fetch->userData;
	emscripten_fetch_close(fetch); // Also free data on failure.
}

void fetch_data(std::string root, std::string path, FetchDataHandler handler) {
	std::string fullPath = joinPath(root, path);

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.userData = new MyFetchData{ handler };
	attr.onsuccess = downloadSucceeded;
	attr.onerror = downloadFailed;
	emscripten_fetch(&attr, fullPath.c_str());
}

#else

void fetch_data(std::string root, std::string path, std::function<void(int, unsigned char*)> handler) {
	std::string fullPath = joinPath(root, path);

	auto ifs = std::ifstream(fullPath, std::ifstream::binary);
	if (ifs.fail()) {
		spdlog::critical("File {} not found!", fullPath);
		return;
	}

	ifs.seekg(0, std::ios::end);
	size_t fileSize = ifs.tellg();
	unsigned char* buffer = new unsigned char[fileSize];
	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)buffer, fileSize);

	if (ifs.eof()) {
		spdlog::critical("From file {}, {} bytes have been read while {} were expected!", fullPath, ifs.gcount(), fileSize);
		return;
	}
	else if (ifs.fail()) {
		spdlog::critical("File {} was unable to be read!", fullPath);
		return;
	}

	handler(fileSize, buffer);
}

#endif
