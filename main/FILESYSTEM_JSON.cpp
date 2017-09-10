#include <stdlib.h>
#include <JSON.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <esp_log.h>
#include <File.h>
#include <FileSystem.h>

static char tag[] = "FILESYSTEM_JSON";
JsonObject FILESYSTEM_GET_JSON_CONTENT(std::string path) {
	// Stat the file to make sure it is there and to determine what kind of a file
	// it is.
	struct stat statBuf;
	int rc = stat(path.c_str(), &statBuf);

	JsonObject obj = JSON::createObject();
	if (rc == -1) {
		ESP_LOGE(tag, "Failed to stat file %s, errno=%s", path.c_str(), strerror(errno));
		obj.setInt("errno", errno);
		return obj;
	}
	obj.setString("path", path);
	std::vector<std::string> parts = FileSystem::pathSplit(path);
	obj.setString("name", parts[parts.size()-1]);
	// Do one thing if the file is a regular file.
	if (S_ISREG(statBuf.st_mode)) {
			obj.setBoolean("directory", false);
			File file(path.c_str());
			std::string data = file.getContent(true);
			obj.setString("data", data);
	}
	// Do a different thing if the file is a directory.
	else if (S_ISDIR(statBuf.st_mode)) {
		obj.setBoolean("directory", true);

	} // End ... found a directory.
	return obj;
} // FILESYSTEM_GET_JSON_CONTENT


/**
 * @brief Get the content of the specified path as a JSON object.
 * @param [in] path The path to access.
 * @return A JSON object containing what was found at the path.
 */
JsonObject FILESYSTEM_GET_JSON_DIRECTORY(std::string path, bool isRecursive) {
	ESP_LOGD(tag, "FILESYSTEM_GET_JSON_DIRECTORY: path is %s", path.c_str());

	// Stat the file to make sure it is there and to determine what kind of a file
	// it is.
	struct stat statBuf;
	int rc = stat(path.c_str(), &statBuf);

	JsonObject obj = JSON::createObject();
	if (rc == -1) {
		ESP_LOGE(tag, "Failed to stat file, errno=%s", strerror(errno));
		obj.setInt("errno", errno);
		return obj;
	}

	obj.setString("path", path);
	std::vector<std::string> parts = FileSystem::pathSplit(path);
	obj.setString("name", parts[parts.size()-1]);

	// Do one thing if the file is a regular file.
	if (S_ISREG(statBuf.st_mode)) {
			obj.setBoolean("directory", false);
	}
	// Do a different thing if the file is a directory.
	else if (S_ISDIR(statBuf.st_mode)) {
		obj.setBoolean("directory", true);
		JsonArray dirArray = JSON::createArray();
		obj.setArray("dir", dirArray);

		ESP_LOGD(tag, "Processing directory: %s", path.c_str());
		std::vector<File> files = FileSystem::getDirectoryContents(path);

		// Now that we have the list of files in the directory, iterator over them adding them
		// to our array of entries.
		for (std::vector<File>::iterator it = files.begin(); it!= files.end(); it++) {
			if (isRecursive) {
				JsonObject dirEntry = FILESYSTEM_GET_JSON_DIRECTORY(path + "/" + (*it).getName(), isRecursive);
				dirArray.addObject(dirEntry);
			} else {
				JsonObject dirEntry = JSON::createObject();
				dirEntry.setString("name", (*it).getName());
				dirEntry.setInt("type", (*it).getType());
				dirArray.addObject(dirEntry);
			}

		} // End ... for each entry in the directory.
	} // End ... found a directory.
	return obj;
} // FILESYSTEM_GET_JSON_DIRECTORY
