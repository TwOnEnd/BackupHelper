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
			sendMessage(player, myString(u8"§eSaving．．． from ", playerName));
		});

		if(_access("back\\temp", 0) == -1) {
			_mkdir("back\\temp");
		} else {
			std::filesystem::remove_all("back\\temp");
			Sleep(1000);
			_mkdir("back\\temp");
		}

		clock_t start = clock();
		//Copy Files
		//CleanTempDir();
		filesystem::create_directories("back\\temp\\", ec);
		ec.clear();

		std::filesystem::copy("worlds\\" + worldName,
							  "back\\temp\\" + worldName,
							  std::filesystem::copy_options::recursive, ec);

		if(ec.value() != 0) {
			level->forEachPlayer([&](Player *player) {
				sendMessage(player, u8"§cFailed to copy save files!\n" + ec.message());
			});
			FailEnd(GetLastError());
			return;
		}

		//Truncate
		for(auto &file : files) {
			string toFile = "back\\temp\\" + file.path;

			LARGE_INTEGER pos;
			pos.QuadPart = file.size;
			LARGE_INTEGER curPos;
			HANDLE hSaveFile = CreateFileW(string2wstring(toFile).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

			if(hSaveFile == INVALID_HANDLE_VALUE || !SetFilePointerEx(hSaveFile, pos, &curPos, FILE_BEGIN)
			   || !SetEndOfFile(hSaveFile)) {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"§cFailed to truncate " + toFile + "!");
				});
				FailEnd(GetLastError());
				return;
			}
			CloseHandle(hSaveFile);
		}
		clock_t end = clock();

		std::string backupSize = getBackupSize("back\\temp");
		std::string backupNum = " [" + std::to_string(getAllBackups().size()) + "]";
		std::string note = myString(" [", UTF8ToGBK(_note.c_str()), "]");
		std::string allString = myString(UTF8ToGBK(timeNow()), " [",
										 backupSize, "]", backupNum, " [", playerName, "]", note);
		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;

		makeBackupLog(player, _note, backupSize, takeTime);

		std::string newName = myString("back\\", allString);
		std::filesystem::rename("back\\temp", newName);

		std::string a = myString(timeNow(), "\n",
								 playerName, " make", GBKToUTF8(note),
								 " | Take time: ", takeTime, "s\n-> ", GBKToUTF8(allString));
		const std::string log = a;
		BackupHelperLog(log);

		level->forEachPlayer([&](Player *_player) {
			sendMessage(_player, u8"§e" + a);
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
			sendMessage(player, myString(u8"§cError：no backups"));
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
						myString(u8"§cError：no backups"));
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player,
						myString(u8"§cError：max slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"§eRemoving．．． from ", playerName));
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
			sendMessage(player, u8"§e" + a);
		});

		isWorking = false;
	}


	void BackupHelper::restoreBackup(int slot) {
		isWorking = true;
		std::string playerName = player->getNameTag().c_str();
		std::vector<std::string> backList = getAllBackups();
		if(backList.empty()) {
			sendMessage(player,
						myString(u8"§cError：no backups"));
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player,
						myString(u8"§cError：max slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"§eRestoring．．． from ", playerName));
		});

		std::string restore_path = backList[slot - 1];
		std::string level_name = getLevelName();

		if(_access(myString("worlds\\_" + level_name).c_str(), 0) == -1) {
			_mkdir(myString("worlds\\_" + level_name).c_str());
		} else {
			std::filesystem::remove_all(myString("worlds\\_" + level_name).c_str());
			_mkdir(myString("worlds\\_" + level_name).c_str());
		}
		clock_t start = clock();
		std::filesystem::copy("back\\" + restore_path + "\\" + getLevelName(),
							  "worlds\\_" + level_name,
							  std::filesystem::copy_options::recursive);
		/*std::filesystem::copy("back\\" + restore_path + "\\info.json",
							  "worlds\\_" + level_name);*/
		clock_t end = clock();

		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;
		std::string a = myString(timeNow(), "\n", playerName,
								 " restore [", slot, "] | Take time:", takeTime, "s\n-> ",
								 GBKToUTF8(restore_path));


		const std::string log = a;
		BackupHelperLog(log);

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, u8"§e" + a + "\nPlace wait 5s...");
		});

		Sleep(5000);
		LOCK1 = true;

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
		std::string TwOnEnd = u8"作者:TwOnEnd\n";
		std::string TwOnEnd_QQ = u8"QQ:2445905733\nQQ群:541102114 | 617067009\n";
		std::string TwOnEnd_Email = "Email:2445905733@qq.com\n";
		//std::string c = u8"本插件仅供水布垭野服使用";
		std::string info = myString(TwOnEnd, TwOnEnd_QQ, TwOnEnd_Email/*, c*/);

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, info);
		});
	}


	void BackupHelper::_restore() {
		std::cout << "1";
		std::string level_name = getLevelName();
		std::string ph = "worlds\\" + level_name;
		std::cout << "2";
		std::filesystem::remove_all(ph);
		std::cout << "3";
		std::filesystem::rename("worlds\\_" + level_name, "worlds\\" + level_name);
		std::cout << "4";
		Sleep(1000);
		system("mc_start.bat");
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
#include "pch.h"