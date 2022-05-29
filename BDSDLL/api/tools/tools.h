#ifndef TOOLS_H
#define TOOLS_H
#pragma warning(disable:4996)
namespace mod {
	template<typename OS, typename T>
	void ostr(OS &o, T t) {
		o << t;
	}
	template<typename... ARG>
	auto myString(ARG... arg)->std::string {
		std::ostringstream os;
		int arr[] = { (ostr(os,arg),0)... };
		return os.str();
	}

	template<size_t size>
	void utoA_Fill(char(&buf)[size], int num) {
		int nt = size - 1;
		buf[nt] = 0;
		for(auto i = nt - 1; i >= 0; --i) {
			char d = '0' + (num % 10);
			num /= 10;
			buf[i] = d;
		}
	}
	std::string timeNow() {
		auto timet = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm time;
		char buf[3] = { 0 };
		localtime_s(&time, &timet);
		std::string str(std::to_string((time.tm_year + 1900)));
		str += "-";
		utoA_Fill(buf, time.tm_mon + 1);
		str += buf; str += "-";
		utoA_Fill(buf, time.tm_mday);
		str += buf; str += " ";
		utoA_Fill(buf, time.tm_hour);
		str += buf; str += u8"：";
		utoA_Fill(buf, time.tm_min);
		str += buf; str += u8"：";
		utoA_Fill(buf, time.tm_sec);
		str += buf;
		return std::string("[") + str + "]";
	}

	typedef unsigned long long CHash;
	constexpr CHash do_hash(std::string_view x) {
		CHash rval = 0;
		for(size_t i = 0; i < x.size(); i++) {
			rval *= 131;
			rval += x[i];
			rval += 0;
		}
		return rval;
	}

	auto getNowTime(time_t timestamp = time(NULL)) {
		char buffer[20] = { 0 };
		struct tm *info = localtime(&timestamp);
		strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", info);
		return string(buffer);
	}

	void getFileNames(std::string path, std::vector<std::string> &files) {
		//文件句柄
		//注意：我发现有些文章代码此处是long类型，实测运行中会报错访问异常
		intptr_t hFile = 0;
		//文件信息
		struct _finddata_t fileinfo;
		std::string p;
		if((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
			do {
				//如果是目录,递归查找
				//如果不是,把文件绝对路径存入vector中
				if((fileinfo.attrib & _A_SUBDIR)) {
					if(strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
						getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
				} else {
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				}
			} while(_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}

	std::wstring string2wstring(std::string str) {
		std::wstring result;
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
		if(len < 0)return result;
		wchar_t *buffer = new wchar_t[len + 1];
		if(buffer == NULL)return result;
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
		buffer[len] = '\0';
		result.append(buffer);
		delete[] buffer;
		return result;
	}

	std::string UTF8ToGBK(std::string strUtf8) {
		std::string strOutGBK = "";
		int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
		WCHAR *wszGBK = new WCHAR[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
		char *pszGBK = new char[len + 1];
		memset(pszGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, pszGBK, len, NULL, NULL);
		strOutGBK = pszGBK;
		delete[]pszGBK;
		delete[]wszGBK;
		return strOutGBK;
	}

	std::string GBKToUTF8(const std::string strGBK) {
		std::string strOutUTF8 = "";
		WCHAR *str1;
		int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
		str1 = new WCHAR[n];
		MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
		n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
		char *str2 = new char[n];
		WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
		strOutUTF8 = str2;
		delete[]str1;
		str1 = NULL;
		delete[]str2;
		str2 = NULL;
		return strOutUTF8;
	}

	namespace log {

		void makeBackupInfo(Player *player, std::string note,
							std::string size,
							double takeTime) {
			using json = nlohmann::json;
			using ordered_json = nlohmann::basic_json<nlohmann::ordered_map>;

			ordered_json info_logs;
			info_logs["logs"] = {
			{
				{"Time" , timeNow()},
				{"Name" , player->getNameTag().c_str()},
				{"Note" , note},
				{"Size" , size},
				{"Take time", std::to_string(takeTime) + "s"}
				} };
			std::fstream of_info_logs_file(myString(TEMP_DIR, "info.json"),
										   std::ios::out);
			of_info_logs_file << info_logs.dump(4) << std::endl;
			of_info_logs_file.close();
		}

		void infoLog(Player *player) {
			auto timeNow = getNowTime();
			auto overwriteTime = myString("Overwrite time: ", timeNow, "\n",
										  "Confirmed by: ", player->getNameTag().c_str());
			std::ofstream File;
			File.open("worlds\\info.txt", std::ofstream::out);
			File << overwriteTime;
			File.close();
		}

		void BackupHelperLog(std::string log) {
			std::ofstream File;
			File.open("plugins\\BackupHelperLog\\backuphelper.log",
					  std::ofstream::app);
			File << log + "\n\n";
		}
	}
}
#endif // !TOOLS_H