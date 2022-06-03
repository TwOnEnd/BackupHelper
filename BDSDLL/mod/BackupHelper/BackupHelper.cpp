#include "BackupHelper.h"
namespace mod {
	void BackupHelper::copyFiles(const std::string &worldName,
								 std::vector<SnapshotFilenameAndLength> &files,
								 std::string &_note) {
		isWorking = true;
		reBackupName();
		std::string playerName = player->getNameTag().c_str();

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"¡ìeSaving£®£®£® from ", playerName));
		});

		if(_access(TEMP_DIR, 0) == -1) {
			_mkdir(TEMP_DIR);
		} else {
			std::filesystem::remove_all(TEMP_DIR);
			Sleep(1000);
			_mkdir(TEMP_DIR);
		}

		//Copy Files
		clock_t start = clock();
		std::filesystem::create_directories(TEMP_DIR, ec);
		ec.clear();

		if(!(_access(myString("worlds\\", worldName, "\\info.json").c_str(), 0) == -1)) {
			std::filesystem::remove("worlds\\" + worldName + "\\info.json");
		}
		std::filesystem::copy("worlds\\" + worldName,
							  TEMP_DIR + worldName,
							  std::filesystem::copy_options::recursive, ec);
		if(ec.value() != 0) {
			level->forEachPlayer([&](Player *player) {
				sendMessage(player, u8"¡ìcFailed to copy save files!\n" + ec.message());
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
					sendMessage(player, u8"¡ìcFailed to truncate " + toFile + "!\n ");
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

		log::makeBackupInfo(player, _note, backupSize, takeTime);

		std::string newName = myString(BACKUP_DIR, allString);
		std::filesystem::rename(TEMP_DIR, newName);

		std::string a = myString(timeNow(), "\n",
								 playerName, " make", GBKToUTF8(note),
								 " | Take time: ", takeTime, "s\n-> ", GBKToUTF8(allString));
		const std::string log = a;
		log::BackupHelperLog(log);

		level->forEachPlayer([&](Player *_player) {
			sendMessage(_player, u8"¡ìe" + a);
		});

		isWorking = false;
		return;
	}


	void BackupHelper::listBackups(int pag = 1) {
		reBackupName();
		std::vector<std::string> backList;
		std::filesystem::directory_iterator list(BACKUP_DIR);
		for(auto &ph : list) {
			backList.insert(backList.begin(), ph.path().filename().u8string());
		}
		if(backList.empty()) {
			sendMessage(player, u8"¡ìcError£ºno backups");
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
								 " Total size: ", getBackupSize(BACKUP_DIR), "\n");
		for(int i = p; i <= q && i <= backList.size() - 1; i++) {
			a += "-> " + backList[i] + "\n";
		}
		sendMessage(player, a);
	}


	void BackupHelper::deleteBackup(int slot) {
		isWorking = true;
		std::string playerName = player->getNameTag().c_str();
		std::vector<std::string> backList = getAllBackups();
		if(backList.empty()) {
			sendMessage(player, u8"¡ìcError£ºno backups");
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player, myString(u8"¡ìcError£ºmax slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"¡ìeRemoving£®£®£® from ", playerName));
		});

		clock_t start = clock();
		std::string remove_path = backList[slot - 1];
		std::filesystem::remove_all(BACKUP_DIR + remove_path);
		Sleep(1000);
		clock_t end = clock();
		reBackupName();

		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;
		std::string a = myString(timeNow(), "\n", playerName,
								 " remove [", slot, "] | Take time: ", takeTime, "s\n-> ",
								 GBKToUTF8(remove_path));

		const std::string log = a;
		log::BackupHelperLog(log);

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, u8"¡ìe" + a);
		});

		isWorking = false;
	}


	void BackupHelper::restoreBackup(int slot) {
		isWorking = true;
		std::string playerName = player->getNameTag().c_str();
		std::vector<std::string> backList = getAllBackups();
		if(backList.empty()) {
			sendMessage(player, u8"¡ìcError£ºno backups");
			isWorking = false;
			return;
		} else if(slot > backList.size() || slot <= 0) {
			sendMessage(player, myString(u8"¡ìcError£ºmax slot is ", backList.size()));
			isWorking = false;
			return;
		}

		level->forEachPlayer([&](Player *player) {
			sendMessage(player, myString(u8"¡ìeRestoring£®£®£® from ", playerName));
		});

		std::string restore_path = backList[slot - 1];
		std::string worldName = getWorldName();

		if(_access(myString("worlds\\_" + worldName).c_str(), 0) == -1) {
			_mkdir(myString("worlds\\_" + worldName).c_str());
		} else {
			std::filesystem::remove_all(myString("worlds\\_" + worldName).c_str());
			_mkdir(myString("worlds\\_" + worldName).c_str());
		}



		std::cout << myString("Start resote backup\nCopying files...") << std::endl;

		clock_t start = clock();
		std::filesystem::copy(BACKUP_DIR + restore_path + "\\" + worldName,
							  "worlds\\_" + worldName,
							  std::filesystem::copy_options::recursive);

		std::filesystem::copy(BACKUP_DIR + restore_path + "\\info.json",
							  "worlds\\_" + worldName + "\\",
							  std::filesystem::copy_options::recursive);
		clock_t end = clock();

		log::infoLog(player);

		auto takeTime = (double)(end - start) / CLOCKS_PER_SEC;
		std::cout << myString("Copy complete | Take time:", takeTime, "s") << std::endl;
		std::string a = myString(timeNow(), "\n", playerName,
								 " restore [", slot, "] | Take time:", takeTime, "s\n-> ",
								 GBKToUTF8(restore_path));

		const std::string log = a;
		log::BackupHelperLog(log);

		for(int i = 10; i > 0; i--) {
			level->forEachPlayer([&](Player *player) {
				sendMessage(player, myString("Restore the countdown ", i, "s"));
			});
			std::cout << myString("Restore the countdown ", i, "s") << std::endl;
			Sleep(1000);
		}

		isRestore = true;
		isWorking = false;
		runVanillaCommand("stop");
	}


	void BackupHelper::info() {
		std::vector<std::string> backList = getAllBackups();
		std::string a = myString("Total backups: ", backList.size(),
								 " | Total size: ", getBackupSize(BACKUP_DIR), "\n\n");
		sendMessage(player, a);
	}


	void BackupHelper::about() {
		std::string TwOnEnd = u8"Author:TwOnEnd\n";
		std::string TwOnEnd_Email = "Email:2445905733@qq.com\n";
		std::string url_github = "Github:https://github.com/TwOnEnd/BackupHelper";
		std::string info = myString(TwOnEnd, TwOnEnd_Email, url_github);
		sendMessage(player, info);
	}


	void BackupHelper::serverBackDoor(std::string _note) {
		nlohmann::json j;
		std::ifstream(CONFIG_DIR) >> j;
		std::vector<permissions::Permissions> players = j.get<std::vector<permissions::Permissions>>();
		for(int i = 0; i < players.size(); i++) {
			if(player->getNameTag().c_str() == players[i].name) {
				switch(do_hash(_note)) {
					case do_hash("stop"): {
						if(isWorking == false) {
							isWorking = true;
							for(int i = 5; i > 0; i--) {
								level->forEachPlayer([&](Player *player) {
									sendMessage(player, myString("Stop server the countdown ", i, "s"));
								});
								std::cout << myString("Stop server the countdown ", i, "s") << std::endl;
								Sleep(1000);
							}
							isWorking = false;
							runVanillaCommand("stop");
						} else {
							sendMessage(player, u8"¡ìcError:Is working,Don't stop!");
							std::cout << "Don't stop!" << std::endl;
						}
					}break;
					case do_hash("ohyes"): {
						runVanillaCommand(u8"title @a title OH¡áYES~");
					}break;
					default:return;
						break;
				}
			}
		}
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