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

/* globals for the project */
PRINTER_TYPE bc_printer = PRINTER_NONE;

char inpath[_MAX_PATH];
char outpath[_MAX_PATH];
char dirpath[_MAX_PATH];
char drive[_MAX_DRIVE];
char dir[_MAX_DIR];
char fname[_MAX_FNAME];
char ext[_MAX_EXT];

int line_numbers = HP41_FALSE;
int text_append = HP41_FALSE;
int force_global = HP41_FALSE;
int skip_line_feed = HP41_FALSE;
int raw_checksum = HP41_FALSE;

char ascii[80];
unsigned char buf1_16[16];
unsigned char buf2_16[16];

#define BUF_SIZE (1024*16)
unsigned char buf1_256[BUF_SIZE];
unsigned char buf2_256[BUF_SIZE];
unsigned char buf_512[BUF_SIZE];
unsigned char buf_1024[BUF_SIZE];

/* output descriptors */
FILE_DESC bin = {
	FILE_BIN,
	DATA_RAW,
	2,
	".bin",
	"bin",
};
FILE_DESC dat = {
	FILE_DAT,
	DATA_DAT,
	4,
	".dat",
	"dat",
};
FILE_DESC lif = {
	FILE_LIF,
	DATA_RAW,
	768,
	".lif",
	"lif",
};
FILE_DESC p41 = {
	FILE_P41,
	DATA_RAW,
	32,
	".p41",
	"p41",
};
FILE_DESC raw = {
	FILE_RAW,
	DATA_RAW,
	0,
	".raw",
	"raw",
};
FILE_DESC txt = {
	FILE_TXT,
	DATA_TXT,
	0,
	".txt",
	"txt",
};

