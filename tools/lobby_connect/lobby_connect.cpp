/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
*/

#include "steam/steam_api.h"
#include "dll/common_includes.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else

#endif
int main() {
    if (SteamAPI_Init()) {
        //Set appid to: LOBBY_CONNECT_APPID
        SteamAPI_RestartAppIfNecessary(LOBBY_CONNECT_APPID);
        std::cout << "This is a program to find lobbies and run the game with lobby connect parameters" << std::endl;
        std::cout << "Api initialized, ";
top:
        std::cout << "waiting a few seconds for connections:" << std::endl;
        for (int i = 0; i < 10; ++i) {
            SteamAPI_RunCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        int friend_count = SteamFriends()->GetFriendCount(k_EFriendFlagAll);
        std::cout << "People on the network: " << friend_count << std::endl;
        for (int i = 0; i < friend_count; ++i) {
            CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagAll);
            const char *name = SteamFriends()->GetFriendPersonaName(id);

            FriendGameInfo_t friend_info = {};
            SteamFriends()->GetFriendGamePlayed(id, &friend_info);
            std::cout << name << " is playing: " << friend_info.m_gameID.AppID() << std::endl;
        }

        std::cout << std::endl << "--------------Menu-------------" << std::endl << "\tappid\tname\tcommand line" << std::endl;

        std::vector<std::pair<std::string, uint32>> arguments;
        for (int i = 0; i < friend_count; ++i) {
            CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagAll);
            const char *name = SteamFriends()->GetFriendPersonaName(id);
            const char *connect = SteamFriends()->GetFriendRichPresence( id, "connect");
            FriendGameInfo_t friend_info = {};
            SteamFriends()->GetFriendGamePlayed(id, &friend_info);
            auto appid = friend_info.m_gameID.AppID();

            if (strlen(connect) > 0) {
                std::cout << arguments.size() << "\t" << appid << "\t" << name << "\t" << connect << std::endl;
                arguments.emplace_back(connect, appid);
            } else {
                if (friend_info.m_steamIDLobby != k_steamIDNil) {
                    std::string connect = "+connect_lobby " + std::to_string(friend_info.m_steamIDLobby.ConvertToUint64());
                    std::cout << arguments.size() << "\t" << appid << "\t" << name << "\t" << connect << std::endl;
                    arguments.emplace_back(connect, appid);
                }
            }
        }

        std::cout << arguments.size() << ": Retry." << std::endl;
        std::cout << std::endl << "Enter the number corresponding to your choice then press Enter." << std::endl;
        unsigned int choice;
        std::cin >> choice;

        if (choice >= arguments.size()) goto top;

        auto connect = arguments[choice].first;
#ifdef _WIN32
        auto appid = arguments[choice].second;
        std::cout << "starting the game with: " << connect << std::endl;

        char szBaseDirectory[MAX_PATH] = "";
        GetModuleFileNameA(0, szBaseDirectory, MAX_PATH);
        if (auto bs = strrchr(szBaseDirectory, '\\')) {
            *bs = '\0';
        }
        auto lobbyFile = std::string(szBaseDirectory) + "\\lobby_connect_" + std::to_string(appid) + ".txt";

        auto readLobbyFile = [&lobbyFile]() {
            std::string data;
            std::ifstream ifs(lobbyFile);
            if (ifs.is_open())
                std::getline(ifs, data);
            return data;
        };

        auto writeLobbyFile = [&lobbyFile](const std::string& data) {
            std::ofstream ofs(lobbyFile);
            ofs << data << std::endl;
        };

        auto fileExists = [](const std::string& filename) {
            std::ifstream ifs(filename);
            return ifs.is_open();
        };
        
        auto joinLobby = [&connect](std::string filename) {
            filename = "\"" + filename + "\" " + connect;
            std::cout << filename << std::endl;

            STARTUPINFOA lpStartupInfo;
            PROCESS_INFORMATION lpProcessInfo;

            ZeroMemory( &lpStartupInfo, sizeof( lpStartupInfo ) );
            lpStartupInfo.cb = sizeof( lpStartupInfo );
            ZeroMemory( &lpProcessInfo, sizeof( lpProcessInfo ) );

            auto success = !!CreateProcessA( NULL,
                        const_cast<char *>(filename.c_str()), NULL, NULL,
                        NULL, NULL, NULL, NULL,
                        &lpStartupInfo,
                        &lpProcessInfo
                        );

            CloseHandle(lpProcessInfo.hThread);
            CloseHandle(lpProcessInfo.hProcess);
            return success;
        };

        std::string filename = readLobbyFile();
        if (filename.empty() || !fileExists(filename) || !joinLobby(filename)) {
            std::cout << "Please select the game exe" << std::endl;

            OPENFILENAMEA ofn;
            char szFileName[MAX_PATH] = "";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = 0;
            ofn.lpstrFilter = "Exe Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
            ofn.lpstrDefExt = "exe";
            if(GetOpenFileNameA(&ofn) && joinLobby(szFileName)) {
                writeLobbyFile(szFileName);
            }
        }
#else
        std::cout << "Please launch the game with these arguments: " << connect << std::endl;
#endif
    }
}
