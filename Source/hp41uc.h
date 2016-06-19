/*
HP41UC
User-Code File Converter/Compiler/De-compiler/Barcode Generator.
Copyright (c) Leo Duran, 2000-2016.  All rights reserved.

Build environment: Microsoft Visual Studio or GNU C compiler.
*/

/*
This file is part of HP41UC.

HP41UC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HP41UC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HP41UC.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef __GNUC__
#include <stddef.h>
#include <sys/io.h>
#include <unistd.h>		/* access() */
#include <dirent.h>		/* opendir() */
#include <limits.h>		/* realpath() */
#include <errno.h>
#include <termios.h>
#else
#include <io.h>
#include <conio.h>
#endif

#ifndef _MAX_DRIVE
#define _MAX_DRIVE	(3)
#endif

#ifndef _MAX_DIR
#define _MAX_DIR	(256)
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME	(256)
#endif

#ifndef _MAX_EXT
#define _MAX_EXT	(256)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH	(_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT)
#endif

#ifdef __GNUC__
int _getch(void);
int _getche(void);
char *_strupr(char *str);
char *to_unix_path(char *p);
int _fileno(FILE *stream);
int _stricmp(const char * s1, const char *s2);
#endif

long get_filelength(int fd);

/* find files */
typedef struct {
	char *name;
	int size;
	void *phandle;
#ifdef __GNUC__
	union {
		struct {
			char dir_path[_MAX_PATH];
			char file_ext[_MAX_EXT];
		};
		char file_spec[_MAX_FNAME + _MAX_EXT];
	} data;
#else	 /* _MSC_VER */
	char data[sizeof(struct _finddata_t)];
#endif
} FIND_FILE;

void findfile_init(FIND_FILE *ff);
int findfile_first(char *path, FIND_FILE *ff);
int findfile_next(FIND_FILE *ff);
void findfile_close(FIND_FILE *ff);

#ifdef __GNUC__
typedef enum {
	FILE_EXIST = F_OK,
	FILE_WRITE_OK = W_OK,
	FILE_READ_OK = R_OK,
	FILE_READ_WRITE = X_OK
} FILE_MODE;
#else	 /* _MSC_VER */
typedef enum {
	FILE_EXIST = 0,
	FILE_WRITE_OK = 2,
	FILE_READ_OK = 4,
	FILE_READ_WRITE = 6
} FILE_MODE;
#endif

int file_access(
	const char *path,
	FILE_MODE mode
	);

char *file_fullpath(
	char *absPath,
	const char *relPath
	);

void file_splitpath(
	const char* path,
	char* drv,
	char* dir,
	char* name,
	char* ext
	);

int getiopath(char *in_file,
	char *in_ext,
	FIND_FILE *ff,
	int max_in_files,
	char *in_dir,
	char *out_file,
	char *out_ext,
	char *out_path);

void getfullpath(char *fullpath, char *dirpath, char *name);
void override_file_ext(char *cur_path, char *new_path, char *new_ext);


/* file types */
typedef enum {
	FILE_BIN,
	FILE_DAT,
	FILE_LIF,
	FILE_P41,
	FILE_RAW,
	FILE_TXT
} FILE_TYPE;

/* data types */
typedef enum {
	DATA_DAT,
	DATA_RAW,
	DATA_TXT
} DATA_TYPE;

typedef struct {
	FILE_TYPE type;
	int header;
	char *ext;
	char *banner;
} FILE_DESC;

/* compiler flags */
typedef enum {
	COMPILE_FLAG_NONE,
	COMPILE_FLAG_ERROR,
	COMPILE_FLAG_END,
	COMPILE_FLAG_RESTART,
	COMPILE_FLAG_RESTART_EOF,
	COMPILE_FLAG_EOF
} COMPILE_FLAG;

/* barcode printers */
typedef enum {
	PRINTER_NONE,
	PRINTER_HP,
	PRINTER_POSTCRIPT
} PRINTER_TYPE;

void bintoxxx(char *infile, char *outfile, char *name, FILE_DESC *pout);

void dattoxxx(char *infile, char *outfile, char *name, FILE_DESC *pout);

void liftoxxx(char *infile, char *outfile, char *name, FILE_DESC *pout);
void lifdump(char *infile, char *name);

void p41toxxx(char *infile, char *outfile, FILE_DESC *pout);
void p41dump(char *infile);

void rawtoxxx(char *infile, char *outfile, char *name, FILE_DESC *pout);

void txttoxxx(char *infile, char *outfile, char *name, FILE_DESC *pout);

void barcode_init(void);
void barcode(char *infile, char *outfile, char *title);

void decompile_init(void);
int decompile(unsigned char *out_buffer, int out_size,
	unsigned char **pin_buffer, int *pin_count,
	int *ppending, int *pend);

