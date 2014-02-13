/*
This is part of my own extension to the original LibCE
My extension includes:
	"tmpnam.h" stored in the include library
	"tmpnam.cpp" stored in the ./src directory
*/
/* Minimum length for termporary file name (constant) */

#define L_tmpnam 4

/* Max number of unique temp file names that can be generated = 26^(L_tmpnam-1) = 26^3 */
#define TMP_MAX 17576

#ifdef __cplusplus
extern "C" {
#endif

char* __cdecl tmpnam (char* );

#ifdef __cplusplus
}
#endif