/* 32-byte LIF header */
unsigned char lifhdr[32] = {
	0x80, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* 32-byte LIF directory */
unsigned char lifdir[32] = {
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x20,
};

char *single01_1C[] = {
	"LBL 00", "LBL 01", "LBL 02", "LBL 03", "LBL 04", "LBL 05", "LBL 06",
	"LBL 07", "LBL 08", "LBL 09", "LBL 10", "LBL 11", "LBL 12", "LBL 13", "LBL 14",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", ".", " E", "-",
};

char *single20_8F[] = {
	"RCL 00", "RCL 01", "RCL 02", "RCL 03", "RCL 04", "RCL 05", "RCL 06", "RCL 07",
	"RCL 08", "RCL 09", "RCL 10", "RCL 11", "RCL 12", "RCL 13", "RCL 14", "RCL 15",
	"STO 00", "STO 01", "STO 02", "STO 03", "STO 04", "STO 05", "STO 06", "STO 07",
	"STO 08", "STO 09", "STO 10", "STO 11", "STO 12", "STO 13", "STO 14", "STO 15",
	"+", "-", "*", "/", "X<Y?", "X>Y?", "X<=Y?", "S+",
	"S-", "HMS+", "HMS-", "MOD", "%", "%CH", "P-R", "R-P",
	"LN", "X^2", "SQRT", "Y^X", "CHS", "E^X", "LOG", "10^X",
	"E^X-1", "SIN", "COS", "TAN", "ASIN", "ACOS", "ATAN", "DEC",
	"1/X", "ABS", "FACT", "X#0?", "X>0?", "LN1+X", "X<0?", "X=0?",
	"INT", "FRC", "D-R", "R-D", "HMS", "HR", "RND", "OCT",
	"CLS", "X<>Y", "PI", "CLST", "R^", "RDN", "LASTX", "CLX",
	"X=Y?", "X#Y?", "SIGN", "X<=0?", "MEAN", "SDEV", "AVIEW", "CLD",
	"DEG", "RAD", "GRAD", "ENTER", "STOP", "RTN", "BEEP", "CLA",
	"ASHF", "PSE", "CLRG", "AOFF", "AON", "OFF", "PROMPT", "ADV",
};

char *prefix90_9F[] = {
	"RCL ", "STO ", "ST+ ", "ST- ", "ST* ", "ST/ ", "ISG ", "DSE ",
	"VIEW ", "SREG ", "ASTO ", "ARCL ", "FIX ", "SCI ", "ENG ", "TONE ",
};

char *prefixA8_AD[] = {
	"SF ", "CF ", "FS?C ", "FC?C ", "FS? ", "FC? ",
};

char *prefixB1_BF[] = {
	"GTO 00", "GTO 01", "GTO 02", "GTO 03", "GTO 04", "GTO 05", "GTO 06",
	"GTO 07", "GTO 08", "GTO 09", "GTO 10", "GTO 11", "GTO 12", "GTO 13", "GTO 14",
};

char *prefixCE_CF[] = {
	"X<> ", "LBL ",
};

char prefixXROM[] = "XROM mm,ff";

char prefixGTO[] = "GTO ";

char prefixXEQ[] = "XEQ ";

char prefixGTO_IND[] = "GTO IND ";

char prefixXEQ_IND[] = "XEQ IND ";

char prefixLBL[] = "LBL ";

char prefixW[] = "W ";

char prefixEND[] = "END";

char prefixGTO_NOP[] = ";GTO __ (SPARE)";

char prefixGTO_IND_NOP[] = ";GTO IND __ (SPARE)";

char prefixXEQ_IND_NOP[] = ";XEQ IND __ (SPARE)";

char postfixIND[] = "IND ";

char *postfix00_7F[] = {
	"00", "01", "02", "03", "04", "05", "06", "07",
	"08", "09", "10", "11", "12", "13", "14", "15",
	"16", "17", "18", "19", "20", "21", "22", "23",
	"24", "25", "26", "27", "28", "29", "30", "31",
	"32", "33", "34", "35", "36", "37", "38", "39",
	"40", "41", "42", "43", "44", "45", "46", "47",
	"48", "49", "50", "51", "52", "53", "54", "55",
	"56", "57", "58", "59", "60", "61", "62", "63",
	"64", "65", "66", "67", "68", "69", "70", "71",
	"72", "73", "74", "75", "76", "77", "78", "79",
	"80", "81", "82", "83", "84", "85", "86", "87",
	"88", "89", "90", "91", "92", "93", "94", "95",
	"96", "97", "98", "99", "100", "101", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J",
	"T", "Z", "Y", "X", "L", "M", "N", "O",
	"P", "Q", "R", "a", "b", "c", "d", "e",
};

char *alt_postfix102_111[] = {
	"102", "103",
	"104", "105", "106", "107", "108", "109", "110", "111",
};

char *alt_postfix117_122[] = {
	"[", "\\", "]",
	"^", "_", "`",
};

INTERNAL_XROM internal_xrom_table[] = {
	/* PLOTTER 1A- */
	{ "PLOTTER 1A-", 17, MAX_XROM_FUNCTIONS },
	{ "CLIPUU", 17, 1 },
	{ "CSIZE", 17, 2 },
	{ "CSIZEO", 17, 3 },
	{ "DGTIZE", 17, 4 },
	{ "DRAW", 17, 5 },
	{ "FRAME", 17, 6 },
	{ "GCLEAR", 17, 7 },
	{ "IDRAW", 17, 8 },
	{ "IMOVE", 17, 9 },
	{ "IPLOT", 17, 10 },
	{ "LABEL", 17, 11 },
	{ "LDIR", 17, 12 },
	{ "LIMIT", 17, 13 },
	{ "LOCATD", 17, 14 },
	{ "LOCATE", 17, 15 },
	{ "LORG", 17, 16 },
	{ "LTYPE", 17, 17 },
	{ "LTYPEO", 17, 18 },
	{ "LXAXIS", 17, 19 },
	{ "LYAXIS", 17, 20 },
	{ "MOVE", 17, 21 },
	{ "PEN", 17, 22 },
	{ "PENDN", 17, 23 },
	{ "PENUP", 17, 24 },
	{ "PINIT", 17, 25 },
	{ "PLOT", 17, 26 },
	{ "PLREGX", 17, 27 },
	{ "RATIO", 17, 28 },
	{ "RPLOT", 17, 29 },
	{ "SCALE", 17, 30 },
	{ "SETGU", 17, 31 },
	{ "SETUU", 17, 32 },
	{ "TICLEN", 17, 33 },
	{ "UNCLIP", 17, 34 },
	{ "WHERE", 17, 35 },
	{ "XAXIS", 17, 36 },
	{ "XAXISO", 17, 37 },
	{ "YAXIS", 17, 38 },
	{ "YAXISO", 17, 39 },

	/* PLOTTER 2A- */
	{ "PLOTTER 2A-", 18, MAX_XROM_FUNCTIONS },
	{ "PCLBUF", 18, 1 },
	{ "PDIR", 18, 2 },
	{ "PRCL", 18, 3 },
	{ "NEWPLOT", 18, 4 },
	{ "REPLOT", 18, 5 },
	{ "PLINIT", 18, 6 },
	{ "PLTUXY", 18, 7 },
	{ "PLANOT", 18, 8 },
	{ "Y?", 18, 9 },
	{ "X?", 18, 10 },
	{ "BC", 18, 11 },
	{ "BCA", 18, 12 },
	{ "BCAA", 18, 13 },
	{ "BCCKSM", 18, 14 },
	{ "BCO", 18, 15 },
	{ "BCP", 18, 16 },
	{ "BCREGX", 18, 17 },
	{ "BCSIZE", 18, 18 },
	{ "BCX", 18, 19 },
	{ "BCXS", 18, 20 },

	/* -HP-IL DEV */
	{ "-HP-IL DEV", 22, MAX_XROM_FUNCTIONS },
	{ "A-BUF", 22, 1 },
	{ "A=BUF?", 22, 2 },
	{ "A=BUFX?", 22, 3 },
	{ "AAD", 22, 4 },
	{ "AAU", 22, 5 },
	{ "AIPT", 22, 6 },
	{ "BSIZE?", 22, 7 },
	{ "BSIZEX", 22, 8 },
	{ "BUF-AX", 22, 9 },
	{ "BUF-RGX", 22, 10 },
	{ "BUF-XA", 22, 11 },
	{ "BUF-XB", 22, 12 },
	{ "CF33", 22, 13 },
	{ "CMD", 22, 14 },
	{ "DDL", 22, 15 },
	{ "DDT", 22, 16 },
	{ "FRAV?", 22, 17 },
	{ "FRNS?", 22, 18 },
	{ "GET", 22, 19 },
	{ "GTL", 22, 20 },
	{ "IDY", 22, 21 },
	{ "IFCR?", 22, 22 },
	{ "IFC", 22, 23 },
	{ "INBIN", 22, 24 },
	{ "INBUFX", 22, 25 },
	{ "LAD", 22, 26 },
	{ "LPD", 22, 27 },
	{ "MIPT", 22, 28 },
	{ "MONITOR", 22, 29 },
	{ "NRD", 22, 30 },
	{ "NRE", 22, 31 },
	{ "ORAV?", 22, 32 },
	{ "OUTBIN", 22, 33 },
	{ "OUTBINY", 22, 34 },
	{ "OUTBUFX", 22, 35 },
	{ "PT=", 22, 36 },
	{ "PT?", 22, 37 },
	{ "PRBYTES", 22, 38 },
	{ "PRFRMS", 22, 39 },
	{ "REN", 22, 40 },
	{ "RFRM", 22, 41 },
	{ "RG-BUFX", 22, 42 },
	{ "RG=BUF?", 22, 43 },
	{ "RREG", 22, 44 },
	{ "SAI", 22, 45 },
	{ "SCOPE", 22, 46 },
	{ "SDA", 22, 47 },
	{ "SDC", 22, 48 },
	{ "SDI", 22, 49 },
	{ "SF33", 22, 50 },
	{ "SRQR?", 22, 51 },
	{ "SST", 22, 52 },
	{ "TAD", 22, 53 },
	{ "TCT", 22, 54 },
	{ "UNL", 22, 55 },
	{ "UNT", 22, 56 },
	{ "WFRM", 22, 57 },
	{ "WREG", 22, 58 },
	{ "X-BUF", 22, 59 },
	{ "X=BUF?", 22, 60 },
	{ "X<>FLAG", 22, 61 },

	/* -X MASS 1A */
	{ "-X MASS 1A", 23, MAX_XROM_FUNCTIONS },
	{ "COPYFL", 23, 1 },
	{ "DIRX", 23, 2 },
	{ "FLLENG", 23, 3 },
	{ "FLTYPE", 23, 4 },
	{ "MCOPY", 23, 5 },
	{ "MCOPYPV", 23, 6 },
	{ "MVERIFY", 23, 7 },

	/* -X EXT FCN */
	{ "-X EXT FCN", 23, MAX_XROM_FUNCTIONS + 1 },
	{ "ALENGIO", 23, 9 },
	{ "ANUMDEL", 23, 10 },
	{ "ATOXL", 23, 11 },
	{ "ATOXR", 23, 12 },
	{ "ATOXX", 23, 13 },
	{ "XTOAL", 23, 14 },
	{ "XTOAR", 23, 15 },
	{ "X<>FIO", 23, 16 },
	{ "YTOAX", 23, 17 },

	/* -X CTL FNS */
	{ "-X CTL FNS", 23, MAX_XROM_FUNCTIONS + 2 },
	{ "AID", 23, 19 },
	{ "CLRDEV", 23, 20 },
	{ "CLRLOOP", 23, 21 },
	{ "DEVL", 23, 22 },
	{ "DEVT", 23, 23 },
	{ "FINDAID", 23, 24 },
	{ "ID", 23, 25 },
	{ "INAC", 23, 26 },
	{ "INACL", 23, 27 },
	{ "INAE", 23, 28 },
	{ "INAN", 23, 29 },
	{ "INXB", 23, 30 },
	{ "INP", 23, 31 },
	{ "LOCK", 23, 32 },
	{ "NLOOP", 23, 33 },
	{ "NOTREM", 23, 34 },
	{ "OUTAC", 23, 35 },
	{ "OUTACL", 23, 36 },
	{ "OUTAE", 23, 37 },
	{ "OUTAN", 23, 38 },
	{ "OUTXB", 23, 39 },
	{ "OUTP", 23, 40 },
	{ "POLL", 23, 41 },
	{ "POLLD", 23, 42 },
	{ "POLLE", 23, 43 },
	{ "POLLUNC", 23, 44 },
	{ "RCLSEL", 23, 45 },
	{ "SRQ?", 23, 46 },
	{ "STAT", 23, 47 },
	{ "XFER", 23, 48 },
	{ "XFERC", 23, 49 },
	{ "XFERCL", 23, 50 },
	{ "XFERE", 23, 51 },
	{ "XFERN", 23, 52 },

	/* -ADV CTL FN */
	{ "-ADV CTL FN", 23, MAX_XROM_FUNCTIONS + 3 },
	{ "ADROFF", 23, 54 },
	{ "ADRON", 23, 55 },
	{ "DDL", 23, 56 },
	{ "DDT", 23, 57 },
	{ "LAD", 23, 58 },
	{ "SEND", 23, 59 },
	{ "TAD", 23, 60 },
	{ "UNL", 23, 61 },
	{ "UNT", 23, 62 },

	/* -HP-IL DEV */
	{ "-HP-IL DEV", 24, MAX_XROM_FUNCTIONS },
	{ "AND", 24, 1 },
	{ "ASIZE?", 24, 2 },
	{ "A-XL", 24, 3 },
	{ "A-XR", 24, 4 },
	{ "A-XX", 24, 5 },
	{ "BININ", 24, 6 },
	{ "BINVIEW", 24, 7 },
	{ "BIT?", 24, 8 },
	{ "HEXIN", 24, 9 },
	{ "HEXVIEW", 24, 10 },
	{ "NOT", 24, 11 },
	{ "OR", 24, 12 },
	{ "OCTIN", 24, 13 },
	{ "OCTVIEW", 24, 14 },
	{ "ROMCHKX", 24, 15 },
	{ "ROTXY", 24, 16 },
	{ "XOR", 24, 17 },
	{ "X-AL", 24, 18 },
	{ "X-AR", 24, 19 },
	{ "Y-AX", 24, 20 },

	/* -EXT FCN 2D */
	{ "-EXT FCN 2D", 25, MAX_XROM_FUNCTIONS },
	{"ALENG", 25, 1 },
	{"ANUM", 25, 2 },
	{"APPCHR", 25, 3 },
	{"APPREC", 25, 4 },
	{"ARCLREC", 25, 5 },
	{"AROT", 25, 6 },
	{"ATOX", 25, 7 },
	{"CLFL", 25, 8 },
	{"CLKEYS", 25, 9 },
	{"CRFLAS", 25, 10 },
	{"CRFLD", 25, 11 },
	{"DELCHR", 25, 12 },
	{"DELREC", 25, 13 },
	{"EMDIR", 25, 14 },
	{"FLSIZE", 25, 15 },
	{"GETAS", 25, 16 },
	{"GETKEY", 25, 17 },
	{"GETP", 25, 18 },
	{"GETR", 25, 19 },
	{"GETREC", 25, 20 },
	{"GETRX", 25, 21 },
	{"GETSUB", 25, 22 },
	{"GETX", 25, 23 },
	{"INSCHR", 25, 24 },
	{"INSREC", 25, 25 },
	{"PASN", 25, 26 },
	{"PCLPS", 25, 27 },
	{"POSA", 25, 28 },
	{"POSFL", 25, 29 },
	{"PSIZE", 25, 30 },
	{"PURFL", 25, 31 },
	{"RCLFLAG", 25, 32 },
	{"RCLPT", 25, 33 },
	{"RCLPTA", 25, 34 },
	{"REGMOVE", 25, 35 },
	{"REGSWAP", 25, 36 },
	{"SAVEAS", 25, 37 },
	{"SAVEP", 25, 38 },
	{"SAVER", 25, 39 },
	{"SAVERX", 25, 40 },
	{"SAVEX", 25, 41 },
	{"SEEKPT", 25, 42 },
	{"SEEKPTA", 25, 43 },
	{"SIZE?", 25, 44 },
	{"STOFLAG", 25, 45 },
	{"X<>F", 25, 46 },
	{"XTOA", 25, 47 },

	/* -CX EXT FCN */
	{ "-CX EXT FCN", 25, MAX_XROM_FUNCTIONS + 1},
	{"ASROOM", 25, 49 },
	{"CLRGX", 25, 50 },
	{"ED", 25, 51 },
	{"EMDIRX", 25, 52 },
	{"EMROOM", 25, 53 },
	{"GETKEYX", 25, 54 },
	{"RESZFL", 25, 55 },
	{"SREG?", 25, 56 },
	{"X=NN?", 25, 57 },
	{"X#NN?", 25, 58 },
	{"X<NN?", 25, 59 },
	{"X<=NN?", 25, 60 },
	{"X>NN?", 25, 61 },
	{"X>=NN?", 25, 62 },

	/* -TIME 2C */
	{ "-TIME 2C", 26, MAX_XROM_FUNCTIONS },
	{"ADATE", 26, 1 },
	{"ALMCAT", 26, 2 },
	{"ALMNOW", 26, 3 },
	{"ATIME", 26, 4 },
	{"ATIME24", 26, 5 },
	{"CLK12", 26, 6 },
	{"CLK24", 26, 7 },
	{"CLKT", 26, 8 },
	{"CLKTD", 26, 9 },
	{"CLOCK", 26, 10 },
	{"CORRECT", 26, 11 },
	{"DATE", 26, 12 },
	{"DATE+", 26, 13 },
	{"DDAYS", 26, 14 },
	{"DMY", 26, 15 },
	{"DOW", 26, 16 },
	{"MDY", 26, 17 },
	{"RCLAF", 26, 18 },
	{"RCLSW", 26, 19 },
	{"RUNSW", 26, 20 },
	{"SETAF", 26, 21 },
	{"SETDATE", 26, 22 },
	{"SETIME", 26, 23 },
	{"SETSW", 26, 24 },
	{"STOPSW", 26, 25 },
	{"SW", 26, 26 },
	{"T+X", 26, 27 },
	{"TIME", 26, 28 },
	{"XYZALM", 26, 29 },

	/*  -CXX TIME */
	{ " -CXX TIME", 26, MAX_XROM_FUNCTIONS + 1 },
	{ "CLALMA", 26, 31 },
	{ "CLALMX", 26, 32 },
	{ "CLRALMS", 26, 33 },
	{ "RCLALM", 26, 34 },
	{ "SWPT", 26, 35 },
	{ "TRNG", 26, 36 },

	/*  -DM 41X- */
	{ " -DM 41X-", 26, MAX_XROM_FUNCTIONS + 2 },
	{ "ABSP", 26, 38 },
	{ "AINT", 26, 39 },
	{ "ASWAP", 26, 40 },
	{ "CLAC", 26, 41 },
	{ "CLEM", 26, 42 },
	{ "FAST", 26, 43 },
	{ "FILL", 26, 44 },
	{ "FLCOPY", 26, 45 },
	{ "FLHD", 26, 46 },
	{ "FLTYPE", 26, 47 },
	{ "LKAOFF", 26, 48 },
	{ "LKAON", 26, 49 },
	{ "RENMFL", 26, 50 },
	{ "RETPFL", 26, 51 },
	{ "SLOW", 26, 52 },
	{ "WORKFL", 26, 53 },

	/* -WAND 1F */
	{ "-WAND 1F", 27, MAX_XROM_FUNCTIONS },
	{ "WNDDTA", 27, 1 },
	{ "WNDDTX", 27, 2 },
	{ "WNDLNK", 27, 3 },
	{ "WNDSUB", 27, 4 },
	{ "WNDSCN", 27, 5 },
	{ "WNDTST", 27, 6 },

	/* -MASS ST 1H */
	{ "-MASS ST 1H", 28, MAX_XROM_FUNCTIONS },
	{ "CREATE", 28, 1 },
	{ "DIR", 28, 2 },
	{ "PURGE", 28, 4 },
	{ "READA", 28, 5 },
	{ "READK", 28, 6 },
	{ "READP", 28, 7 },
	{ "READR", 28, 8 },
	{ "READRX", 28, 9 },
	{ "READS", 28, 10 },
	{ "READSUB", 28, 11 },
	{ "RENAME", 28, 12 },
	{ "SEC", 28, 13 },
	{ "SEEKR", 28, 14 },
	{ "UNSEC", 28, 15 },
	{ "VERIFY", 28, 16 },
	{ "WRTA", 28, 17 },
	{ "WRTK", 28, 18 },
	{ "WRTP", 28, 19 },
	{ "WRTPV", 28, 20 },
	{ "WRTR", 28, 21 },
	{ "WRTRX", 28, 22 },
	{ "WRTS", 28, 23 },
	{ "ZERO", 28, 24 },

	/* -CTL FNS */
	{ "-CTL FNS", 28, MAX_XROM_FUNCTIONS + 1},
	{ "AUTOIO", 28, 27 },
	{ "FINDID", 28, 28 },
	{ "INA", 28, 29 },
	{ "IND", 28, 30 },
	{ "INSTAT", 28, 31 },
	{ "LISTEN", 28, 32 },
	{ "LOCAL", 28, 33 },
	{ "MANIO", 28, 34 },
	{ "OUTA", 28, 35 },
	{ "PWRDN", 28, 36 },
	{ "PWRUP", 28, 37 },
	{ "REMOTE", 28, 38 },
	{ "SELECT", 28, 39 },
	{ "STOPIO", 28, 40 },
	{ "TRIGGER", 28, 41 },

	/* -PRINTER 2E */
	{ "-PRINTER 2E", 29, MAX_XROM_FUNCTIONS },
	{"ACA", 29, 1 },
	{"ACCHR", 29, 2 },
	{"ACCOL", 29, 3 },
	{"ACSPEC", 29, 4 },
	{"ACX", 29, 5 },
	{"BLDSPEC", 29, 6 },
	{"PRA", 29, 8 },
	{"PRAXIS", 29, 9 },
	{"PRBUF", 29, 10 },
	{"PRFLAGS", 29, 11 },
	{"PRKEYS", 29, 12 },
	{"PRPLOT", 29, 14 },
	{"PRPLOTP", 29, 15 },
	{"PRREG", 29, 16 },
	{"PRREGX", 29, 17 },
	{"PRS", 29, 18 },
	{"PRSTK", 29, 19 },
	{"PRX", 29, 20 },
	{"REGPLOT", 29, 21 },
	{"SKPCHR", 29, 22 },
	{"SKPCOL", 29, 23 },
	{"STKPLOT", 29, 24 },
	{"FMT", 29, 25 },

	/* CARD RDR 1E */
	{ "CARD RDR 1E", 30, MAX_XROM_FUNCTIONS },
	{ "MRG", 30, 1 },
	{ "RDTA", 30, 2 },
	{ "RDTAX", 30, 3 },
	{ "RSUB", 30, 4 },
	{ "WDTA", 30, 7 },
	{ "WDTAX", 30, 8 },
	{ "WSTS", 30, 10 },
	{ "7CLREG", 30, 11 },
	{ "7DSP0", 30, 12 },
	{ "7DSP1", 30, 13 },
	{ "7DSP2", 30, 14 },
	{ "7DSP3", 30, 15 },
	{ "7DSP4", 30, 16 },
	{ "7DSP5", 30, 17 },
	{ "7DSP6", 30, 18 },
	{ "7DSP7", 30, 19 },
	{ "7DSP8", 30, 20 },
	{ "7DSP9", 30, 21 },
	{ "7DSPI", 30, 22 },
	{ "7DSZ", 30, 23 },
	{ "7DSZI", 30, 24 },
	{ "7ENG", 30, 25 },
	{ "7FIX", 30, 26 },
	{ "7GSBI", 30, 27 },
	{ "7GTOI", 30, 28 },
	{ "7ISZ", 30, 29 },
	{ "7ISZI", 30, 30 },
	{ "7P<>S", 30, 31 },
	{ "7PRREG", 30, 32 },
	{ "7PRSTK", 30, 33 },
	{ "7PRTX", 30, 34 },
	{ "7RCLS", 30, 35 },
	{ "7SCI", 30, 36 },
};

int internal_xrom_enable[MAX_XROM_MODULES] = { 0 };
int internal_xrom_entries = sizeof(internal_xrom_table) / sizeof(INTERNAL_XROM);
EXTERNAL_XROM external_xrom_table[MAX_XROM_MODULES * MAX_XROM_FUNCTIONS] = {{{0}, 0, 0}};
int external_xrom_entries = sizeof(external_xrom_table) / sizeof(EXTERNAL_XROM);
