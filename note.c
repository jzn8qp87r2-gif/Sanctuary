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

*/
