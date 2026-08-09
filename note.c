/*
Note d'utilisation des fonctions Windows API pour la lecture, l'écriture de fichiers, et allocation de mémoire :

    CreateFileA(
        LPCSTR                lpFileName,
        DWORD                 dwDesiredAccess,
        DWORD                 dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD                 dwCreationDisposition,
        DWORD                 dwFlagsAndAttributes,
        HANDLE                hTemplateFile
    );
    Pour ouvrir un fichier en lecture, on utilise dwDesiredAccess = GENERIC_READ et dwCreationDisposition = OPEN_EXISTING.
    hFile = CreateFileA(nomFichier, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    Pour ouvrir un fichier en écriture, on utilise dwDesiredAccess = GENERIC_WRITE et dwCreationDisposition = CREATE_ALWAYS.
    hFile = CreateFileA(nomFichier, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    virtualalloc(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD  flAllocationType,
        DWORD  flProtect
    );
    Option par défaut : lpAddress = NULL, flAllocationType = MEM_COMMIT | MEM_RESERVE, flProtect = PAGE_READWRITE

    ReadFile(
        HANDLE       hFile,
        LPVOID       lpBuffer,
        DWORD        nNumberOfBytesToRead,
        LPDWORD      lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped
    );
    Pour lire un fichier, on utilise lpBuffer = buffer (données en sortie), nNumberOfBytesToRead = taille et lpOverlapped = NULL.

    WriteFile(
        HANDLE       hFile,
        LPCVOID      lpBuffer,
        DWORD        nNumberOfBytesToWrite,
        LPDWORD      lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped
    );
    Pour ecrire dans un fichier, on utilise lpBuffer = buffer (données en entrée), nNumberOfBytesToWrite = taille et lpOverlapped = NULL.

    Pour effectuer une recherche dans un fichier ini
    DWORD GetPrivateProfileStringA(
          LPCSTR lpAppName,        // Nom de la section (ex: "[Config]")
          LPCSTR lpKeyName,        // Nom de la clé (ex: "TargetIP")
          LPCSTR lpDefault,        // Valeur par défaut si la clé est introuvable
          LPSTR  lpReturnedString, // Buffer qui va recevoir le résultat
          DWORD  nSize,            // Taille de ton buffer
          LPCSTR lpFileName        // Chemin complet vers le fichier .ini
    );

    Pour effectuer un recherche de fichier/répertoire
    HANDLE FindFirstFileA(
          LPCSTR             lpFileName,     // Le chemin avec un wildcard (ex: "C:\\Temp\\*")
          LPWIN32_FIND_DATAA lpFindFileData  // Structure WIN32_FIND_DATAA qui recevra les infos du premier fichier/dossier trouvé
    );

    A utiliser dans une boucle
    BOOL FindNextFileA(
          HANDLE             hFindFile,      // Le handle retourné par FindFirstFileA
          LPWIN32_FIND_DATAA lpFindFileData  // La même structure qui sera mise à jour
);

    Fonction pour ouvrir un processus Windows
    HANDLE OpenProcess(
          DWORD dwDesiredAccess, // Les droits voulus (ex: PROCESS_ALL_ACCESS, PROCESS_TERMINATE, PROCESS_VM_READ)
          BOOL  bInheritHandle,  // Généralement FALSE
          DWORD dwProcessId      // Le PID cible
    );

    Fonction pour effectuer un snapshot de l'état du systeme
    HANDLE CreateToolhelp32Snapshot(
          DWORD dwFlags,       // TH32CS_SNAPPROCESS pour lister les processus
          DWORD th32ProcessID  // 0 pour lister tous les processus du système
    );

    BOOL Process32First(
          HANDLE           hSnapshot, // Le handle du snapshot
          LPPROCESSENTRY32 lppe       // Structure PROCESSENTRY32 qui recevra les infos du processus (PID, nom de l'exe, etc.)
    );

    Chercher une ressource dans un binaire
    HRSRC FindResourceA(
          HMODULE hModule, // NULL pour chercher dans le binaire actuel, ou un handle retourné par LoadLibrary
          LPCSTR  lpName,  // ID de la ressource (souvent via MAKEINTRESOURCE)
          LPCSTR  lpType   // Type (ex: RT_RCDATA pour de la donnée brute, RT_BITMAP, etc.)
    );
*/
