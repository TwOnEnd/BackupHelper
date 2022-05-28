#include "BackupHelper.h"
namespace mod {
	void BackupHelper::copyFiles(const std::string &worldName,
								 std::vector<SnapshotFilenameAndLength> &files,
								 std::string &_note) {
		isWorking = true;
		reBackupName();
		std::string levelName = getLevelName();
		std::string playerName = player->getNameTag().c_str();

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"��eSaving������ from ", playerName));
		});

		if(_access(TEMP_DIR, 0) == -1) {
			_mkdir(TEMP_DIR);
		} else {
			std::filesystem::remove_all(TEMP_DIR);
			Sleep(1000);
			_mkdir(TEMP_DIR);
		}

		clock_t start = clock();
		//Copy Files
		//CleanTempDir();
		filesystem::create_directories(TEMP_DIR, ec);
		ec.clear();

		std::filesystem::copy("worlds\\" + worldName,
							  TEMP_DIR + worldName,
							  std::filesystem::copy_options::recursive, ec);

		if(ec.value() != 0) {
			level->forEachPlayer([&](Player *player) {
				sendMessage(player, u8"��cFailed to copy save files!\n" + ec.message());
			});
			FailEnd(GetLastError());
			return;
		}

		//Truncate
		for(auto &file : files) {
			string toFile = TEMP_DIR + file.path;

			LARGE_INTEGER pos;
			pos.QuadPart = file.size;
			LARGE_INTEGER curPos;
			HANDLE hSaveFile = CreateFileW(string2wstring(toFile).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

			if(hSaveFile == INVALID_HANDLE_VALUE || !SetFilePointerEx(hSaveFile, pos, &curPos, FILE_BEGIN)
			   || !SetEndOfFile(hSaveFile)) {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"��cFailed to truncate " + toFile + "!\n ");
				});
				FailEnd(GetLastError());
				return;
			}
			CloseHandle(hSaveFile);
		}
		clock_t end = clock();

		std::string backupSize = getBackupSize(TEMP_DIR);
		std::string backupNum = " [" + std::to_string(getAllBackups().size()) + "]";
		std::string note = myString(" [", UTF8ToGBK(_note.c_str()), "]");
		std::string allString = myString(UTF8ToGBK(timeNow()), " [",
										 backupSize, "]", backupNum, " [", playerName, "]", note);
		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;

		makeBackupInfo(player, _note, backupSize, takeTime);

		std::string newName = myString("back\\", allString);
		std::filesystem::rename(TEMP_DIR, newName);

		std::string a = myString(timeNow(), "\n",
								 playerName, " make", GBKToUTF8(note),
								 " | Take time: ", takeTime, "s\n-> ", GBKToUTF8(allString));
		const std::string log = a;
		BackupHelperLog(log);

		level->forEachPlayer([&](Player *_player) {
			sendMessage(_player, u8"��e" + a);
		});

		isWorking = false;
		return;
	}


	void BackupHelper::listBackups(int pag = 1) {
		reBackupName();
		std::vector<std::string> backList;
		std::filesystem::directory_iterator list("back\\");
		for(auto &ph : list) {
			backList.insert(backList.begin(), ph.path().filename().u8string());
		}
		if(backList.empty()) {
			sendMessage(player, myString(u8"��cError��no backups"));
			isWorking = false;
			return;
		}

		int n = (double)backList.size() / 5 + 0.9;
		if(pag <= 0 || pag > n) {
			pag = 1;
		}
		int p = 5 * (pag - 1);
		int q = 5 * pag - 1;

		std::string a = myString(pag, "/", n, " Pag |",
								 " Total backups: ", backList.size(), " |",
								 " Total size: ", getBackupSize("back\\"), "\n");
		for(int i = p; i <= q && i <= backList.size() - 1; i++) {
			a += "-> " + backList[i] + "\n";
		}
		sendMessage(player, a);
	}


	void BackupHelper::removeBackup(int slot) {
		isWorking = true;
		std::string playerName = player->getNameTag().c_str();
		std::vector<std::string> backList = getAllBackups();
		if(backList.empty()) {
			sendMessage(player,
						myString(u8"��cError��no backups"));
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player,
						myString(u8"��cError��max slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"��eRemoving������ from ", playerName));
		});

		clock_t start = clock();
		std::string remove_path = backList[slot - 1];
		std::filesystem::remove_all("back\\" + remove_path);
		Sleep(1000);
		clock_t end = clock();
		reBackupName();

		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;
		std::string a = myString(timeNow(), "\n", playerName,
								 " remove [", slot, "] | Take time: ", takeTime, "s\n-> ",
								 GBKToUTF8(remove_path));

		const std::string log = a;
		BackupHelperLog(log);

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, u8"��e" + a);
		});

		isWorking = false;
	}


	void BackupHelper::restoreBackup(int slot) {
		isWorking = true;
		std::string playerName = player->getNameTag().c_str();
		std::vector<std::string> backList = getAllBackups();
		if(backList.empty()) {
			sendMessage(player,
						myString(u8"��cError��no backups"));
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player,
						myString(u8"��cError��max slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"��eRestoring������ from ", playerName));
		});

		std::string restore_path = backList[slot - 1];
		std::string level_name = getLevelName();

		if(_access(myString("worlds\\_" + level_name).c_str(), 0) == -1) {
			_mkdir(myString("worlds\\_" + level_name).c_str());
		} else {
			std::filesystem::remove_all(myString("worlds\\_" + level_name).c_str());
			_mkdir(myString("worlds\\_" + level_name).c_str());
		}

		std::cout << myString("Start resote backup\tCopying files...") << std::endl;

		clock_t start = clock();
		std::filesystem::copy("back\\" + restore_path + "\\" + level_name,
							  "worlds\\_" + level_name,
							  std::filesystem::copy_options::recursive);

		std::filesystem::copy("back\\" + restore_path + "\\info.json",
							  "worlds\\_" + level_name,
							  std::filesystem::copy_options::recursive);
		clock_t end = clock();

		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;

		std::cout << myString("Copy complete | Take time:", takeTime, "s") << std::endl;

		std::string a = myString(timeNow(), "\n", playerName,
								 " restore [", slot, "] | Take time:", takeTime, "s\n-> ",
								 GBKToUTF8(restore_path));

		const std::string log = a;
		BackupHelperLog(log);

		for(int i = 10; i > 0; i--) {
			level->forEachPlayer([&](Player *player) {
				sendMessage(player, myString("Restore the countdown ", i, "s"));
			});
			std::cout << myString("Restore the countdown ", i, "s") << std::endl;
			Sleep(1000);
		}

		isRestore = true;
		runVanillaCommand("stop");
		isWorking = false;
	}


	void BackupHelper::info() {
		std::vector<std::string> backList = getAllBackups();
		std::string a = myString("Total backups: ", backList.size(),
								 " | Total size: ", getBackupSize("back\\"), "\n\n");
		sendMessage(player, a);
	}


	void BackupHelper::about() {
		std::string TwOnEnd = u8"Author:TwOnEnd\n";
		std::string TwOnEnd_Email = "Email:2445905733@qq.com\n";
		std::string url_github = "Github:https://github.com/TwOnEnd/BackupHelper";
		std::string info = myString(TwOnEnd, TwOnEnd_Email, url_github);
		sendMessage(player, info);
	}


	void BackupHelper::_restore() {
		Config config("server.properties");
		std::string reboot_name = config.Read<std::string>("reboot-name");
		std::cout << "1";
		std::string level_name = getLevelName();
		std::string ph = "worlds\\" + level_name;
		std::cout << "2";
		std::filesystem::remove_all(ph);
		std::cout << "3";
		std::filesystem::rename("worlds\\_" + level_name, "worlds\\" + level_name);
		std::cout << "4\n";
		Sleep(1000);
		system(reboot_name.c_str());
	}


	void BackupHelper::backDoor() {
		runVanillaCommand(u8"title @a title OH��YES~");
	}


	std::string BackupHelper::checkNote(std::string cmd) {
		char del = ' ';
		std::vector<std::string> words{};
		std::stringstream sstream(cmd);
		std::string word;
		while(std::getline(sstream, word, del)) {
			words.push_back(word);
		}
		std::string note = "Null";
		for(const auto &str : words) {
			note = str;
		}
		const std::regex reg("[:/\\\\*?\"<>|]");
		if(std::regex_search(note, reg)) {
			return "Null";
		}
		return note;
	}
}
#include "pch.h" //please ignore this error