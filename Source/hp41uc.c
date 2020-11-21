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

#include "hp41uc.h"
#include "hp41ucg.h"

static char *xrom_file_path[MAX_XROM_MODULES] = { 0 };
static int xrom_file_count = 0;

/* compiler states */
typedef enum {
	XROM_SEEK_START_LINE,
	XROM_GET_LINE,
	XROM_SEEK_END_LINE,
	XROM_LINE_OUTPUT,
	XROM_IGNORE_BAD_LINE
} XROM_STATE;

#define XROM_ARGS	3
#define XROM_LINE	128

int main(int argc, char *argv[])
{
	int h, i, j = 0, k;
	char in = 0, out = 0;
	char *name = NULL;
	char *infile = NULL, *outfile = NULL;
	int xrom_disable_all = HP41_FALSE;
	int xrom_disable_one = HP41_FALSE;
	int xrom_id;
	char *endptr;

	/* enable XROMs by default */
	enable_internal_xrom_all(HP41_TRUE);

	/* parser */
	for (h = 0, i = 1; i < argc && !h; ++i) {
		/* input & output parameters */
		if (argv[i][0] == '-' || argv[i][0] == '/') {
			j = 1;
			if (argv[i][2] == ':' || argv[i][2] == '=')
				k = 3;
			else
				k = 2;
		}
		else if (argv[i][1] == ':' || argv[i][1] == '=') {
			j = 0;
			k = 2;
		}
		else {
			k = 0;
		}

		/* start parsing */
		if (_stricmp(argv[i], "?") == HP41_OK)
			h = 1;
		else if (_stricmp(argv[i], "-a") == HP41_OK ||
			_stricmp(argv[i], "/a") == HP41_OK) {
			text_append = HP41_TRUE;
		}
    else if (_stricmp(argv[i], "-f") == HP41_OK ||
      _stricmp(argv[i], "/f") == HP41_OK) {
      skip_line_feed = HP41_TRUE;
    }
		else if (_stricmp(argv[i], "-g") == HP41_OK ||
			_stricmp(argv[i], "/g") == HP41_OK) {
			force_global = HP41_TRUE;
		}
		else if (_stricmp(argv[i], "-h") == HP41_OK ||
			_stricmp(argv[i], "/h") == HP41_OK) {
			bc_printer = PRINTER_HP;
		}
		else if (_stricmp(argv[i], "-s") == HP41_OK ||
			_stricmp(argv[i], "/s") == HP41_OK) {
			bc_printer = PRINTER_POSTCRIPT;
		}
		else if (_stricmp(argv[i], "-n") == HP41_OK ||
			_stricmp(argv[i], "/n") == HP41_OK) {
			line_numbers = HP41_TRUE;
		}
		else if (_stricmp(argv[i], "-k") == HP41_OK ||
			_stricmp(argv[i], "/k") == HP41_OK) {
			raw_checksum = HP41_TRUE;
		}
		else if (_strnicmp(argv[i], "-x", 2) == HP41_OK ||
			_strnicmp(argv[i], "/x", 2) == HP41_OK) {
			if (strlen(argv[i]) == 2) {
				xrom_disable_all = HP41_TRUE;
			}
			else {
				xrom_id = (int)strtoul(&argv[i][2], &endptr, 10);
				if (xrom_id < MAX_XROM_MODULES) {
					enable_internal_xrom_one(xrom_id, HP41_FALSE);
					xrom_disable_one = HP41_TRUE;
				}
				else {
					h = 1;
				}
			}
		}
		else if (k) {
			if (argv[i][j] == 'b' || argv[i][j] == 'B') {
				if (in == 0) {
					in = 'B';
					infile = &argv[i][k];
				}
				else if (in != 'B' && out == 0) {
					out = 'B';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'd' || argv[i][j] == 'D') {
				if (in == 0) {
					in = 'D';
					infile = &argv[i][k];
				}
				else if (in != 'D' && out == 0) {
					out = 'D';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'l' || argv[i][j] == 'L') {
				if (in == 0) {
					in = 'L';
					infile = &argv[i][k];
				}
				else if (in != 'L' && out == 0) {
					out = 'L';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'p' || argv[i][j] == 'P') {
				if (in == 0) {
					in = 'P';
					infile = &argv[i][k];
				}
				else if (in != 'P' && out == 0) {
					out = 'P';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'r' || argv[i][j] == 'R') {
				if (in == 0) {
					in = 'R';
					infile = &argv[i][k];
				}
				else if (in != 'R' && out == 0) {
					out = 'R';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 't' || argv[i][j] == 'T') {
				if (in == 0) {
					in = 'T';
					infile = &argv[i][k];
				}
				else if (in != 'T' && out == 0) {
					out = 'T';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'h' || argv[i][j] == 'H') {
				if (out == 0) {
					out = 'W';
					outfile = &argv[i][k];
					bc_printer = PRINTER_HP;
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 's' || argv[i][j] == 'S') {
				if (out == 0) {
					out = 'W';
					outfile = &argv[i][k];
					bc_printer = PRINTER_POSTCRIPT;
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'w' || argv[i][j] == 'W') {
				if (out == 0) {
					out = 'W';
					outfile = &argv[i][k];
				}
				else {
					h = 1;
				}
			}
			else if (argv[i][j] == 'm' || argv[i][j] == 'M') {
				if (xrom_file_count < MAX_XROM_MODULES && argv[i][k])
					xrom_file_path[xrom_file_count++] = &argv[i][k];
			}
			else {
				h = 1;
			}
		}
		else if (name == NULL) {
			name = argv[i];
		}
		else {
			h = 1;
		}
	}

	/* dispatcher */
	if (h == 0) {
		/* generate barcode to default file? */
		if (out == 0 && bc_printer != PRINTER_NONE)
			out = 'W';

		/* set default help */
		h = in ? in : (out ? out : 1);

		/* input file spec? */
		if (!infile || *infile == '\0') {
			if (in == 'L' && out == 0 && name && *name != '\0') {
				lifdump(name, NULL);
				h = 0;
			}
			else if (in == 'P' && out == 0 && name && *name != '\0') {
				p41dump(name);
				h = 0;
			}
			else if (in == 0) {
				if (xrom_disable_all) {
					dump_internal_xrom_module_all();
					h = 0;
				}
				if (xrom_disable_one) {
					dump_excluded_xrom_modules();
					h = 0;
				}
			}
			else if (out) {
				printf("Error: missing input path specification.\n");
			}
		}
		else {
			if (xrom_disable_all)
				enable_internal_xrom_all(HP41_FALSE);

			if (in == 'B') {
				if (out == 'L') {
					convert(infile, &bin, outfile, &lif, name);
					h = 0;
				}
				else if (out == 'P') {
					convert(infile, &bin, outfile, &p41, name);
					h = 0;
				}
				else if (name == NULL) {
					if (out == 'D') {
						convert(infile, &bin, outfile, &dat, NULL);
						h = 0;
					}
					else if (out == 'R') {
						convert(infile, &bin, outfile, &raw, NULL);
						h = 0;
					}
					else if (out == 'T') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &bin, outfile, &txt, NULL);
						h = 0;
					}
				}
			}
			else if (in == 'D') {
				if (out == 'L') {
					convert(infile, &dat, outfile, &lif, name);
					h = 0;
				}
				else if (out == 'P') {
					convert(infile, &dat, outfile, &p41, name);
					h = 0;
				}
				else if (name == NULL) {
					if (out == 'B') {
						convert(infile, &dat, outfile, &bin, NULL);
						h = 0;
					}
					else if (out == 'R') {
						convert(infile, &dat, outfile, &raw, NULL);
						h = 0;
					}
					else if (out == 'T') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &dat, outfile, &txt, NULL);
						h = 0;
					}
				}
			}
			else if (in == 'L') {
				if (out == 'B') {
					convert(infile, &lif, outfile, &bin, name);
					h = 0;
				}
				else if (out == 'D') {
					convert(infile, &lif, outfile, &dat, name);
					h = 0;
				}
				else if (out == 'P') {
					convert(infile, &lif, outfile, &p41, name);
					h = 0;
				}
				else if (out == 'R') {
					convert(infile, &lif, outfile, &raw, name);
					h = 0;
				}
				else if (out == 'T') {
					if (parse_xrom_files() == HP41_OK)
						convert(infile, &lif, outfile, &txt, name);
					h = 0;
				}
				else if (out == 0) {
					lifdump(infile, name);
					h = 0;
				}
			}
			else if (in == 'P') {
				if (name == NULL) {
					if (out == 'B') {
						convert(infile, &p41, outfile, &bin, NULL);
						h = 0;
					}
					else if (out == 'D') {
						convert(infile, &p41, outfile, &dat, NULL);
						h = 0;
					}
					else if (out == 'L') {
						convert(infile, &p41, outfile, &lif, NULL);
						h = 0;
					}
					else if (out == 'R') {
						convert(infile, &p41, outfile, &raw, NULL);
						h = 0;
					}
					else if (out == 'T') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &p41, outfile, &txt, NULL);
						h = 0;
					}
					else if (out == 0) {
						p41dump(infile);
						h = 0;
					}
				}
			}
			else if (in == 'R') {
				if (out == 'L') {
					convert(infile, &raw, outfile, &lif, name);
					h = 0;
				}
				else if (out == 'P') {
					convert(infile, &raw, outfile, &p41, name);
					h = 0;
				}
				else if (out == 'W') {
					barcode_init();
					barcode(infile, outfile, name);
					h = 0;
				}
				else if (name == NULL) {
					if (out == 'B') {
						convert(infile, &raw, outfile, &bin, NULL);
						h = 0;
					}
					else if (out == 'D') {
						convert(infile, &raw, outfile, &dat, NULL);
						h = 0;
					}
					else if (out == 'T') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &raw, outfile, &txt, NULL);
						h = 0;
					}
				}
			}
			else if (in == 'T') {
				if (out == 'L') {
					if (parse_xrom_files() == HP41_OK)
						convert(infile, &txt, outfile, &lif, name);
					h = 0;
				}
				else if (out == 'P') {
					if (parse_xrom_files() == HP41_OK)
						convert(infile, &txt, outfile, &p41, name);
					h = 0;
				}
				else if (name == NULL) {
					if (out == 'B') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &txt, outfile, &bin, NULL);
						h = 0;
					}
					else if (out == 'D') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &txt, outfile, &dat, NULL);
						h = 0;
					}
					else if (out == 'R') {
						if (parse_xrom_files() == HP41_OK)
							convert(infile, &txt, outfile, &raw, NULL);
						h = 0;
					}
				}
			}
		}
	}

	/* need help? */
	help(h);

	return 0;
}

