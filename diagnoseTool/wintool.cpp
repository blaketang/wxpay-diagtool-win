#include "wintool.h"
#include <windows.h>
#include <winbase.h>
#include <psapi.h>
#include <winver.h>
#include <QSysInfo>
#include <QEventLoop>
#include <QTimer>

QString WinTool::GetFileVersion(const QString &filepath)
{
    TCHAR fileName[_MAX_PATH];
    filepath.toWCharArray(fileName);
    fileName[filepath.size()] = 0;
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSize(fileName, &handle);
    if(size < 256)
        size = 256;
    BYTE versionInfo[size];
    if (!GetFileVersionInfo(fileName, handle, size, versionInfo))
    {
        return QString();
    }
    // we have version information
    UINT    			len = 0;
    VS_FIXEDFILEINFO*   vsfi = NULL;
    VerQueryValue(versionInfo, TEXT("\\"), (void**)&vsfi, &len);
    USHORT aVersion[4];
    aVersion[0] = HIWORD(vsfi->dwFileVersionMS);
    aVersion[1] = LOWORD(vsfi->dwFileVersionMS);
    aVersion[2] = HIWORD(vsfi->dwFileVersionLS);
    aVersion[3] = LOWORD(vsfi->dwFileVersionLS);
    return QString().sprintf("%d.%d.%d.%d", aVersion[0], aVersion[1], aVersion[2] , aVersion[3]);
}
