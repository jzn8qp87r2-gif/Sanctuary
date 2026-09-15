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
