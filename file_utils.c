/* ============================================================
   parcours_callbacks.c
   Parcours recursif d'une arborescence de fichiers (API Windows)
   avec systeme de callbacks generique.

   Deux callbacks fournis en exemple :
     - CallbackTrouverFichier   : cherche un fichier par son nom
     - CallbackSupprimerFichier : supprime un fichier par son nom
   ============================================================ */

#include <windows.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------
   Le type de callback.
   Il est appele pour CHAQUE entree (fichier ou dossier) trouvee.

   - fullPath : chemin complet de l'entree
   - fd       : infos Windows sur l'entree (nom, attributs...)
   - userData : pointeur libre pour transporter un contexte
                (parametres de recherche, resultats, compteur...)

   Retourne TRUE pour continuer le parcours, FALSE pour l'arreter.
   ------------------------------------------------------------ */
typedef BOOL (*FileCallback)(const char* fullPath,
                              const WIN32_FIND_DATAA* fd,
                              void* userData);

/* ------------------------------------------------------------
   Fonction generique de parcours recursif.
   Appelle "callback" sur chaque fichier/dossier rencontre.
   C'est LA seule fonction qui connait windows.h en detail ;
   les callbacks, eux, ne font que reagir a ce qu'ils recoivent.
   ------------------------------------------------------------ */
BOOL ParcoursRecursif(const char* dossier, FileCallback callback, void* userData)
{
    char pattern[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE hFind;
    BOOL continuer = TRUE;

    snprintf(pattern, MAX_PATH, "%s\\*", dossier);
    hFind = FindFirstFileA(pattern, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return TRUE; /* dossier vide ou inaccessible : on continue le reste */

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char fullPath[MAX_PATH];
        snprintf(fullPath, MAX_PATH, "%s\\%s", dossier, fd.cFileName);

        /* On appelle le callback pour cette entree */
        continuer = callback(fullPath, &fd, userData);
        if (!continuer)
            break;

        /* Si c'est un dossier, on descend dedans */
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continuer = ParcoursRecursif(fullPath, callback, userData);
            if (!continuer)
                break;
        }

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return continuer;
}

/* ============================================================
   CALLBACK 1 : trouver un fichier
   ============================================================ */
typedef struct {
    const char* nomRecherche;
    char cheminTrouve[MAX_PATH];
    BOOL trouve;
} RechercheContext;

BOOL CallbackTrouverFichier(const char* fullPath, const WIN32_FIND_DATAA* fd, void* userData)
{
    RechercheContext* ctx = (RechercheContext*)userData;

    if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        _stricmp(fd->cFileName, ctx->nomRecherche) == 0)
    {
        strcpy(ctx->cheminTrouve, fullPath);
        ctx->trouve = TRUE;
        return FALSE; /* trouve : on arrete tout le parcours */
    }
    return TRUE;
}

/* ============================================================
   CALLBACK 2 : supprimer un fichier
   ============================================================ */
typedef struct {
    const char* nomCible;
    int nbSupprimes;
    BOOL arreterApresPremier; /* TRUE = ne supprime que la 1ere occurrence */
} SuppressionContext;

BOOL CallbackSupprimerFichier(const char* fullPath, const WIN32_FIND_DATAA* fd, void* userData)
{
    SuppressionContext* ctx = (SuppressionContext*)userData;

    if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        _stricmp(fd->cFileName, ctx->nomCible) == 0)
    {
        if (DeleteFileA(fullPath)) {
            printf("Supprime : %s\n", fullPath);
            ctx->nbSupprimes++;
        } else {
            printf("Erreur suppression %s (code %lu)\n", fullPath, GetLastError());
        }

        if (ctx->arreterApresPremier)
            return FALSE;
    }
    return TRUE;
}

/* ============================================================
   DEMO
   ============================================================ */
int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Usage: %s <dossier> <nom_fichier>\n", argv[0]);
        return 1;
    }

    const char* dossier = argv[1];
    const char* nomFichier = argv[2];

    /* --- Recherche --- */
    RechercheContext rctx = { nomFichier, "", FALSE };
    ParcoursRecursif(dossier, CallbackTrouverFichier, &rctx);

    if (rctx.trouve)
        printf("Trouve : %s\n", rctx.cheminTrouve);
    else
        printf("Fichier '%s' introuvable dans %s\n", nomFichier, dossier);

    /* --- Suppression (decommenter pour tester) ---
    SuppressionContext sctx = { nomFichier, 0, FALSE };
    ParcoursRecursif(dossier, CallbackSupprimerFichier, &sctx);
    printf("%d fichier(s) supprime(s).\n", sctx.nbSupprimes);
    */

    return 0;
}


#ifndef MACROS_API_WIN_H
#define MACROS_API_WIN_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/* ================================================================
   MACROS ALLOCATION MEMOIRE (VirtualAlloc / VirtualFree)
   ================================================================ */

/* Allocation lecture/ecriture (donnees classiques) */
#define ALLOCWR(size_byte) \
    VirtualAlloc(NULL, (size_byte), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)

/* Allocation lecture/ecriture/execution (ex: shellcode, JIT, buffers executables) */
#define ALLOCRWX(size_byte) \
    VirtualAlloc(NULL, (size_byte), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)

