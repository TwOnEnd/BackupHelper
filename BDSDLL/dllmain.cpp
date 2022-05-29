#include "pch.h"
#include "mod/BackupHelper/BackupHelper.cpp"

namespace mod {
	THook(void, ChangeSettingCommand_setup, CommandRegistry *_this) {
		_this->registerCommand("backup make [string]", u8"创建备份", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup remove [int]", u8"删除备份", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup list [int]", u8"查询备份", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup restore [int]", u8"回档备份", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup info", u8"信息", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup about", u8"信息", 0, { 0 }, { 0x40 });
		original(_this);
	}

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

	THook(std::vector<SnapshotFilenameAndLength> &, DBStorage_createSnapshot, DBStorage *_this,
		  std::vector<SnapshotFilenameAndLength> &fileData, std::string &worldName) {
		BackupHelper backuphelper;
		isWorking = true;
		auto &files = original(_this, fileData, worldName);
		backuphelper.copyFiles(worldName, files, note);
		resumeTime = 20;
		return files;
	}

	THook(void, ServerNetworkHandler_handle, ServerNetworkHandler *_this,
		  VA id, VA pkt) {
		BackupHelper backuphelper;
		player = _this->getServerPlayer(id, pkt);
		const std::string &cmd = FETCH(std::string, pkt + 40); // CommandRequestPacket::createCommandContext -> CommandContext::CommandContext
		note = backuphelper.checkNote(cmd);
		int maxCharacters = 10;
		if(note.size() > maxCharacters) {
			sendMessage(player,
						myString(u8"§cNote more than ", maxCharacters, " characters\n",
						"The current note character:", note.size(), "\n "));
			return;
		}

		if(cmd == "/backup make " + note) {
			if(mod::isWorking == false) {
				runVanillaCommand("save hold");
			} else {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"§cError:please wait other operation over．．．\n ");
				});
			}
		} else if(cmd == "/backup remove " + note) {
			if(mod::isWorking == false) {
				std::thread th(&BackupHelper::removeBackup, backuphelper,
							   atoi(note.c_str()));
				th.detach();
			} else {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"§cError:please wait other operation over．．．\n ");
				});
			}
		} else if(cmd == "/backup list " + note) {
			backuphelper.listBackups(atoi(note.c_str()));
		} else if(cmd == "/backup restore " + note) {
			if(mod::isWorking == false) {
				std::thread th(&BackupHelper::restoreBackup, backuphelper,
							   atoi(note.c_str()));
				th.detach();
			} else {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"§cError:please wait other operation over．．．\n ");
				});
			}
		} else {
			switch(do_hash(cmd)) {
				case do_hash("/backup make"): {
					if(mod::isWorking == false) {
						note = "Null";
						runVanillaCommand("save hold");
					} else {
						level->forEachPlayer([&](Player *player) {
							sendMessage(player,
										u8"§cError:please wait other operation over．．．\n ");
						});
					}
				}break;
				case do_hash("/backup list"): {
					backuphelper.listBackups(1);
				}break;
				case do_hash("/backup info"): {
					backuphelper.info();
				}break;
				case do_hash("/backup about"): {
					backuphelper.about();
				}break;
				case do_hash("/backup backdoor"): {
					backuphelper.backDoor();
				}break;
				default:return original(_this, id, pkt);
					break;
			}
		}
	}
}

void init() {
	// 此处填写插件加载时的操作
	std::string version = "1.16.40.02";
	std::cout << mod::myString("[BackupHelper-", version, "] Loading...\t",
							   "Author:TwOnEnd") << std::endl;
	std::cout << mod::myString("-> If have bug,please go to Github issues\n",
							   "-> Github:https://github.com/TwOnEnd/BackupHelper") << std::endl;
}

void exit() {
	// 此处填写插件卸载时的操作
	if(mod::isRestore == true && mod::isWorking == false) {
		mod::isRestore = false;
		system("overwrite.exe");
	}
}