void enable_internal_xrom_all(int state)
{
	int id;

	for (id = 0; id < MAX_XROM_MODULES; ++id)
		internal_xrom_enable[id] = state;
}

void enable_internal_xrom_one(int id, int state)
{
	if (id < MAX_XROM_MODULES)
		internal_xrom_enable[id] = state;
}

int is_enable_internal_xrom(int id)
{
	if (id < MAX_XROM_MODULES)
		return internal_xrom_enable[id];
	return HP41_FALSE;
}

void dump_excluded_xrom_modules(void)
{
	int id;

	for (id = 0; id < MAX_XROM_MODULES; ++id) {
		if (internal_xrom_enable[id] == HP41_FALSE) {
			dump_internal_xrom_module_one(id);
		}
	}
}

void dump_internal_xrom_module_all(void)
{
	int i;

	printf("Supported XROM Modules:\n");
	for (i = 0; i < internal_xrom_entries; ++i) {
		if (internal_xrom_table[i].function_id >= MAX_XROM_FUNCTIONS) {
			printf(" [ %02d ] %s\n", internal_xrom_table[i].module_id,
				internal_xrom_table[i].function_name);
		}
	}
}

void dump_internal_xrom_module_one(int id)
{
	int i, n, functions;

	functions = 0;
	for (i = 0; i < internal_xrom_entries; ++i) {
		if (internal_xrom_table[i].module_id == id) {
			if (internal_xrom_table[i].function_id >= MAX_XROM_FUNCTIONS)
				printf("XROM Module[ %d ]: %s\n", id, internal_xrom_table[i].function_name);
			else {
				++functions;
				printf(" %s", internal_xrom_table[i].function_name);
				n = (int)strlen(internal_xrom_table[i].function_name);
				while (n++ < MAX_XROM_NAME)
					printf(" ");
				printf("%02d,%02d\n", id, internal_xrom_table[i].function_id);
			}
		}
	}

	printf("XROM Module[ %d ]: %d functions supported.\n", id, functions);
}

