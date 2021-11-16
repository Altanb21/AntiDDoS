#include <stdio.h>
#include <string>
#include <stdexcept>
#include <thread>
#include <vector>
#include <algorithm>

#define MAX_HIT_THRESHOLD_CONSTANT 298

std::vector<std::string> explode(const std::string& str, const char ch, bool allowEmpty = true)
{
	std::string next;
	std::vector<std::string> result;
	if (str.find(ch) == std::string::npos) {
		result.push_back(str);
		return result;
	} // check if it even exists lol, if not return result instantly

	for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
		try {
			if (*it == ch) {
				if (!next.empty() || allowEmpty) {
					result.push_back(next);
					next.clear();
				}
			}
			else {
				next += *it;
			}
		}
		catch (...) { continue; }
	}
	if (!next.empty())
		result.push_back(next);

	return result;
}

// LINUX ONLY!
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    try {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL)
            result += buffer;
    }
    catch (...) {}

    pclose(pipe);
    return result;
}

bool StringToUInt32Safe(const std::string& str, uint32_t* output)
{
	try {
		*output = std::stoul(str);
		return true;
	}
	catch (...) {}
	return false;
}

std::string CleanString(const std::string& s)
{
	size_t offs = 0;
	for (char c : s)
	{
		if (c != ' ')
			break;

		offs++;
	}
	return s.substr(offs);
}

int main()
{
    while (1)
    {
		printf("(Sanitizing traffic...)\n");

		const auto stuff = explode(exec("netstat -ntu | awk '{print $5}' | cut -d: -f1 | sort | uniq -c | sort -n"), '\n');
		for (const auto& tk : stuff)
		{
			auto data = CleanString(tk);

			auto entry = explode(data, ' ');
			if (entry.size() < 2)
				continue;

			uint32_t hits;
			if (!StringToUInt32Safe(entry[0], &hits))
				continue;

			if (hits >= MAX_HIT_THRESHOLD_CONSTANT)
			{
				// block the IP:
				auto ip = entry[1];
				printf("DDoS DETECTED FROM IP: '%s'\n", ip.c_str());

				auto res = exec(std::string("ip route add blackhole " + ip).c_str());

				//printf(res == "" ? "=> Successfully blocked." : res.c_str());
			}
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}