/*
HP41UC
User-Code File Converter/Compiler/De-compiler/Barcode Generator.
Copyright (c) Leo Duran, 2000-2020.  All rights reserved.

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

typedef struct {
	char *prefix;
	int  code;
} FCN;

/* compiler states */
typedef enum {
	COMPILE_SEEK_START_LINE,
	COMPILE_GET_LINE,
	COMPILE_SEEK_END_LINE,
	COMPILE_LINE_OUTPUT,
	COMPILE_IGNORE_BAD_LINE
} COMPILE_STATE;

int compile_args(unsigned char *code_buffer,
	int line_argc,
	unsigned char *line_argv[],
	char * line);

int compile_num(unsigned char *code, unsigned char *num);
int compile_text(unsigned char *code, unsigned char *text, int count);
int compile_alpha(unsigned char *code, unsigned char *prefix, unsigned char *alpha, int key_code);
int compile_xrom(char *function, unsigned char *code_buffer, int *codes);
int compile_arg1(unsigned char *code, unsigned char *prefix);
int compile_arg2(unsigned char *code, unsigned char *prefix, unsigned char *postfix);
int compile_arg3(unsigned char *code, unsigned char *prefix, unsigned char *pind, unsigned char *postfix);
int compile_label(unsigned char *code, unsigned char *label, unsigned char *alpha, int asn);

int get_numeric_prefix(unsigned char *numeric, unsigned char *buffer);

int get_text_prefix(unsigned char *text, unsigned char *buffer, int *pcount);

int get_alpha_postfix(unsigned char *alpha, unsigned char *buffer);

int is_postfix(unsigned char *postfix, int *pindex);

int parse_text(unsigned char *text, unsigned char *buffer, int *pcount);

int is_append(unsigned char *prefix);

int is_text(unsigned char *prefix);

int is_local_label(char *alpha);

int get_key(unsigned char *key);

char get_xdigit(unsigned char xdigit);

FCN alt_fcn1[] = {
	{"‰+", 0x47},
	{"SIGMA+", 0x47},
	{"‰-", 0x48},
	{"SIGMA-", 0x48},
	{"P->R", 0x4E},
	{"R->P", 0x4F},
	{"X**2", 0x51},
	{"Y**X", 0x53},
	{"E**X", 0x55},
	{"10**X", 0x57},
	{"E**X-1", 0x58},
	{"X!=0?", 0x63},
	{"X<>0?", 0x63},
	{"D->R", 0x6A},
	{"R->D", 0x6B},
	{"CL‰", 0x70},
	{"CLSIGMA", 0x70},
	{"RUP", 0x74},
	{"X!=Y?", 0x79},
	{"X<>Y?", 0x79},
	{"ENTER^", 0x83}
};

FCN alt_fcn2[] = {
	{"STO+", 0x92},
	{"STO-", 0x93},
	{"STO*", 0x94},
	{"STO/", 0x95},
	{"‰REG", 0x99},
	{"SIGREG", 0x99},
	{"SIGMAREG", 0x99}
};