int parse_xrom_files(void)
{
	/* use static on large arrays to avoid growing the stack */
	static char xrom_path[_MAX_PATH];
	FILE *fin;
	int i, f_error;
	struct stat f_stats;
	size_t f_size, f_block;

	for (i = 0; i < xrom_file_count; ++i) {
		if (xrom_file_path[i]) {
			f_error = get_file_path(xrom_path, xrom_file_path[i], ".txt");
			if (f_error)
				return f_error;

			fin = open_input(xrom_path, xrom_file_path[i]);
			if (fin == NULL)
				return HP41_ERROR;

			f_error = fstat(_fileno(fin), &f_stats);
			if (f_error) {
				printf("Error getting size of XROM file[ %s ]\n", xrom_path);
				fclose(fin);
				return f_error;
			}

			f_size = (size_t)f_stats.st_size;
			f_block = sizeof(buf_1024);
			do {
				if (f_block > f_size)
					f_block = f_size;
				if (fread(buf_1024, 1, f_block, fin) != f_block) {
					printf("Error reading XROM file[ %s ]\n", xrom_path);
					fclose(fin);
					return HP41_ERROR;
				}
				f_error = parse_xrom_buffer((char *)buf_1024, f_block);
				if (f_error)
					printf("Error parsing XROM file[ %s ]\n", xrom_path);
				f_size -= f_block;
			} while (f_size && !f_error);

			fclose(fin);
			if (f_error) {
				return f_error;
			}
		}
	}

	return HP41_OK;
}

int parse_xrom_buffer(char *buffer, size_t count)
{
	static int source_line = 0;
	static int line_index = 0;
	static int line_end = HP41_FALSE;
	static unsigned char *line_ptr;
	static unsigned char *line_argv[XROM_ARGS];
	static unsigned char line_buffer[XROM_LINE];
	static unsigned char error_line[XROM_LINE];
	static XROM_STATE state = XROM_SEEK_START_LINE;
	static int xrom_index = 0;
	EXTERNAL_XROM *xrom;
	int parse_error = HP41_OK;
	int line_argc;
	char *endptr;
	char *name;
	int mm, ff;
	char c;

	while (!parse_error && (count || state == XROM_LINE_OUTPUT)) {
		switch (state) {
		case XROM_SEEK_START_LINE:
			c = *buffer++;
			--count;
			if (c == '\n' || c == '\f' || c == 0x1A) {
				++source_line;
			}
			else if (c != '\r') {
				line_buffer[line_index++] = c;
				state = XROM_GET_LINE;
			}
			break;

		case XROM_GET_LINE:
			c = *buffer++;
			--count;
			if (c == '\n' || c == '\f' || c == 0x1A) {
				line_end = HP41_TRUE;
			}
			else if (c == '\r') {
				state = XROM_SEEK_END_LINE;
			}
			else if (line_index == XROM_LINE) {
				line_index = 0;
				++source_line;
				printf("XROM Error: line too long!\n");
				state = XROM_IGNORE_BAD_LINE;
				parse_error = HP41_ERROR;
			}
			else {
				line_buffer[line_index++] = c;
			}
			break;

		case XROM_SEEK_END_LINE:
			c = *buffer++;
			--count;
			if (c == '\n' || c == '\f' || c == 0x1A) {
				line_end = HP41_TRUE;
			}
			else if (c != '\r') {
				line_index = 0;
				++source_line;
				printf("XROM Error: found[ %c ] expecting end-of-line character.\n", c);
				state = XROM_IGNORE_BAD_LINE;
				parse_error = HP41_ERROR;
			}
			break;

		case XROM_LINE_OUTPUT:
			/* parse line arguments */
			strcpy((char *)error_line, (char *)line_ptr);
			if ((line_argc = get_line_args(line_argv, &line_ptr, HP41_FALSE, XROM_ARGS))) {
				if (xrom_index < external_xrom_entries) {
					if (line_argc < XROM_ARGS || strlen((char *)line_ptr)) {
						parse_error = HP41_ERROR;
					}
					else {
						name = get_xrom_function((char *)line_argv[0]);
						mm = (int)strtoul((char *)line_argv[1], &endptr, 10);
						ff = (int)strtoul((char *)line_argv[2], &endptr, 10);

						if (name && mm && mm < MAX_XROM_MODULES && ff < MAX_XROM_FUNCTIONS) {
							xrom = &external_xrom_table[xrom_index++];
							strcpy(xrom->function_name, name);
							xrom->module_id = mm;
							xrom->function_id = ff;
						}
						else {
							parse_error = HP41_ERROR;
						}
					}
					if (parse_error) {
						printf("XROM Error: invalid arguments[ %s ]\n", error_line);
						state = XROM_SEEK_START_LINE;
					}
				}
				else {
					printf("XROM Error: file too long!\n");
					state = XROM_SEEK_START_LINE;
					parse_error = HP41_ERROR;
				}
			}
			else {
				/* nothing to do in this line, let's contuinue */
				state = XROM_SEEK_START_LINE;
			}
			break;

		case XROM_IGNORE_BAD_LINE:
			c = *buffer++;
			--count;
			/* end-of-line? */
			if (c == '\n' || c == '\f' || c == 0x1A) {
				state = XROM_SEEK_START_LINE;
			}
			break;
		}

		/* end-of-line? */
		if (line_end) {
			line_buffer[line_index] = '\0';
			line_ptr = line_buffer;
			line_index = 0;
			line_end = HP41_FALSE;
			++source_line;
			state = XROM_LINE_OUTPUT;
		}

		/* any errors? */
		if (parse_error) {
			printf("XROM error on line %d.\n", source_line);
		}
	}

	return parse_error;
}

char *get_xrom_function(char *name)
{
	char *str;
	int i, n;

	str = name;
	if (name) {
		n = strlen(name);
		/* remove quotes */
		if (n > 1 && name[0] == '\"' && name[n - 1] == '\"') {
			name[n - 1] = '\0';
			++str;
			n -= 2;
		}

		/* invalid length? */
		if (n == 0 || n >= MAX_XROM_NAME) {
			str = NULL;
		}
		else {
			/* invalid chracters? */
			for (i = 0; i < n && str; ++i) {
				if (str[i] < 0x20 ||
					str[i] > 0x7E ||
					str[i] == ',' ||
					str[i] == '.' ||
					str[i] == ':') {
					str = NULL;
				}
			}
		}
	}

	return str;
}

