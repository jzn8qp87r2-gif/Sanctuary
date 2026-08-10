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

/*
    FONCTION : ReplaceString
    DESCRIPTION : Fonction qui permet de remplacer une chaîne de caractères par une autre dans un fichier texte

#include <stdio.h>
#include <windows.h> 

// =====================================================================
// 1. FONCTIONS DE BASE (Version avec boucles FOR)
// =====================================================================

int my_strlen(const char *str) {
    int len;
    // La boucle for fait tout le travail dans sa déclaration !
    // Initialisation ; Condition de maintien ; Incrémentation
    for (len = 0; str[len] != '\0'; len++) {
        // Le corps est vide, on se contente d'incrémenter 'len'
    }
    return len;
}

int my_strncmp(const char *s1, const char *s2, int n) {
    // On compare les 'n' premiers caractères
    for (int i = 0; i < n; i++) {
        // Si on trouve une différence ou si on atteint la fin d'une chaîne
        if (s1[i] != s2[i] || s1[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0; // Identique
}

const char *my_strstr(const char *haystack, const char *needle) {
    if (needle[0] == '\0') return haystack;

    // On parcourt le texte avec l'index 'i'
    for (int i = 0; haystack[i] != '\0'; i++) {
        int j;
        
        // On vérifie si le mot correspond à partir de la position 'i'
        // haystack[i + j] permet de regarder devant sans modifier 'i'
        for (j = 0; needle[j] != '\0' && haystack[i + j] == needle[j]; j++) {
            // Les caractères correspondent, on continue de vérifier
        }

        // Si on a parcouru tout 'needle', on l'a trouvé !
        if (needle[j] == '\0') {
            return &haystack[i]; // On retourne l'adresse mémoire à la position 'i'
        }
    }
    return NULL;
}

void my_strcpy(char *dest, const char *src) {
    int i;
    // On copie la source dans la destination
    for (i = 0; src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0'; // On termine la chaîne
}

// =====================================================================
// 2. LA FONCTION DE REMPLACEMENT
// =====================================================================

char *replace_str(const char *input, const char *pattern, const char *replacement) {
    if (!input || !pattern || !replacement) return NULL;

    int input_len = my_strlen(input);
    int pattern_len = my_strlen(pattern);
    int replacement_len = my_strlen(replacement);

    if (pattern_len == 0) {
        char *copy = (char *)VirtualAlloc(NULL, input_len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (copy) my_strcpy(copy, input);
        return copy;
    }

    // --- ÉTAPE 1 : Compter ---
    int count = 0;
    // Le for est très pratique ici : on initialise 'tmp', on boucle tant qu'il trouve 
    // le mot, et à chaque tour, on l'avance de la taille du mot (pattern_len).
    for (const char *tmp = input; (tmp = my_strstr(tmp, pattern)) != NULL; tmp += pattern_len) {
        count++;
    }

    // --- ÉTAPE 2 : Allouer ---
    int new_len = input_len + count * (replacement_len - pattern_len);
    char *result = (char *)VirtualAlloc(NULL, new_len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!result) return NULL;

    // --- ÉTAPE 3 : Remplacer et copier ---
    int dest_idx = 0; // Index pour écrire dans le résultat

    // On parcourt l'input avec l'index 'i'.
    // Note : On n'incrémente pas 'i' dans la définition du for (i++ est absent), 
    // car on l'incrémentera manuellement selon ce qu'on trouve.
    for (int i = 0; input[i] != '\0'; ) {
        
        // Si on trouve le mot à la position 'i'
        if (my_strncmp(&input[i], pattern, pattern_len) == 0) {
            
            // On écrit le mot de remplacement dans le résultat
            for (int j = 0; replacement[j] != '\0'; j++) {
                result[dest_idx] = replacement[j];
                dest_idx++;
            }
            i += pattern_len; // On fait sauter 'i' de la taille de l'ancien mot
            
        } else {
            // Sinon, on copie juste la lettre normale
            result[dest_idx] = input[i];
            dest_idx++;
            i++;
        }
    }
    
    result[dest_idx] = '\0'; // Fin de chaîne
    return result;
}

// =====================================================================
// 3. PROGRAMME PRINCIPAL
// =====================================================================

int main() {
    const char *texte = "Le petit chat boit du lait. Le chat est heureux.";
    const char *recherche = "chat";
    const char *remplacement = "chien";

    char *nouveau_texte = replace_str(texte, recherche, remplacement);

    if (nouveau_texte) {
        printf("Avant : %s\n", texte);
        printf("Apres : %s\n", nouveau_texte);
        
        VirtualFree(nouveau_texte, 0, MEM_RELEASE);
    }

    return 0;
}

*/
