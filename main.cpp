#ifdef _WIN32
#undef UNICODE
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "nlohmann/json.hpp"

const std::string GAMENAME = "Game.exe";
const uint32_t GAME_WIDTH = 800;
const uint32_t GAME_HEIGHT = 600;

enum ScreenLayout
{
	None = 0,
    Tiled = 1,
    Stacked = 2,
    Cascading = 3
};

class D2Window
{
private:
    HANDLE m_hProcess;
    PROCESS_INFORMATION m_pi;
	HWND m_hWnd;
    int m_X;
	int m_Y;
    int m_Width;
	int m_Height;
	
public:
	D2Window(HANDLE hProcess, PROCESS_INFORMATION pi) : m_hProcess(hProcess), m_pi(pi) {}
	~D2Window() { CloseHandle(m_hProcess); }
	
    HANDLE GetProcessHandle() const { return m_hProcess; }
	PROCESS_INFORMATION GetProcessInfo() const { return m_pi; }
	HWND GetWindowHandle() const { return m_hWnd; }
	int GetX() const { return m_X; }
	int GetY() const { return m_Y; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	
	void SetWindowHandle(HWND hWnd) { m_hWnd = hWnd; }	
    void SetX(int x) { m_X = x; }
    void SetY(int y) { m_Y = y; }
    void SetWidth(int width) { m_Width = width; }
    void SetHeight(int height) { m_Height = height; }

    D2Window& operator= (const D2Window& d2window)
    {
        //Copy the necessary info. 
        //Everything else we'll need to grab on the fly
		//when spawning the new process/window.
        m_X = d2window.GetX();
		m_Y = d2window.GetY();
		m_Width = d2window.GetWidth();
		m_Height = d2window.GetHeight();        
		
        return *this;
    }
	
    friend std::ostream& operator<<(std::ostream& out, const D2Window& dw)
    {
        //Not implemented... yet?
        return out;
    }
};

void ConvertScreenLayout(std::string_view str, ScreenLayout& layout)
{
    if (str != "")
    {        
        if (str == "Tiled")
        {
            layout = ScreenLayout::Tiled;
        }
        else if (str == "Stacked")
        {
            layout = ScreenLayout::Stacked;
        }
        else if (str == "Cascading")
        {
            layout = ScreenLayout::Cascading;
        }
    }
}

void GetAllWindowsFromProcessID(DWORD dwProcessID, HWND &vhWnd)
{
    // find all hWnds (vhWnds) associated with a process id (dwProcessID)
    HWND hCurWnd = NULL;
    do
    {
        hCurWnd = FindWindowEx(NULL, hCurWnd, NULL, NULL);
        std::wstring title(GetWindowTextLength(hCurWnd) + 1, L'\0');
        GetWindowTextW(hCurWnd, &title[0], title.size());
        DWORD dwProcID = 0;
        GetWindowThreadProcessId(hCurWnd, &dwProcID);
        if (dwProcID == dwProcessID)
        {
            if (title.find(L"Diablo II") != std::string::npos)
            {
                vhWnd = hCurWnd;
                break;
            }
        }
    }
    while (hCurWnd != NULL);
}

int main(int argc, char* argv[])
{
	std::vector<D2Window*> d2windows;
    std::ifstream f("config.json");
    if(!f.is_open())
    {
        std::cout << "Error opening file" << std::endl;
    }
    nlohmann::json data = nlohmann::json::parse(f);
    std::string game_path = data["Path"];    
    std::cout << "GamePath: " << game_path << std::endl;
    std::string screenCount = data["ScreenCount"];
    std::cout << "ScreenCount: " << screenCount << std::endl;
    ScreenLayout layout;
    ConvertScreenLayout(data["ScreenLayout"], layout);
    std::cout << "ScreenLayout: " << layout << std::endl;

    std::string gameStr = game_path + "/" + GAMENAME;    
    std::wstring gameWStr = std::wstring(gameStr.begin(), gameStr.end());
    std::wcout << gameWStr << std::endl;

    d2windows.reserve((size_t)std::stoi(screenCount));

    for(int i = 0; i < std::stoi(screenCount); i++)
    {
        STARTUPINFO si;     
        PROCESS_INFORMATION pi;
        ZeroMemory( &si, sizeof(STARTUPINFO) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        LPCWSTR game = gameWStr.c_str();
        LPCSTR game2 = gameStr.c_str();
        std::string cmdArgStr = GAMENAME + " -w";
        LPSTR cmdArgs = (LPSTR)cmdArgStr.c_str();
    
        if(!CreateProcess(
            game2,
            cmdArgs,           // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,             // Pointer to STARTUPINFO structure
            &pi             // Pointer to PROCESS_INFORMATION structure
            ))
        {    
			if(GetLastError() == 740)
                std::cout << "Failed to load game. Please elevate privileges. (Run as Admin Dummy :kekw:)" << std::endl;
            return -1;
        }
        else
        {
			std::cout << "Game process created successfully." << std::endl;
			d2windows.push_back(new D2Window(pi.hProcess, pi));
            Sleep(500);
		}
    }
 
    int x = 0;
    int y = 0;
    int screen_width = 0;
	int screen_height = 0;
	int screen_count = 0;
    RECT rect = {0, 0, 0, 0};
    HWND hwnd = NULL;
    HDC mainHDC = GetDC(NULL);

    screen_width = GetDeviceCaps(mainHDC, HORZRES);
    screen_height = GetDeviceCaps(mainHDC, VERTRES);
	
    for(auto& window : d2windows)
    {
        GetAllWindowsFromProcessID(window->GetProcessInfo().dwProcessId, hwnd);
        if (hwnd == NULL)
            continue;
        window->SetWindowHandle(hwnd);
        if (GetWindowRect(window->GetWindowHandle(), &rect))
        {
            window->SetWidth(rect.right - rect.left);
            window->SetHeight(rect.bottom - rect.top);
        }
        
        //There has to be a better way :pepeHands:
        switch (std::stoi(screenCount))
        {
        case 1:
            window->SetX((screen_width) / 2);
            window->SetY((screen_height) / 2);
            break;
        case 2:
			if (screen_count == 0)
			{
				window->SetX((screen_width / 2) - window->GetWidth());
				window->SetY((screen_height / 2) - (window->GetHeight() / 2));
			}
			else if (screen_count == 1)
			{
                window->SetX(screen_width / 2);
				window->SetY((screen_height / 2) - (window->GetHeight() / 2));
			}
            break;
        case 3:
            if (screen_count == 0)
            {
                window->SetX((screen_width / 2) - (window->GetWidth()/2));
                window->SetY((screen_height / 2) - window->GetHeight());
            }
            else if (screen_count == 1)
            {
                window->SetX(screen_width / 2);
                window->SetY(screen_height / 2);
            }
			else if (screen_count == 2)
			{
				window->SetX((screen_width / 2) - window->GetWidth());
				window->SetY(screen_height / 2);
			}
            break;
        case 4:
            if (screen_count == 0)
            {
                window->SetX((screen_width / 2) - window->GetWidth());
                window->SetY((screen_height / 2) - window->GetHeight());
            }
            else if (screen_count == 1)
            {
                window->SetX(screen_width / 2);
                window->SetY((screen_height / 2) - window->GetHeight());
            }
            else if (screen_count == 2)
            {
                window->SetX(screen_width / 2);
                window->SetY(screen_height / 2);
            }
			else if (screen_count == 3)
			{
                window->SetX((screen_width / 2) - window->GetWidth());
                window->SetY(screen_height / 2);
			}
            break;
        default:
            break;
        }
		
        SetWindowPos(hwnd, 0, window->GetX(), window->GetY(), window->GetWidth(), window->GetHeight(), SWP_FRAMECHANGED | SWP_NOSIZE);
        screen_count++;
    }
    
    //Development stuff so we don't spawn too many windows.
    WaitForSingleObject(d2windows[0]->GetProcessHandle(), INFINITE);

    for (auto& window : d2windows)
    {
        delete window;
    }
	
    return 0;
}
