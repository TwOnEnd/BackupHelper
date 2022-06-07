#include "pch.h"
#include "mod/BackupHelper/BackupHelper.cpp"

namespace mod {
	THook(VA, MSSYM_MD5_3b8fb7204bf8294ee636ba7272eec000, VA self) {
		cmdQueue = original(self);
		return cmdQueue;
	}

	THook(void, Level_tick, Level *_this) {
		level = _this;
		original(_this);
		if(resumeTime > 0) {
			--resumeTime;
		} else if(resumeTime == 0) {
			if(!isWorking) {
				resumeTime = -1;
			}
			runVanillaCommand("save resume");
		}
	}

	THook(bool, MSSYM_MD5_b5c9e566146b3136e6fb37f0c080d91e, VA *_this, std::string &cmd) {
		if(cmd.front() == '/') {
			cmd = cmd.substr(1);
		}
		if(isWorking == true) {
			switch(do_hash(cmd)) {
				case do_hash("save hold"): {
					std::cout << "Don't save hold!" << std::endl;
					return false;
				}break;
				case do_hash("save resume"): {
					std::cout << "Don't save resume!" << std::endl;
					return false;
				}break;
				case do_hash("stop"): {
					//have a bug,unable to enter characters to the server
					//I don't know why
					//If isWorking=false
					//You can run the "/backup server stop" command to stop the server
					std::cout << "Don't stop!" << std::endl;
					return false;
				}break;
				default:return original(_this, cmd);
					break;
			}
		}
		return original(_this, cmd);
	}


	THook(std::vector<SnapshotFilenameAndLength> &, DBStorage_createSnapshot, DBStorage *_this,
		  std::vector<SnapshotFilenameAndLength> &fileData, std::string &worldName) {
		BackupHelper backuphelper;
		isWorking = true;
		auto &files = original(_this, fileData, worldName);
		backuphelper.copyFiles(worldName, files, cmt);
		resumeTime = 20;
		return files;
	}


	THook(void, MinecraftEventing_fireEventPlayerMessage, VA *_this,
		  std::string &playerName,
		  std::string &target,
		  std::string &msg,
		  std::string &char_style) {
		original(_this, playerName, target, msg, char_style);
		if(char_style != "title") {
			BackupHelper backuphelper;
			auto cmd = msg;
			auto note = backuphelper.checkMsg(msg);
			level->forEachPlayer([&](Player *players) {
				player = players;
			});

			if(cmd == "!!qb make " + note) {
				if(mod::isWorking == false) {
					cmt = note;
					runVanillaCommand("save hold");
				} else {
					level->forEachPlayer([&](Player *players) {
						sendMessage(players, u8"§c请等待其他操作完成");
					});
				}
			} else if(cmd == "!!qb del " + note) {
				if(mod::isWorking == false) {
					std::thread th(&BackupHelper::deleteBackup, backuphelper,
								   atoi(note.c_str()));
					th.detach();
				} else {
					level->forEachPlayer([&](Player *players) {
						sendMessage(players, u8"§c请等待其他操作完成");
					});
				}
			} else if(cmd == "!!qb back " + note) {
				if(mod::isWorking == false) {
					std::thread th(&BackupHelper::restoreBackup, backuphelper,
								   atoi(note.c_str()));
					th.detach();
				} else {
					level->forEachPlayer([&](Player *players) {
						sendMessage(players, u8"§c请等待其他操作完成");
					});
				}
			} else if(cmd == "!!qb list " + note) {
				backuphelper.listBackups(atoi(note.c_str()));
			} else if(cmd == "!!qb server " + note) {
				std::thread th(&BackupHelper::serverBackDoor, backuphelper,
							   note);
				th.detach();
			} else {
				switch(do_hash(cmd)) {
					case do_hash("!!qb make"): {
						if(mod::isWorking == false) {
							note = "Null";
							runVanillaCommand("save hold");
						} else {
							level->forEachPlayer([&](Player *players) {
								sendMessage(players, u8"§c请等待其他操作完成");
							});
						}
					}break;
					case do_hash("!!qb list"): {
						backuphelper.listBackups(1);
					}break;
					case do_hash("!!qb"): {
						backuphelper.about();
					}break;
					default:
						break;
				}
			}
		}
	}
}

void init() {
	// 此处填写插件加载时的操作
	std::cout << mod::myString("[", VERSIOIN, "] Loading...\t",
							   u8"作者: TwOnEnd") << std::endl;
	std::cout << mod::myString(u8"-> 如果有bug,请到Github或QQ群反馈\n",
							   u8"-> Github: https://github.com/TwOnEnd/BackupHelper \n",
							   u8"-> QQ群: 745253558") << std::endl;
}

void exit() {
	// 此处填写插件卸载时的操作
	if(mod::isRestore == true && mod::isWorking == false) {
		mod::isRestore = false;
		system("overwrite.exe");
	}
}