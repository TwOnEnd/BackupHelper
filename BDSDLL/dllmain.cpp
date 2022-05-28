#include "pch.h"
#include "mod/BackupHelper/BackupHelper.cpp"

namespace mod {
	THook(void, ChangeSettingCommand_setup, CommandRegistry *_this) {
		_this->registerCommand("backup make [string]", u8"��������", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup remove [int]", u8"ɾ������", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup list [int]", u8"��ѯ����", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup restore [int]", u8"�ص�����", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup info", u8"��Ϣ", 0, { 0 }, { 0x40 });
		_this->registerCommand("backup about", u8"��Ϣ", 0, { 0 }, { 0x40 });
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
		if(cmd == "/backup make " + note) {
			if(mod::isWorking == false) {
				runVanillaCommand("save hold");
			} else {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"��cError:please wait other operation over������");
				});
			}
		} else if(cmd == "/backup remove " + note) {
			if(mod::isWorking == false) {
				std::thread th(&BackupHelper::removeBackup, backuphelper,
							   atoi(note.c_str()));
				th.detach();
			} else {
				level->forEachPlayer([&](Player *player) {
					sendMessage(player, u8"��cError:please wait other operation over������");
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
					sendMessage(player, u8"��cError:please wait other operation over������");
				});
			}
		} else {
			switch(do_hash(cmd)) {
				case do_hash("/backup make"): {
					if(mod::isWorking == false) {
						runVanillaCommand("save hold");
					} else {
						level->forEachPlayer([&](Player *player) {
							sendMessage(player,
										u8"��cError:please wait other operation over������");
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
				default:return original(_this, id, pkt);
					break;
			}
		}
	}
}

void init() {
	// �˴���д�������ʱ�Ĳ���
	std::cout << "[BackupHelper] Loading..." << std::endl;
}

void exit() {
	// �˴���д���ж��ʱ�Ĳ���
	if(mod::LOCK1 == true && mod::isWorking == false) {
		mod::LOCK1 = false;
		mod::BackupHelper backuphelper;
		backuphelper._restore();
	}
}