void compile_init(void);
int compile(unsigned char *out_buffer, int out_size,
	unsigned char **pin_buffer, int *pin_count,
	COMPILE_FLAG *pflag, int *ppending, int *pend_count);

int hextoascii(unsigned char *ascii_buffer,
	unsigned char *hex_buffer, int hex_count);
int asciitohex(unsigned char *hex_buffer,
	unsigned char *ascii_buffer, int ascii_count);
int nonxdigit_buffer(unsigned char *buffer, int count);

int seek_end(unsigned char *buffer, int count);

void compile_end(unsigned char *buffer, int bytes);

int oktowrite(char *path);

FILE *open_input(char *inpath, long inlength, char *infile, char *outpath);

FILE *open_output(char *outpath);

long read_bin_size(FILE *fin, char *inpath, long length);

long read_dat_size(FILE *fin, char *inpath, long length);

long read_p41_dir(FILE *fin, char *inpath, long *plength, char *name);

long read_lif_dir(FILE *fin, char *inpath, long *plength, char *name,
	long dirblks);

long read_lif_hdr(FILE *fin, char *inpath, long size);

long get_lif_size(unsigned char *buffer2, long length);

void get_lif_name(char *pname, char *name, char *filename);

int is_lif_name(char *lifname, char *name);

long write_lif_hdr(FILE *fout, long *pstartblk, long *pdirpos, int files);

long write_lif_dir(FILE *fout, char *name, long startblk, long size);

void copy_file(FILE *fout, char *outpath,
	FILE_TYPE outftype, long *poutlength,
	char *pname, int files, long inlength,
	FILE *fin, char *inpath, DATA_TYPE indtype);

int copy_blocks(FILE *fout, char *outpath,
	DATA_TYPE outdtype, long *poutlength,
	long *psize, unsigned char *pchksum, long *pstartblk,
	FILE *fin, char *inpath,
	DATA_TYPE indtype, long inlength);

int copy_raw_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	long *poutlength, long *psize,
	unsigned char *pchksum, long *pstartblk,
	FILE *fin, char *inpath, long inlength);

int copy_dat_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	long *poutlength, long *psize,
	unsigned char *pchksum, long *pstartblk,
	FILE *fin, char *inpath, long inlength);

int copy_txt_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	long *poutlength, long *psize,
	unsigned char *pchksum, long *pstartblk,
	FILE *fin, char *inpath, long inlength);

long write_raw_checksum(FILE *fout, char *outpath, long *poutlength,
	unsigned char *buffer, long bufsize,
	unsigned char chksum);

long write_dat_checksum(FILE *fout, char *outpath, long *poutlength,
	unsigned char chksum);

long write_raw_size(FILE *fout, long size);

long write_dat_size(FILE *fout, long size);

void dump_lif_dir(FILE *fin, char *inpath, long count, char *name);

void show_lif_info(char *name, long size);

void closefiles(FILE *fin, FILE *fout,
	long outlength, int header);

void help(int do_help);

/* global variables */
extern PRINTER_TYPE bc_printer;

extern char inpath[_MAX_PATH];
extern char outpath[_MAX_PATH];
extern char dirpath[_MAX_PATH];
extern char drive[_MAX_DRIVE];
extern char dir[_MAX_DIR];
extern char fname[_MAX_FNAME];
extern char ext[_MAX_EXT];

extern int line_numbers;
extern int text_append;
extern int raw_checksum;
extern int do_xrom23;
extern int do_xrom25;
extern int do_xrom26;
extern int do_xrom28;
extern int force_global;

extern char ascii[80];
extern unsigned char buf1_16[16];
extern unsigned char buf2_16[16];
extern unsigned char buf1_256[256];
extern unsigned char buf2_256[256];
extern unsigned char buf_512[512];
extern unsigned char buf_1024[1024];

/* 32-byte LIF header */
extern unsigned char lifhdr[32];

/* 32-byte LIF directory */
extern unsigned char lifdir[32];

extern char *single01_1C[];

extern char *single20_8F[];

extern char *prefix90_9F[];

extern char *prefixA8_AD[];

extern char *prefixB1_BF[];

extern char *prefixCE_CF[];

extern char prefixXROM[];

extern char prefixGTO[];

extern char prefixXEQ[];

extern char prefixGTO_IND[];

extern char prefixXEQ_IND[];

extern char prefixLBL[];

extern char prefixW[];

extern char prefixEND[];

extern char prefixGTO_NOP[];

extern char prefixGTO_IND_NOP[];

extern char prefixXEQ_IND_NOP[];

extern char postfixIND[];

extern char *postfix00_7F[];

extern char *alt_postfix102_111[];

extern char *alt_postfix117_122[];

extern char *xrom23[];
extern char *xrom25[];
extern char *xrom26[];
extern char *xrom28[];