int hextoascii(unsigned char *ascii_buffer,
	unsigned char *hex_buffer, int hex_count)
{
	int i, j, k;

	for (i = 0, j = 0; i < hex_count; ++i, j += 2) {
		k = hex_buffer[i] >> 4;
		ascii_buffer[j] = k > 9 ? k + 'A' - 10 : k + '0';
		k = hex_buffer[i] & 0x0F;
		ascii_buffer[j + 1] = k > 9 ? k + 'A' - 10 : k + '0';
	}

	return(j);
}

int asciitohex(unsigned char *hex_buffer,
	unsigned char *ascii_buffer, int ascii_count)
{
	int i, j, k, n;

	for (i = 0, j = 0; i<ascii_count; i += 2, ++j) {
		k = toupper(ascii_buffer[i]);
		n = k > '9' ? k - 'A' + 10 : k - '0';
		n <<= 4;
		k = toupper(ascii_buffer[i + 1]);
		n += k > '9' ? k - 'A' + 10 : k - '0';
		hex_buffer[j] = n;
	}

	return(j);
}

int get_line_args(unsigned char *line_argv[], unsigned char **line_ptr, int comma_as_end, int max_args)
{
	unsigned char *pc;
	int i, j, count, done;

	count = 0;
	pc = *line_ptr;
	done = (*pc == '\0');
	while (!done) {
		/* ignore leading spaces */
		while (*pc == '\t' || *pc == 0x20)
			++pc;

		/* ignore comment line */
		if (*pc == ';' ||
			*pc == '#' ||
			*pc == '\0') {
			done = HP41_TRUE;
		}
		else {
			/* get argument */
			i = 0;
			do {
				/* consider quotes as a unit */
				j = is_inquotes(&pc[i]);
				i += j + 1;

				/* end of line? */
				if (pc[i] == '\0') {
					done = 2;
				}
				else if (pc[i] == ',') {
					/* end of instruction? */
					if (comma_as_end) {
						if (pc[i] == '\t' || pc[i] == 0x20) {
							pc[i] = '\0';
							done = HP41_TRUE;
						}
					}
					/* end of argument */
					else {
						pc[i] = '\0';
					}
				}
				/* end of argument? */
				else if (pc[i] == '\t' || pc[i] == 0x20) {
					pc[i] = '\0';
				}
			} while (pc[i] != '\0');

			/* put argument in list */
			line_argv[count++] = pc;

			/* point to next argument */
			pc += strlen((char *)pc);
			if (done != 2)
				++pc;

			/* full list? */
			if (count == max_args) {
				done = HP41_TRUE;
			}
		}
	}

	/* update line pointer */
	*line_ptr = pc;

	return(count);
}

int is_inquotes(unsigned char *buffer)
{
	int i, j;

	j = (int)strlen((char *)buffer);
	if (buffer[0] == '\"' || buffer[0] == '\'') {
		for (i = 1; i < j; ++i) {
			if (buffer[i] == buffer[0] &&
				buffer[i - 1] != '\\') {
				if (buffer[i + 1] == buffer[0])
					++i;
				else if (buffer[i + 1] == '\0' ||
					buffer[i + 1] == '\t' ||
					buffer[i + 1] == 0x20) {
					return(i);
				}
			}
		}
	}

	return 0;
}

int find_input_files(FIND_FILE *ff, char *in_dir, char *in_file, char *in_ext)
{
	/* use static on large arrays to avoid growing the stack */
	static char path[_MAX_PATH];
	int files = 1;

	/* valid input path? */
	findfile_init(ff);
	if (file_fullpath(path, in_file) == NULL) {
		printf("Error: invalid input path[ %s ]\n", in_file);
		return 0;
	}

	/* find FIRST file, allow wildcards */
	strcpy(in_dir, path);
	if (findfile_first(in_dir, ff) == HP41_ERROR) {
		strcat(in_dir, in_ext);
		if (findfile_first(in_dir, ff) == HP41_ERROR) {
			strcpy(in_dir, path);
			strcat(in_dir, "\\*.*");
			if (findfile_first(in_dir, ff) == HP41_ERROR) {
				files = 0;
			}
		}
	}

	/* find NEXT files */
	if (files) {
		/* count number of files in directory */
		while (findfile_next(ff) == HP41_OK)
			++files;

		/* rewind search to the beginning */
		findfile_close(ff);
		if (findfile_first(in_dir, ff) == HP41_ERROR) {
			files = 0;
		}
	}

	/* found any files? */
	if (!files)
		printf("Error: could not find input file[ %s ]\n", in_file);

	return(files);
}

int get_file_path(char *io_path, char *io_file, char *io_ext)
{
	if (file_fullpath(io_path, io_file) == NULL) {
		printf("Error: invalid file path[ %s ]\n", io_file);
		return HP41_ERROR;
	}

	/* use default .ext? */
	if (file_access(io_path, FILE_EXIST) != HP41_OK && io_ext) {
		file_splitpath(io_path, drive, dir, fname, ext);
		if (*ext == '\0') {
			strcat(io_path, io_ext);
		}
	}

	return HP41_OK;
}

int exists_as_directory(char *path)
{
	FILE *file;

	if (file_access(path, FILE_EXIST) != HP41_ERROR &&
		file_access(path, FILE_WRITE_OK) != HP41_ERROR) {
		/* try to open it as a file */
		if ((file = fopen(path, "w+b")) == NULL)
			return HP41_OK;
		fclose(file);
	}

	return HP41_ERROR;
}

FILE *open_input_ex(char *inpath, long inlength, char *infile, char *outpath)
{
	if (outpath && _stricmp(inpath, outpath) == HP41_OK) {
		printf("Error: input file cannot be output file[ %s ]\n",
			outpath);
		return NULL;
	}

	if (inlength <= 0) {
		printf("Error: empty input file[ %s ]\n", inpath);
		return NULL;
	}

	return open_input(inpath, infile);
}

FILE *open_input(char *inpath, char *infile)
{
	FILE *fin;

	if ((fin = fopen(inpath, "rb")) == NULL) {
		printf("Error opening input file[ %s ]", inpath);
		if (_stricmp(inpath, infile) != HP41_OK)
			printf(", from[ %s ]", infile);
		printf("\n");
	}

	return(fin);
}

FILE *open_output(char *outpath)
{
	FILE *fout = NULL;

	//if (oktowrite(outpath)) {
		if ((fout = fopen(outpath, "w+b")) == NULL) {
			printf("Error opening output file[ %s ]\n", outpath);
		}
	//}

	return(fout);
}

