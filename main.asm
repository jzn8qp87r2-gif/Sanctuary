; MASM x86 - wrappers simples autour de VirtualAlloc/VirtualFree

.386
.model flat, stdcall
option casemap:none

includelib kernel32.lib

EXTERN __imp__VirtualAlloc@16:DWORD
EXTERN __imp__VirtualFree@12:DWORD

MEM_COMMIT     equ 1000h
MEM_RESERVE    equ 2000h
MEM_RELEASE    equ 8000h
PAGE_READWRITE equ 04h

; pResult recoit l'adresse de la zone allouee.
VirtualAllocMacro MACRO pResult, dwSize, flAllocationType, flProtect
	push flProtect
	push flAllocationType
	push dwSize
	push 0
	call DWORD PTR [__imp__VirtualAlloc@16]
	mov pResult, eax
ENDM

; Retourne zero dans pResult en cas d'echec.
VirtualFreeMacro MACRO pResult, pAddress
	push MEM_RELEASE
	push 0
	push pAddress
	call DWORD PTR [__imp__VirtualFree@12]
	mov pResult, eax
ENDM

.data
	pMemory DWORD ?
	freeOk  DWORD ?

.code
main PROC
	VirtualAllocMacro pMemory, 1000h, MEM_RESERVE or MEM_COMMIT, PAGE_READWRITE
	test pMemory, pMemory
	jz allocation_failed

	; Utiliser la memoire via pMemory ici.

	VirtualFreeMacro freeOk, pMemory

allocation_failed:
	ret
main ENDP

END main
; MASM x86 - parcours recursif d'un dossier avec les API Win32

.386
.model flat, stdcall
option casemap:none

includelib kernel32.lib

EXTERN __imp__FindFirstFileA@8:DWORD
EXTERN __imp__FindNextFileA@8:DWORD
EXTERN __imp__FindClose@4:DWORD
EXTERN __imp__GetStdHandle@4:DWORD
EXTERN __imp__WriteFile@20:DWORD
EXTERN __imp__lstrcpyA@8:DWORD
EXTERN __imp__lstrcatA@8:DWORD
EXTERN __imp__ExitProcess@4:DWORD

INVALID_HANDLE_VALUE equ -1
STD_OUTPUT_HANDLE    equ -11
FILE_ATTRIBUTE_DIRECTORY equ 10h

; Offsets utiles dans WIN32_FIND_DATAA.
FIND_DATA_SIZE       equ 320
FD_ATTRIBUTES        equ 0
FD_FILE_NAME         equ 44

; Appel indirect d'une API stdcall importee.
ApiCall MACRO pFunction, argCount
	call DWORD PTR [pFunction]
ENDM

; Les arguments doivent etre empiles de droite a gauche avant ApiCall.
FindFirstFileMacro MACRO pPattern, pFindData
	push pFindData
	push pPattern
	ApiCall __imp__FindFirstFileA@8, 2
ENDM

FindNextFileMacro MACRO hFind, pFindData
	push pFindData
	push hFind
	ApiCall __imp__FindNextFileA@8, 2
ENDM

FindCloseMacro MACRO hFind
	push hFind
	ApiCall __imp__FindClose@4, 1
ENDM

.data
	rootPath  db 'C:', 0
	star      db '\*', 0
	separator db '\', 0
	newLine   db 13, 10, 0
	hConsole  dd 0

.code

; EAX = longueur de la chaine pointee par pText.
StringLength PROC uses esi, pText:DWORD
	mov esi, pText
	xor eax, eax
@@:
	cmp BYTE PTR [esi + eax], 0
	je @F
	inc eax
	jmp @B
@@:
	ret
StringLength ENDP

PrintText PROC uses ebx, pText:DWORD
	LOCAL bytesWritten:DWORD

	push pText
	call StringLength
	add esp, 4
	mov ebx, eax
	push 0
	lea eax, bytesWritten
	push eax
	push ebx
	push pText
	push hConsole
	ApiCall __imp__WriteFile@20, 5
	ret
PrintText ENDP

; Parcourt pDirectory et rappelle cette procedure pour chaque sous-dossier.
ListDirectory PROC uses ebx esi edi, pDirectory:DWORD
	LOCAL searchPath[260]:BYTE
	LOCAL childPath[260]:BYTE
	LOCAL findData[FIND_DATA_SIZE]:BYTE
	LOCAL hFind:DWORD

	; searchPath = pDirectory + "\\*"
	push pDirectory
	lea eax, searchPath
	push eax
	ApiCall __imp__lstrcpyA@8, 2
	lea eax, star
	push eax
	lea eax, searchPath
	push eax
	ApiCall __imp__lstrcatA@8, 2

	FindFirstFileMacro OFFSET searchPath, OFFSET findData
	mov hFind, eax
	cmp eax, INVALID_HANDLE_VALUE
	je list_done

next_entry:
	; Ignorer les entrees speciales . et ..
	lea esi, findData[FD_FILE_NAME]
	cmp BYTE PTR [esi], '.'
	jne not_dot
	cmp BYTE PTR [esi + 1], 0
	je get_next
	cmp BYTE PTR [esi + 1], '.'
	jne not_dot
	cmp BYTE PTR [esi + 2], 0
	je get_next

not_dot:
	; childPath = pDirectory + "\\" + cFileName
	push pDirectory
	lea eax, childPath
	push eax
	ApiCall __imp__lstrcpyA@8, 2
	lea eax, separator
	push eax
	lea eax, childPath
	push eax
	ApiCall __imp__lstrcatA@8, 2
	lea eax, findData[FD_FILE_NAME]
	push eax
	lea eax, childPath
	push eax
	ApiCall __imp__lstrcatA@8, 2

	lea eax, childPath
	push eax
	call PrintText
	add esp, 4
	lea eax, newLine
	push eax
	call PrintText
	add esp, 4

	test DWORD PTR findData[FD_ATTRIBUTES], FILE_ATTRIBUTE_DIRECTORY
	jz get_next

	lea eax, childPath
	push eax
	call ListDirectory
	add esp, 4

get_next:
	FindNextFileMacro hFind, OFFSET findData
	test eax, eax
	jnz next_entry

	FindCloseMacro hFind

list_done:
	ret
ListDirectory ENDP

main PROC
	push STD_OUTPUT_HANDLE
	ApiCall __imp__GetStdHandle@4
	mov hConsole, eax

	push OFFSET rootPath
	call ListDirectory
	add esp, 4

	push 0
	ApiCall __imp__ExitProcess@4
main ENDP

END main