/* Liberation memoire allouee avec VirtualAlloc (met le pointeur a NULL) */
#define FREEMEM(ptr) \
    do { if (ptr) { VirtualFree((ptr), 0, MEM_RELEASE); (ptr) = NULL; } } while (0)


/* ================================================================
   MACROS HANDLES
   ================================================================ */

/* Fermeture securisee d'un HANDLE : evite les double-close et les handles fantomes */
#define SAFE_CLOSE(h) \
    do { if ((h) != NULL && (h) != INVALID_HANDLE_VALUE) { CloseHandle(h); (h) = NULL; } } while (0)


/* ================================================================
   MACROS VERIFICATION / ERREURS
   ================================================================ */

/* Quitte le programme si la condition est fausse, avec message + code d'erreur Windows */
#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[ERREUR] %s (%s:%d) - code %lu\n", (msg), __FILE__, __LINE__, GetLastError()); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

/* Affiche le message d'erreur Windows lisible correspondant a GetLastError() */
static inline void PrintLastError(const char* contexte)
{
    DWORD err = GetLastError();
    LPSTR msg = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, 0, (LPSTR)&msg, 0, NULL);

    fprintf(stderr, "[%s] Erreur %lu : %s\n", contexte, err, msg ? msg : "(message indisponible)");

    if (msg) LocalFree(msg);
}


/* ================================================================
   MACROS DEBUG
   ================================================================ */

#ifdef _DEBUG
#define DBG(fmt, ...) fprintf(stderr, "[DBG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DBG(fmt, ...) do {} while (0)
#endif


/* ================================================================
   MACROS PROTECTION MEMOIRE (VirtualProtect)
   ================================================================ */

/* Passe une zone memoire en lecture/execution seule
   (typiquement apres y avoir ecrit du code dans une zone RW) */
#define PROTECT_RX(ptr, size, oldProtectVar) \
    VirtualProtect((ptr), (size), PAGE_EXECUTE_READ, &(oldProtectVar))

#define PROTECT_RW(ptr, size, oldProtectVar) \
    VirtualProtect((ptr), (size), PAGE_READWRITE, &(oldProtectVar))


/* ================================================================
   MACROS UTILITAIRES GENERALES
   ================================================================ */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif /* MACROS_API_WIN_H */

/* ============================================================
   processus.c
   Listing et manipulation de processus Windows (API Win32)
   ============================================================ */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

/* ------------------------------------------------------------
   Liste tous les processus en cours avec leur PID
   ------------------------------------------------------------ */
void ListerProcessus(void)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Erreur CreateToolhelp32Snapshot : %lu\n", GetLastError());
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32); /* OBLIGATOIRE avant Process32First, sinon echec garanti */

    if (Process32First(hSnapshot, &pe)) {
        do {
            printf("PID: %-6lu  Threads: %-4lu  Parent: %-6lu  %s\n",
                   pe.th32ProcessID, pe.cntThreads, pe.th32ParentProcessID, pe.szExeFile);
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

/* ------------------------------------------------------------
   Trouve le PID d'un processus a partir de son nom (ex: "notepad.exe")
   Retourne 0 si non trouve.
   ------------------------------------------------------------ */
DWORD TrouverPID(const char* nomProcessus)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    DWORD pid = 0;

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, nomProcessus) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return pid;
}

/* ------------------------------------------------------------
   Recupere le chemin complet de l'executable d'un processus.
   Necessite seulement PROCESS_QUERY_LIMITED_INFORMATION
   (pas besoin de droits eleves juste pour lire le chemin).
   ------------------------------------------------------------ */
BOOL ObtenirCheminProcessus(DWORD pid, char* buffer, DWORD tailleBuffer)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess)
        return FALSE;

    DWORD taille = tailleBuffer;
    BOOL ok = QueryFullProcessImageNameA(hProcess, 0, buffer, &taille);

    CloseHandle(hProcess);
    return ok;
}

/* ------------------------------------------------------------
   Termine un processus par son PID
   ------------------------------------------------------------ */
BOOL TerminerProcessus(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        printf("Impossible d'ouvrir le processus %lu : %lu\n", pid, GetLastError());
        return FALSE;
    }

    BOOL ok = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return ok;
}

/* ------------------------------------------------------------
   Suspend tous les threads d'un processus (pause)
   ------------------------------------------------------------ */
void SuspendreProcessus(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return;

    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
}

/* ------------------------------------------------------------
   Reprend tous les threads d'un processus
   ------------------------------------------------------------ */
void ReprendreProcessus(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return;

    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    ResumeThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
}

/* ============================================================
   DEMO
   ============================================================ */
int main(void)
{
    printf("=== Liste des processus ===\n");
    ListerProcessus();

    DWORD pid = TrouverPID("notepad.exe");
    if (pid) {
        char chemin[MAX_PATH] = {0};
        if (ObtenirCheminProcessus(pid, chemin, MAX_PATH))
            printf("\nnotepad.exe (PID %lu) : %s\n", pid, chemin);

        /* Decommenter pour tester :
        SuspendreProcessus(pid);
        Sleep(2000);
        ReprendreProcessus(pid);
        TerminerProcessus(pid);
        */
    } else {
        printf("\nnotepad.exe non trouve.\n");
    }

    return 0;
}