void closefiles(FILE *fin, FILE *fout, long outlength, int header)
{
	if (fin)
		fclose(fin);
	if (fout) {
		fclose(fout);
		if (outlength <= header) {
			outlength = 0;
			remove(outpath);
		}
		printf("%d bytes written.\n", outlength);
	}
}

int oktowrite(char *path)
{
	int c;

	if (file_access(path, FILE_EXIST) != HP41_ERROR) {
		if (file_access(path, FILE_WRITE_OK) != HP41_ERROR) {
			printf("Overwrite %s (Yes/[No])?", path);
			do {
				c = _getch();
				if (c == 'y' || c == 'Y') {
					printf("%c\n", c);
					return HP41_TRUE;
				}
				/* function or cursor key? */
				else if (c == 0x00 || c == 0xE0) {
					/* 2-character sequence */
					_getch();
				}
#ifdef __GNUC__
				else if (c == 0x1B) {
					/* 3-character sequence */
					_getch();
					_getch();
				}
#endif
				else {
					printf("%c", c);
					if (c != 0x0A) {
						printf("\n");
					}
					printf("No output file written.\n");
					return HP41_FALSE;
				}
			} while (1);
		}
		else {
			printf("No permission to write[ %s ]\n", path);
			return HP41_FALSE;
		}
	}

	return HP41_TRUE;
}

void terminate_directory(char *path)
{
	char *end;

	if (path && *path != '\0') {
		end = path + strlen(path) - 1;
		if (*end != '\\' && *end != '/') {
#ifdef __GNUC__
			strcat(path, "/");
#else
			strcat(path, "\\");
#endif
		}
	}
}

void findfile_init(FIND_FILE *ff)
{
#ifdef __GNUC__
	if (ff != NULL) {
		ff->phandle = NULL;
	}
#else	/* _MSC_VER */
	if (ff != NULL) {
		ff->phandle = (void *)HP41_LONG_ERROR;
	}
#endif
}

int findfile_first(char *path, FIND_FILE *ff)
{
#ifdef __GNUC__
	/* NOTE: This is just a partial implementation of _findfirst() */
	/* For example: there's limited support for wildcard search */
	struct dirent *ffd;
	struct stat f_stats;
	DIR *pdir, *f_pdir;
	int check_path_end = HP41_FALSE;
	/* use static on large arrays to avoid growing the stack */
	static char f_path[_MAX_PATH];
	static char f_dir[_MAX_PATH];

	if (ff == NULL)
		return HP41_ERROR;

	if (path != NULL) {
		to_unix_path(path);

		/* check for wildcard on fname */
		ff->data.file_ext[0] = '\0';
		file_splitpath(path, drive, dir, fname, ext);
		if (strcmp(fname, "*") == HP41_OK) {
			strcpy(f_path, drive);
			strcat(f_path, dir);
			strcpy(f_dir, f_path);
			if (strcmp(ext, ".*") != HP41_OK)
				strcpy(ff->data.file_ext, ext);
		}
		else {
			strcpy(f_dir, path);
			check_path_end = HP41_TRUE;
		}

		/* search for directory */
		pdir = opendir(f_dir);
		if (pdir != NULL) {
			do {
				ffd = readdir(pdir);
				if (ffd != NULL && 
					strcmp(ffd->d_name, ".") != HP41_OK &&
					strcmp(ffd->d_name, "..") != HP41_OK) {
					/* ensure path is treated as a directory */
					if (check_path_end) {
						terminate_directory(path);
						check_path_end = HP41_FALSE;
					}

					/* save directory path for 'next' search */
					strcpy(ff->data.dir_path, f_dir);
					
					/* append filespec */
					strcpy(f_path, ff->data.dir_path);
					strcat(f_path, ffd->d_name);

					/* make sure this is not a subdirectoy */
					f_pdir = opendir(f_path);
					if (f_pdir != NULL) {
						closedir(f_pdir);
					}
					else if (file_access(f_path, FILE_EXIST) != HP41_ERROR &&
						stat(f_path, &f_stats) == HP41_OK && f_stats.st_size &&
						matching_file_ext(f_path, ff->data.file_ext) == HP41_OK) {
						/* found first file in directory */
						ff->name = ffd->d_name;
						ff->size = f_stats.st_size;
						ff->phandle = (void *)pdir;
						return HP41_OK;
					}
				}
			} while (ffd != NULL);
			closedir(pdir);
		}
		/* search for file */
		else if (file_access(path, FILE_EXIST) != HP41_ERROR &&
			stat(path, &f_stats) == HP41_OK && f_stats.st_size) {
			/* found a file */
			file_splitpath(path, drive, dir, fname, ext);

			/* searching for specific file.ext? */
			if (ff->data.file_ext[0] == '\0' ||
				strcmp(ext, ff->data.file_ext) == HP41_OK) {
				strcpy(ff->data.file_spec, fname);
				strcat(ff->data.file_spec, ext);
				ff->name = ff->data.file_spec;
				ff->size = f_stats.st_size;
				ff->phandle = NULL;
				return HP41_OK;
			}
		}
	}

	ff->phandle = NULL;
	return HP41_ERROR;
#else	/* _MSC_VER */
	struct _finddata_t *ffd;
	intptr_t hFind;
	int ret = HP41_OK;

	if (ff == NULL)
		return HP41_ERROR;

	if (path != NULL) {
		ffd = (struct _finddata_t *)ff->data;
		hFind = _findfirst(path, ffd);
		if (hFind != HP41_LONG_ERROR) {
			while (ret == HP41_OK &&
				(ffd->size == 0 ||
				strcmp(ffd->name, ".") == HP41_OK ||
				strcmp(ffd->name, "..") == HP41_OK)) {
				ret = _findnext(hFind, ffd);
			} 
			if (ret != HP41_OK)
				_findclose(hFind);
			else {
				ff->name = ffd->name;
				ff->size = ffd->size;
				ff->phandle = (void *)hFind;
				return HP41_OK;
			}
		}
	}

	ff->phandle = (void *)HP41_LONG_ERROR;
	return HP41_ERROR;
#endif
}

