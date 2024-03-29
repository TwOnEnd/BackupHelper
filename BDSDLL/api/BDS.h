#ifndef BDS_H
#define BDS_H
namespace mod {
	struct Actor {
		const std::string& getNameTag() const {
			return SYMCALL(std::string&, Actor_getNameTag, this);
		}
	};
	struct Mob :Actor { };
	struct Player :Actor { };

	struct DBStorage { };
	struct SnapshotFilenameAndLength {
		std::string path;
		size_t size;
	};

	struct Level {
		void forEachPlayer(const std::function<void(Player*)>& todo) {
			auto f = [&](Player& actor) {
				todo(&actor);
				return true;
			};
			this->newForEachPlayer(f);
		}
		void newForEachPlayer(std::function<bool(Player&)> todo) {
			SYMCALL(void (*)(Level*, std::function<bool(Player&)>),
				Level_forEachPlayer, this, todo);
		}
	};

	void sendMessage(Player* player, const std::string& s) {
		std::vector<std::string> v;
		v.emplace_back("test");
		SYMCALL(
			void(*)(bool, Player*, const std::string&, std::vector<std::string> &),
			CommandUtils_displayLocalizableMessage, true, player, s, v);
	}

	VA cmdQueue = 0;
	bool runVanillaCommand(const std::string& command) {
		if (cmdQueue) {
			SYMCALL(bool(*) (VA, std::string),
				MSSYM_MD5_b5c9e566146b3136e6fb37f0c080d91e, cmdQueue, command);
			return true;
		}
		return false;
	}

}
#endif BDS_H
