#ifndef BACKUPHELPER_H
#define BACKUPHELPER_H
#include "../../pch.h"
#include "../../api/BDS.h"
#include "../../api/tools/config.h"
#include "../../api/tools/nlohmann.h"
#include "../../api/tools/tools.h"

#define VERSIOIN "BackupHelper-0.2.1-1.16.40.02-beta"
namespace mod {
	Player *player = nullptr;
	Level *level = nullptr;
	int resumeTime = -1;
	bool isRestore = false;
	bool isWorking = false;
	std::string cmt = "Null";
	std::error_code ec;

	class BackupHelper {
	public:
		BackupHelper();
		void copyFiles(const std::string &, std::vector<SnapshotFilenameAndLength> &,
					   std::string &);
		void listBackups(int);
		void deleteBackup(int);
		void restoreBackup(int);
		void info();
		void about();

		std::string checkMsg(std::string);
		void serverBackDoor(std::string);
		~BackupHelper();

	private:
		std::string getWorldName();
		std::string getBackupSize(std::string);
		std::vector<std::string> getAllBackups();

		void FailEnd(int);
		void reBackupName();
	};


	BackupHelper::BackupHelper() {
	}


	std::string BackupHelper::getWorldName() {
		Config config("server.properties");
		std::string level_name;
		level_name = config.Read<std::string>("level-name");
		return level_name;
	}

	void BackupHelper::reBackupName() {
		std::string a = "";
		std::vector<std::string> backList = getAllBackups();
		int b = 0, c = 0;
		std::string d = "";
		for(int i = 0; i < backList.size(); i++) {
			a = backList[i];
			b = a.find("[", 28);
			c = a.find("]", b);
			d = backList[i];
			d.replace(b + 1, c - b - 1, std::to_string(i + 1));
			std::filesystem::rename(BACKUP_DIR + backList[i], BACKUP_DIR + d);
		}
	}

	std::string BackupHelper::getBackupSize(std::string _path) {
		float fsize = 0;
		std::string ssize;
		std::vector<std::string> file_name;
		std::string path(_path);
		getFileNames(path, file_name);
		for(const auto &ph : file_name) {
			fsize += std::filesystem::file_size(ph);
		}
		fsize = fsize / 1048576;
		ssize = fsize < 1024 ?
			std::to_string(fsize) + "MB" :
			std::to_string(fsize / 1024) + "GB";
		ssize.replace(ssize.find(u8".") + 3, 4, "");
		return ssize;
	}


	std::vector<std::string> BackupHelper::getAllBackups() {
		std::vector<std::string> fileName;
		std::filesystem::directory_iterator backList(BACKUP_DIR);
		for(auto &ph : backList) {
			fileName.push_back(ph.path().filename().string());
		}
		return fileName;
	}

	void BackupHelper::FailEnd(int code = -1) {
		level->forEachPlayer([&](Player *player) {
			sendMessage(player,
						std::string(u8"§cBackup Fail") + (code == -1 ? "" :
						u8"§cError code：" + std::to_string(code)));
		});
		player = nullptr;
		isWorking = false;
		cmt = "Null";
		runVanillaCommand("save resume");
	}




	/*std::string BackupHelper::info(Player *p) {

		std::cout << "" << 6G << std::endl;
		std::cout << u8"共 114514 个备份 | 占用 6G\n";

		std::cout << u8"上次创建备份信息 | TwOnEnd\n";
		std::cout << "[time][size][slot][player][note]\n";

		std::cout << u8"上次删除备份信息 | TwOnEnd\n";
		std::cout << "[time][size][slot][player][note]\n";

		std::cout << u8"上次回档备份信息 | TwOnEnd\n";
		std::cout << "[time][size][slot][player][note]\n";
		return "";
	}*/

	BackupHelper::~BackupHelper() {
	}
}
#endif // !BACKUPHELPER_H