int findfile_next(FIND_FILE *ff)
{
#ifdef __GNUC__
	/* NOTE: This is just a partial implementation of _findnext() */
	/* For example: there's limited support for wildcard search */
	struct dirent *ffd;
	struct stat f_stats;
	DIR *pdir, *f_pdir;
	/* use static on large arrays to avoid growing the stack */
	static char f_path[_MAX_PATH];

	if (ff != NULL) {
		pdir = (DIR *)ff->phandle;
		if (pdir != NULL) {
			do {
				ffd = readdir(pdir);
				if (ffd != NULL && 
					strcmp(ffd->d_name, ".") != HP41_OK &&
					strcmp(ffd->d_name, "..") != HP41_OK) {
					/* append filespec */
					strcpy(f_path, ff->data.dir_path);
					strcat(f_path, ffd->d_name);

					/* make sure this is not a subdirectoy */
					f_pdir = opendir(f_path);
					if (f_pdir != NULL)
						closedir(f_pdir);
					else if (file_access(f_path, FILE_EXIST) != HP41_ERROR &&
						stat(f_path, &f_stats) == HP41_OK && f_stats.st_size &&
						matching_file_ext(f_path, ff->data.file_ext) == HP41_OK) {
						/* found next file in directory */
						ff->name = ffd->d_name;
						ff->size = f_stats.st_size;
						return HP41_OK;
					}
				}
			} while (ffd != NULL);
		}
	}
	return HP41_ERROR;
#else	/* _MSC_VER */
	struct _finddata_t *ffd;
	intptr_t hFind;
	int ret;

	if (ff != NULL) {
		hFind = (intptr_t)ff->phandle;
		if (hFind != HP41_LONG_ERROR) {
			ffd = (struct _finddata_t *)ff->data;
			do {
				ret = _findnext(hFind, ffd);
			} while (ret == HP41_OK &&
				(ffd->size == 0 ||
				strcmp(ffd->name, ".") == HP41_OK ||
				strcmp(ffd->name, "..") == HP41_OK));
			if (ret == HP41_OK) {
				ff->name = ffd->name;
				ff->size = ffd->size;
			}
			return ret;
		}
	}
	return HP41_ERROR;
#endif
}

void findfile_close(FIND_FILE *ff)
{
#ifdef __GNUC__
	DIR *pdir;

	if (ff != NULL) {
		pdir = (DIR *)ff->phandle;
		if (pdir != NULL) {
			closedir(pdir);
			ff->phandle = NULL;
		}
	}
#else	/* _MSC_VER */
	intptr_t hFind;

	if (ff != NULL) {
		hFind = (intptr_t)ff->phandle;
		if (hFind != HP41_LONG_ERROR) {
			_findclose(hFind);
			ff->phandle = (void *)HP41_LONG_ERROR;
		}
	}
#endif
}

#ifdef __GNUC__
/* reads from keypress, doesn't echo */
int _getch(void)
{
	struct termios oldattr, newattr;
	int ch;

	tcgetattr( STDIN_FILENO, &oldattr );
	newattr = oldattr;
	newattr.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
	return ch;
}

/* reads from keypress, echoes */
int _getche(void)
{
	struct termios oldattr, newattr;
	int ch;

	tcgetattr( STDIN_FILENO, &oldattr );
	newattr = oldattr;
	newattr.c_lflag &= ~( ICANON );
	tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
	return ch;
}

char *_strupr(char *str)
{
	char *ret = str;
	for ( ; *str; str++) 
		*str = toupper(*str);
	return ret;
}

int _fileno(FILE *stream)
{
	return fileno(stream);
}

int _stricmp(const char * s1, const char *s2)
{
	return strcasecmp(s1, s2);
}

int _strnicmp(const char * s1, const char *s2, size_t n)
{
	return strncasecmp(s1, s2, n);
}
#endif

long get_filelength(int fd)
{
#ifdef __GNUC__
	struct stat f_stats;

	fstat(fd, &f_stats);
	return (long)f_stats.st_size;
#else	/* _MSC_VER */
	return _filelength(fd);
#endif
}

#ifdef __GNUC__
char *to_unix_path(char *p)
{
	if (p != NULL) {
		char *pp = p;
		while (*pp != '\0') {
			if (*pp == '\\')
				*pp = '/';
			++pp;
		}
	}
	return p;
}
#endif

int file_access(
	const char *path,
	FILE_MODE mode
	)
{
#ifdef __GNUC__
	return access(to_unix_path((char *)path), (int)mode);
#else	/* _MSC_VER */
	return _access(path, (int)mode);
#endif
}

char *file_fullpath(
	char *absPath,
	const char *relPath
	)
{
#ifdef __GNUC__
	char *retPath = NULL;
#ifdef __DJGPP__
	if (absPath) {
		*absPath = '\0';
		retPath = realpath(to_unix_path((char *)relPath), absPath);
		if (retPath == NULL && errno == ENOENT && *absPath != '\0')
			retPath = absPath;
	}
#else
	/* use static on large arrays to avoid growing the stack */
	static char path[_MAX_PATH];
	wordexp_t exp;

	if (absPath) {
		if (relPath[0] == '~') {
			wordexp("~", &exp, 0);
			strcpy(path, exp.we_wordv[0]);
			strcat(path, &relPath[1]);
		}
		else {
			strcpy(path, relPath);
		}

		*absPath = '\0';
		retPath = realpath(to_unix_path((char *)path), absPath);
		if (retPath == NULL && errno == ENOENT && *absPath != '\0')
			retPath = absPath;
	}
#endif
	return retPath;
#else	/* _MSC_VER */
	return _fullpath(absPath, relPath, _MAX_PATH);
#endif
}

int matching_file_ext(char *cur_path, char *new_ext)
{
	if (new_ext[0] != '\0') {
		file_splitpath(cur_path, drive, dir, fname, ext);
		return strcmp(ext, new_ext);
	}

	return HP41_OK;
}

void getfullpath(char *fullpath, char *dirpath, char *filespec)
{
	file_splitpath(dirpath, drive, dir, fname, ext);
	strcpy(fullpath, drive);
	strcat(fullpath, dir);
	strcat(fullpath, filespec);
}

void file_splitpath(
	const char *path,
	char *drv,
	char *dir,
	char *name,
	char *ext
	)
{
	const char* end; /* end of processed string */
	const char* p;   /* search pointer */
	const char* s;   /* copy pointer */
	int i;

	/* extract drive name */
	if (path[0] && path[1] == ':') {
		if (drv) {
			drv[0] = path[0];
			drv[1] = path[1];
			drv[2] = '\0';
		}
		path += 2;
	}
	else if (drv) {
		*drv = '\0';
	}

	end = path + strlen(path);

	/* search for begin of file extension */
	for (p = end; p > path && *--p != '\\' && *p != '/';) {
		if (*p == '.') {
			end = p;
			break;
		}
	}

	if (ext) {
		i = 0;
		s = end;
		while (*s != '\0') {
			ext[i++] = *s++;
		}
		ext[i] = '\0';
	}

	/* search for end of directory name */
	for (p = end; p > path;) {
		if (*--p == '\\' || *p == '/') {
			p++;
			break;
		}
	}

	if (name) {
		i = 0;
		s = p;
		while (*s != '\0' && s < end) {
			name[i++] = *s++;
		}
		name[i] = '\0';
	}

	if (dir) {
		i = 0;
		s = path;
		while (s < p) {
			dir[i++] = *s++;
		}
		dir[i] = '\0';
	}
}

void help(int do_help)
{
	int decompiler = HP41_FALSE;

	switch (do_help) {
	case 1:
		printf("User-Code File Converter/Compiler/De-compiler/Barcode Generator - Version 3.00\n");
		printf("+ DM41 Extensions.\n");
		printf("Copyright (c) Leo Duran, 2000-2020. All rights reserved. leo.duran@yahoo.com.\n\n");
		printf("Supported File Formats:\n");
		printf("  LIF [ /l ]: transfer file for Trans41\n");
		printf("  P41 [ /p ]: archive file for HP-41 programs on ftp sites\n");
		printf("  RAW [ /r ]: input/output file for V41 abd LIFUTILS\n");
		printf("  DAT [ /d ]: input/output file for EMU41 (using INP, OUTP)\n");
		printf("  BIN [ /b ]: output file from 41UCC\n");
		printf("  TXT [ /t ]: program listing (text)\n");
		printf("  PCL [ /h ]: barcode printable file (Hewlett Packard PCL)\n");
		printf("  PS  [ /s ]: barcode printable file (Postcript)\n");
		printf("  WND [ /w ]: barcode hex-dump (Wand file)\n\n");
		printf("File Converter:\n");
		printf("  [ LIF, P41, RAW, DAT, BIN ] => [ BIN, DAT, RAW, P41, LIF ]\n");
		printf("Compiler:\n");
		printf("  [ TXT ] => [ LIF, P41, RAW, DAT, BIN ]\n");
		printf("De-compiler:\n");
		printf("  [ LIF, P41, RAW, DAT, BIN ] => [ TXT ]\n");
		printf("Barcode Generator:\n");
		printf("  [ RAW ] => [ PCL, PS, WND ]\n");
		printf("Use a single option[ /b, /d, /h, /l, /p, /r, /s, /t, or /w ] for help.\n");
		break;

	case 'B':
		printf("BIN Format:\n");
		printf("[2-byte header] + [compiled code] + [1-byte checksum] + [trailer]\n");
		printf("  [2-byte header] - size of [compiled code] in big-endian format\n");
		printf("  [1-byte checksum] - sum of all [compiled code]+[2-byte header]\n");
		printf("  [trailer] - filler bytes, so that filesize = multiple 128-byte blocks\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");
		printf(" BIN file(s) to LIF file ( LIF name = <name> or [infile] )\n");
		printf("   /b=infile<.bin>  /l<=outfile<.lif>>  <name>\n\n");
		printf(" BIN file to P41 file ( P41 name = <name> or [infile] )\n");
		printf("   /b=infile<.bin>  /p<=outfile<.p41>>  <name>\n\n");
		printf(" BIN file to RAW file ( remove 2-byte header )\n");
		printf("   /b=infile<.bin>  /r<=outfile<.raw>> </k>\n");
		printf("  Use /k option to exclude checksum and trailer bytes in RAW file\n\n");
		printf(" BIN file to DAT file ( hex digits in ASCII format )\n");
		printf("   /b=infile<.bin>  /d<=outfile<.dat>>\n\n");
		printf(" BIN file to TXT file ( de-compile )\n");
		printf("   /b=infile<.bin>  /t<=outfile<.txt>> </a> </n> </x> </m=file<.txt>> \n");

		decompiler = HP41_TRUE;
		break;

	case 'D':
		printf("DAT Format:\n");
		printf("[4-byte header] + [compiled code] + [2-byte checksum]\n");
		printf("  [4-byte header] - size of [compiled code] in big-endian format\n");
		printf("  [2-byte checksum] - sum of [compiled code]+size of [compiled code]\n");
		printf(" All bytes are hex digits in ASCII format.\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");
		printf(" DAT file(s) to LIF file ( LIF name = <name> or [infile] )\n");
		printf("   /d=infile<.dat>  /l<=outfile<.lif>>  <name>\n\n");
		printf(" DAT file to P41 file ( P41 name = <name> or [infile] )\n");
		printf("   /d=infile<.dat>  /p<=outfile<.p41>>  <name>\n\n");
		printf(" DAT file to RAW file ( remove 2-byte header )\n");
		printf("   /d=infile<.dat>  /r<=outfile<.raw>> </k>\n");
		printf("  Use /k option to exclude checksum and trailer bytes in RAW file\n\n");
		printf(" DAT file to TXT file ( de-compile )\n");
		printf("   /d=infile<.dat>  /t<=outfile<.txt>> </a> </n> </x> </m=file<.txt>> \n");

		decompiler = HP41_TRUE;
		break;

	case 'L':
		printf("LIF Format:\n");
		printf("HP-IL Files: Where, What, Why, How, etc.:\n https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/articles.cgi?read=24\n");
		printf("The HPDir Project:\n http://www.hp9845.net/9845/projects/hpdir/\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");
		printf(" LIF file to P41 file ( search LIF name: <name>, <outfile> or [infile] )\n");
		printf("   /l=infile<.lif>  /p<=outfile<.p41>>  <name>\n\n");
		printf(" LIF file to RAW file\n");
		printf("   /l=infile<.lif>  /r<=outfile<.raw>>  <name>  </k>\n");
		printf("  Use /k option to exclude checksum and trailer bytes in RAW file\n\n");
		printf(" LIF file to DAT file ( hex digits in ASCII format )\n");
		printf("   /l=infile<.lif>  /d<=outfile<.dat>>  <name>\n\n");
		printf(" LIF directory listing ( search for <name> only, if specified )\n");
		printf("   /l=infile<.lif>  <name>\n\n");
		printf(" LIF file to TXT file ( de-compile )\n");
		printf("   /l=infile<.lif>  /t<=outfile<.txt>>  <name> </a> </n> </x> </m=file<.txt>> \n");

		decompiler = HP41_TRUE;
		break;

	case 'P':
		printf("P41 Format:\n");
		printf("[32-byte header] + [compiled code] + [1-byte checksum] + [trailer]\n");
		printf("  [32-byte header] - single LIF directory entry for the program\n");
		printf("  [1-byte checksum] - sum of all [compiled code] bytes\n");
		printf("  [trailer] - filler bytes, so that filesize = 32 + multiple 256-byte blocks\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");
		printf(" P41 file(s) to LIF file\n");
		printf("   /p=infile<.p41>  /l<=outfile<.lif>>\n\n");
		printf(" P41 file to RAW file ( remove 32-byte header )\n");
		printf("   /p=infile<.p41>  /r<=outfile<.raw>> </k>\n");
		printf("  Use /k option to exclude checksum and trailer bytes in RAW file\n\n");
		printf(" P41 file to DAT file ( hex digits in ASCII format )\n");
		printf("   /p=infile<.p41>  /d<=outfile<.dat>>\n\n");
		printf(" P41 file to TXT file ( de-compile )\n");
		printf("   /p=infile<.p41>  /t<=outfile<.txt>> </a> </n> </x> </m=file<.txt>> \n");

		decompiler = HP41_TRUE;
		break;

	case 'R':
		printf("RAW Format:\n");
		printf("[compiled code] + [1-byte checksum] + [trailer]\n");
		printf("  [1-byte checksum] - sum of all [compiled code] bytes\n");
		printf("  [trailer] - filler bytes, so that filesize = multiple 256-byte blocks\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");
		printf(" RAW file(s) to LIF file ( LIF name = <name> or [infile] )\n");
		printf("   /r=infile<.raw>  /l<=outfile<.lif>  <name>\n\n");
		printf(" RAW file to P41 file ( P41 name = <name> or [infile] ) \n");
		printf("   /r=infile<.raw>  /p<=outfile<.p41>>  <name>\n\n");
		printf(" RAW file to DAT file ( hex digits in ASCII format )\n");
		printf("   /r=infile<.raw>  /d<=outfile<.dat>>\n\n");
		printf(" RAW file to TXT file ( de-compile )\n");
		printf("   /r=infile<.raw>  /t<=outfile<.txt>> </a> </n> </x> </m=file<.txt>> \n");

		decompiler = HP41_TRUE;
		break;

	case 'T':
		printf("TXT Format - Compiler:\n");
		printf(" Prefixes are case-insensitive: [ rcl 00 ] same as [ RCL 00 ]\n");
		printf(" Postfixes are case-sensitive: [ lbl a ] different than [ lbl A ]\n");
		printf(" Append text is allowed using: [ >\"text\" ],  [ \"|-text\" ], or [ \"\\-text\" ]\n");
		printf(" Synthetic instructions are allowed: [ rcl M ], [ sto R ]\n");
		printf(" Synthetic NOP (TEXT0) is allowed using an empty string: [ \"\" ]\n");
		printf(" Synthetic strings are allowed using C-style esc-sequences: [ \"\\x0D\\x0A\" ]\n");
		printf(" Comments are allowed if preceded by [ ; ] or [ # ]: [ # comment ]\n");
		printf(" [ /n ] - ignore line numbers: [ 100 STO 00 ]\n");
		printf(" [ /g ] - global label for[ \"A..J\", \"a..e\" ] if in quotes: [ XEQ \"A\" ]\n");
		printf(" [ /x ] - exclude all supported XROM modules (may require [ XROM mm,ff ])\n");
		printf(" [ /x## ] - exclude supported XROM module 1..31 (may require [ XROM mm,ff ])\n");
		printf(" [ /m=file<.txt> ] - import functions from XROM modules in specified text file\n");
		printf("\nUsage: optional parameters are surrounded by <>\n\n");
		printf(" TXT file(s) to LIF file ( LIF name = <name> or [infile] )\n");
		printf("   /t=infile<.txt>  /l<=outfile<.lif>>  <name> </n> </g> </x> </m=file<.txt>>\n\n");
		printf(" TXT file to P41 file ( P41 name = <name> or [infile] )\n");
		printf("   /t=infile<.txt>  /p<=outfile<.p41>>  <name> </n> </g> </x> </m=file<.txt>>\n\n");
		printf(" TXT file to RAW file\n");
		printf("   /t=infile<.txt>  /r<=outfile<.raw>> </k> </n> </g> </x> </m=file<.txt>> \n\n");
		printf(" TXT file to DAT file ( hex digits in ASCII format )\n");
		printf("   /t=infile<.txt>  /d<=outfile<.dat>> </n> </g> </x> </m=file<.txt>> \n");
		break;

	case 'W':
		printf("Barcode Generator:\n");
		printf("Reference: \"Creating Your Own HP-41 Barcode\"\n\n");
		printf("Usage: optional parameters are surrounded by <>\n\n");

		if (bc_printer == PRINTER_HP) {
			printf(" RAW file to PCL (HP raster graphics) file\n");
			printf("   /r=infile<.raw>  /h<=outfile<.pcl>>  <\"Program Title\">\n\n");
			printf("To print the barcode, copy the output file to the printer device.\n");
			printf("Example: hp41uc /r=prog.raw /h \"Program Title\"\n");
			printf("         copy /b prog.pcl lpt1\n");
		}
		else if (bc_printer == PRINTER_POSTCRIPT) {
			printf(" RAW file to Postcript file\n");
			printf("   /r=infile<.raw>  /s<=outfile<.ps>>  <\"Program Title\">\n\n");
			printf("To print the barcode, copy the output file to the printer device.\n");
			printf("Example: hp41uc /r=prog.raw /s \"Program Title\"\n");
			printf("         copy /b prog.ps lpt1\n");
		}
		else {
			printf(" RAW file to Wand (barcode) file\n");
			printf("   /r=infile<.raw>  /w<=outfile<.wnd>>  <\"Program Title\">\n\n");
			printf("The output file will be a 16-byte per row hex-dump in text format.\n\n");
		}
		break;

	default:
		break;
	}

	if (decompiler) {
		printf("\nDe-compiler Options:\n");
		printf("  /a - append text using: [ \"|-text\" ], instead of: [ >\"text\" ]\n");
    printf("  /f - skip line-feed (0x0A) characters on DAT files\n");
		printf("  /n - generate line numbers\n");
		printf("  /x - exclude all supported XROM modules (may generate [ XROM mm,ff ])\n");
		printf("  /x## - exclude supported XROM module 1..31 (may generate [ XROM mm,ff ])\n");
		printf("  /m=file<.txt> - import functions from XROM modules in specified text file\n");
	}
}

