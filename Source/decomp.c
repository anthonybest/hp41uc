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
#include "decomp.h"

static char pKey[] = " Key: rc";
static char psKey[] = " Key: -rc";
static unsigned char synth_buffer[18];
static unsigned char buffer6[6];

static int end = 0;
static int line = 1;
static int numeric = HP41_FALSE;
static DECOMPILE_STATE state = BYTE1;
static SEEK_STATE seek_state = SEEK_BYTE1;

static int count;
static int synth_flag;
static int synth_count;
static int alpha_count;

static int consumed;
static int produced;
static unsigned char *in_buffer;
static unsigned char *out_buffer;

void decompile_init(void)
{
	end = 0;
	line = 1;
	numeric = HP41_FALSE;
	state = BYTE1;
	seek_state = SEEK_BYTE1;
}

int decompile(unsigned char *pout_buffer, int out_size,
	unsigned char **pin_buffer, int *pin_count,
	int *ppending, int *pend)
{
	int i, j, mm, ff;
	unsigned char c;
	char *xrom;
	int do_xrom;
	int done = HP41_FALSE;

	consumed = 0;
	produced = 0;
	in_buffer = *pin_buffer;
	out_buffer = pout_buffer;

	do {
		switch (state) {
		case BYTE1:
			c = *in_buffer;
			if (c >= 0x10 && c <= 0x1C) {
				i = c - 1;
				if (copy_prefix((char *)single01_1C[i], out_size)) {
					numeric = HP41_TRUE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (numeric) {
				numeric = HP41_FALSE;
				state = APPEND_CR;
			}
			else if (c == 0x00) {
				++consumed;
				++in_buffer;
			}
			else if (c >= 0x01 && c <= 0x0F) {
				i = c - 1;
				if (copy_prefix((char *)single01_1C[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c == 0x1D) {
				if (copy_prefix((char *)prefixGTO, out_size)) {
					synth_flag = HP41_FALSE;
					synth_count = 1;
					synth_buffer[0] = c;
					state = ALPHA_OPEN_QUOTE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c == 0x1E) {
				if (copy_prefix((char *)prefixXEQ, out_size)) {
					synth_flag = HP41_FALSE;
					synth_count = 1;
					synth_buffer[0] = c;
					state = ALPHA_OPEN_QUOTE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c == 0x1F) {
				if (copy_prefix((char *)prefixW, out_size)) {
					synth_flag = HP41_FALSE;
					synth_count = 1;
					synth_buffer[0] = c;
					state = ALPHA_OPEN_QUOTE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0x20 && c <= 0x8F) {
				i = c - 0x20;
				if (copy_prefix((char *)single20_8F[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0x90 && c <= 0x9F) {
				i = c - 0x90;
				if (copy_prefix((char *)prefix90_9F[i], out_size)) {
					state = BYTE2_POSTFIX;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0xA0 && c <= 0xA7) {
				++in_buffer;
				++consumed;
				count = c & 0x07;
				state = BYTE2_XROM;
			}
			else if (c >= 0xA8 && c <= 0xAD) {
				i = c - 0xA8;
				if (copy_prefix((char *)prefixA8_AD[i], out_size)) {
					state = BYTE2_POSTFIX;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c == 0xAE) {
				++in_buffer;
				++consumed;
				state = BYTE2_IND;
			}
			else if (c == 0xAF) {
				++in_buffer;
				++consumed;
				synth_buffer[0] = c;
				state = BYTE2_IND_NOP;
			}
			else if (c == 0xB0) {
				++in_buffer;
				++consumed;
				synth_buffer[0] = c;
				state = BYTE2_NOP;
			}
			else if (c >= 0xB1 && c <= 0xBF) {
				i = c - 0xB1;
				if (copy_prefix((char *)prefixB1_BF[i], out_size)) {
					state = BYTE2_GTO_IGNORE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0xC0 && c <= 0xCD) {
				++in_buffer;
				++consumed;
				synth_flag = HP41_FALSE;
				synth_count = 1;
				synth_buffer[0] = c;
				state = BYTE2_GLOBAL_IGNORE;
			}
			else if (c == 0xCE || c == 0xCF) {
				i = c - 0xCE;
				if (copy_prefix((char *)prefixCE_CF[i], out_size)) {
					state = BYTE2_POSTFIX;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0xD0 && c <= 0xDF) {
				if (copy_prefix((char *)prefixGTO, out_size)) {
					state = BYTE2_OF3_IGNORE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else if (c >= 0xE0 && c <= 0xEF) {
				if (copy_prefix((char *)prefixXEQ, out_size)) {
					state = BYTE2_OF3_IGNORE;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else {
				++in_buffer;
				++consumed;
				count = c & 0x0F;
				synth_flag = HP41_FALSE;
				synth_count = 1;
				synth_buffer[0] = c;
				state = TEXT_OPEN_QUOTE;
			}
			break;

		case BYTE2_IND_NOP:
			c = *in_buffer;
			if (c < 0x80) {
				if (copy_prefix((char *)prefixGTO_IND_NOP, out_size)) {
					synth_flag = HP41_TRUE;
					synth_count = 2;
					synth_buffer[1] = c;
					state = APPEND_SYNTH;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else {
				if (copy_prefix((char *)prefixXEQ_IND_NOP, out_size)) {
					synth_flag = HP41_TRUE;
					synth_count = 2;
					synth_buffer[1] = c;
					state = APPEND_SYNTH;
				}
				else {
					done = HP41_TRUE;
				}
			}
			break;

		case BYTE2_NOP:
			c = *in_buffer;
			if (copy_prefix((char *)prefixGTO_NOP, out_size)) {
				synth_flag = HP41_TRUE;
				synth_count = 2;
				synth_buffer[1] = c;
				state = APPEND_SYNTH;
			}
			else {
				done = HP41_TRUE;
			}
			break;

		case BYTE2_GTO_IGNORE:
			++in_buffer;
			++consumed;
			state = APPEND_CR;
			break;

		case BYTE2_XROM:
			c = *in_buffer;
			mm = (count << 2) + (c >> 6);
			ff = c & 0x3F;
			done = HP41_TRUE;
			do_xrom = HP41_FALSE;

			sprintf((char *)buffer6, "%02d", mm);
			memcpy(&prefixXROM[5], buffer6, 2);
			sprintf((char *)buffer6, "%02d", ff);
			memcpy(&prefixXROM[8], buffer6, 2);
			strcpy((char *)synth_buffer, "  ;");
			strcat((char *)synth_buffer, (char *)prefixXROM);

			if (mm < MAX_XROM_MODULES && ff < MAX_XROM_FUNCTIONS) {
				for (i = 0; i < internal_xrom_entries && do_xrom == HP41_FALSE; ++i) {
					xrom = internal_xrom_table[i].function_name;
					if (is_enable_internal_xrom(mm) && strlen(xrom) &&
						mm == internal_xrom_table[i].module_id &&
						ff == internal_xrom_table[i].function_id) {
						do_xrom = HP41_TRUE;
						if (copy_prefix2(xrom, (char *)synth_buffer, out_size)) {
							done = HP41_FALSE;
							state = APPEND_CR;
						}
					}
				}
				for (i = 0; i < external_xrom_entries && do_xrom == HP41_FALSE; ++i) {
					xrom = external_xrom_table[i].function_name;
					if (strlen(xrom) &&
						mm == external_xrom_table[i].module_id &&
						ff == external_xrom_table[i].function_id) {
						do_xrom = HP41_TRUE;
						if (copy_prefix2(xrom, (char *)synth_buffer, out_size)) {
							done = HP41_FALSE;
							state = APPEND_CR;
						}
					}
				}

				if (do_xrom == HP41_FALSE) {
					if (copy_prefix((char *)prefixXROM, out_size)) {
						done = HP41_FALSE;
						state = APPEND_CR;
					}
				}
			}
			break;

		case BYTE2_IND:
			c = *in_buffer;
			if (c < 0x80) {
				i = c;
				if (copy_prefix2((char *)prefixGTO_IND, (char *)postfix00_7F[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else {
				i = c & 0x7F;
				if (copy_prefix2((char *)prefixXEQ_IND, (char *)postfix00_7F[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			break;

		case BYTE2_POSTFIX:
			c = *in_buffer;
			if (c < 0x80) {
				i = c;
				if (copy_postfix((char *)postfix00_7F[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			else {
				i = c & 0x7F;
				if (copy_postfix2((char *)postfixIND, (char *)postfix00_7F[i], out_size)) {
					state = APPEND_CR;
				}
				else {
					done = HP41_TRUE;
				}
			}
			break;

		case BYTE2_OF3_IGNORE:
			++in_buffer;
			++consumed;
			state = BYTE3_POSTFIX;
			break;

		case BYTE3_POSTFIX:
			c = *in_buffer;
			i = c & 0x7F;
			if (copy_postfix((char *)postfix00_7F[i], out_size)) {
				state = APPEND_CR;
			}
			else {
				done = HP41_TRUE;
			}
			break;

		case BYTE2_GLOBAL_IGNORE:
			c = *in_buffer++;
			++consumed;
			++synth_count;
			synth_buffer[1] = c;
			state = BYTE3_GLOBAL;
			break;

		case BYTE3_GLOBAL:
			c = *in_buffer;
			if (c < 0xF0) {
				if (copy_prefix((char *)prefixEND, out_size)) {
					state = APPEND_CR;
					//end = 1; // We really want to decompile everything
				}
				else {
					done = HP41_TRUE;
				}
			}
			else {
				if (copy_prefix2((char *)prefixLBL, "\"", out_size)) {
					++synth_count;
					synth_buffer[2] = c;
					count = c & 0x0F;
					if (count)
						state = BYTE4_GLOBAL_IGNORE;
					else {
						synth_flag = HP41_TRUE;
						state = CLOSE_QUOTE;
					}
				}
				else {
					done = HP41_TRUE;
				}
			}
			break;

		case BYTE4_GLOBAL_IGNORE:
			c = *in_buffer++;
			++consumed;
			++synth_count;
			synth_buffer[3] = c;
			--count;
			if (count)
				state = BYTE_ALPHA;
			else {
				synth_flag = HP41_TRUE;
				state = CLOSE_QUOTE;
			}
			break;

		case ALPHA_OPEN_QUOTE:
			c = *in_buffer++;
			++consumed;
			*out_buffer++ = '\"';
			++produced;
			++synth_count;
			synth_buffer[1] = c;
			if (c <= 0xF0) {
				synth_flag = HP41_TRUE;
				state = CLOSE_QUOTE;
			}
			else {
				count = c & 0x0F;
				state = BYTE_ALPHA;
			}
			break;

		case TEXT_OPEN_QUOTE:
			if (count)
				j = produced + 5;
			else
				j = produced + 1;
			if (line_numbers) {
				if (line > 999)
					j += 5;
				else
					j += 4;
			}
			if (j > out_size)
				done = HP41_TRUE;
			else {
				if (line_numbers) {
					if (line > 99)
						sprintf((char *)buffer6, "%d ", line);
					else
						sprintf((char *)buffer6, " %02d ", line);
					++line;
					if (line > 1999)
						line = 1;
					j = strlen((char *)buffer6);
					memcpy(out_buffer, buffer6, j);
					out_buffer += j;
					produced += j;
				}

				if (count) {
					c = *in_buffer++;
					++consumed;
					if (c == 0x7F) {
						if (text_append) {
							*out_buffer++ = '\"';
							*out_buffer++ = '|';
							*out_buffer++ = '-';
							++produced;
						}
						else {
							*out_buffer++ = '>';
							*out_buffer++ = '\"';
						}
					}
					else {
						*out_buffer++ = '\"';
						if (is_nodisplay(c)) {
							*out_buffer++ = '\\';
							*out_buffer++ = 'x';
							sprintf((char *)buffer6, "%02X", c);
							memcpy(out_buffer, buffer6, 2);
							out_buffer += 2;
							produced += 3;
							synth_flag = HP41_TRUE;
						}
						else {
							*out_buffer++ = c;
						}
					}
					--count;
					produced += 2;
					++synth_count;
					synth_buffer[1] = c;
				}
				else {
					*out_buffer++ = '\"';
					++produced;
					synth_flag = HP41_TRUE;
				}
				if (count)
					state = BYTE_ALPHA;
				else
					state = CLOSE_QUOTE;
			}
			break;

		case BYTE_ALPHA:
			c = *in_buffer++;
			++consumed;
			if (is_nodisplay(c)) {
				*out_buffer++ = '\\';
				*out_buffer++ = 'x';
				sprintf((char *)buffer6, "%02X", c);
				memcpy(out_buffer, buffer6, 2);
				out_buffer += 2;
				produced += 3;
				synth_flag = HP41_TRUE;
			}
			else {
				*out_buffer++ = c;
			}
			++produced;
			--count;
			synth_buffer[synth_count++] = c;
			if (count == 0)
				state = CLOSE_QUOTE;
			break;

		case CLOSE_QUOTE:
			*out_buffer++ = '\"';
			++produced;
			if (synth_flag)
				state = APPEND_SYNTH;
			else if (room_for_key(synth_buffer, synth_count))
				state = APPEND_KEY;
			else
				state = APPEND_CR;
			break;

		case APPEND_SYNTH:
			j = produced + (synth_count * 3) + 2;
			j += room_for_key(synth_buffer, synth_count);
			if (j > out_size)
				done = HP41_TRUE;
			else {
				/* "text" */
				if (synth_buffer[0] > 0xF0 && synth_count > 1)
					i = 1;
				/* GTO/XEQ/W "alpha" */
				else if (synth_buffer[0] >= 0x1D && synth_buffer[0] <= 0x1F &&
					synth_buffer[1] > 0xF0 && synth_count > 2)
					i = 2;
				/* LBL "alpha" */
				else if (synth_buffer[0] >= 0xC0 && synth_buffer[0] <= 0xCD &&
					synth_buffer[2] > 0xF1 && synth_count > 4)
					i = 4;
				else
					i = 0;

				*out_buffer++ = 0x20;
				*out_buffer++ = 0x20;
				*out_buffer++ = ';';
				produced += 3;
				if (i) {
					j = room_for_key(synth_buffer, synth_count);
					*out_buffer++ = '\"';
					++produced;
					synth_count -= i;
					do {
						c = synth_buffer[i++];
						if (is_nodisplay(c)) {
							*out_buffer++ = '.';
							++produced;
						}
						else {
							*out_buffer++ = c;
							++produced;
						}
					} while (--synth_count);
					*out_buffer++ = '\"';
					++produced;
					if (j) {
						copy_key(out_buffer, synth_buffer[3]);
						out_buffer += j;
						produced += j;
					}
					state = APPEND_CR;
				}
				else {
					do {
						c = synth_buffer[i++];
						sprintf((char *)buffer6, "%02X", c);
						memcpy(out_buffer, buffer6, 2);
						out_buffer += 2;
						produced += 2;
						--synth_count;
						if (synth_count) {
							*out_buffer++ = ',';
							++produced;
						}
					} while (synth_count);
					state = APPEND_CR;
				}
			}
			break;

		case APPEND_KEY:
			i = room_for_key(synth_buffer, synth_count);
			j = produced + i + 3;
			if (j > out_size)
				done = HP41_TRUE;
			else {
				*out_buffer++ = 0x20;
				*out_buffer++ = 0x20;
				*out_buffer++ = ';';
				produced += 3;
				copy_key(out_buffer, synth_buffer[3]);
				out_buffer += i;
				produced += i;
				state = APPEND_CR;
			}
			break;

		case APPEND_CR:
			*out_buffer++ = 0x0D;
			++produced;
			state = APPEND_LF;
			break;

		case APPEND_LF:
			*out_buffer++ = 0x0A;
			++produced;
			if (end)
				done = HP41_TRUE;
			state = BYTE1;
			break;
		}

		/* exhausted buffers? */
		if (produced == out_size ||
			(consumed == *pin_count &&
			state != CLOSE_QUOTE &&
			state != APPEND_SYNTH &&
			state != APPEND_KEY &&
			state != APPEND_CR &&
			state != APPEND_LF &&
			(state != TEXT_OPEN_QUOTE || count)))
			done = HP41_TRUE;

	} while (!done);

	/* out_bufferut pending? */
	*ppending = state == CLOSE_QUOTE ||
		state == APPEND_SYNTH ||
		state == APPEND_KEY ||
		state == APPEND_CR ||
		state == APPEND_LF ||
		(state == TEXT_OPEN_QUOTE && !count);

	*pin_buffer += consumed;
	*pin_count -= consumed;
	*pend = end;
	return(produced);
}

int copy_prefix(char *prefix, int out_size)
{
	int j, n;

	n = strlen(prefix);
	j = produced + n;
	if (line_numbers && !numeric) {
		if (line > 999)
			j += 5;
		else
			j += 4;
	}
	if (j > out_size)
		return 0;

	if (line_numbers && !numeric) {
		if (line > 99)
			sprintf((char *)buffer6, "%d ", line);
		else
			sprintf((char *)buffer6, " %02d ", line);
		++line;
		if (line > 1999)
			line = 1;
		j = strlen((char *)buffer6);
		memcpy(out_buffer, buffer6, j);
		out_buffer += j;
		produced += j;
	}

	memcpy(out_buffer, prefix, n);
	out_buffer += n;
	produced += n;
	++in_buffer;
	++consumed;

	return n;
}

int copy_prefix2(char *prefix1, char *prefix2, int out_size)
{
	int j, n, m;

	n = strlen(prefix1);
	m = strlen(prefix2);
	j = produced + n + m;
	if (line_numbers) {
		if (line > 999)
			j += 5;
		else
			j += 4;
	}
	if (j > out_size)
		return 0;

	if (line_numbers) {
		if (line > 99)
			sprintf((char *)buffer6, "%d ", line);
		else
			sprintf((char *)buffer6, " %02d ", line);
		++line;
		if (line > 1999)
			line = 1;
		j = strlen((char *)buffer6);
		memcpy(out_buffer, buffer6, j);
		out_buffer += j;
		produced += j;
	}

	memcpy(out_buffer, prefix1, n);
	out_buffer += n;
	memcpy(out_buffer, prefix2, m);
	out_buffer += m;
	produced += n + m;
	++in_buffer;
	++consumed;

	return n + m;
}

int copy_postfix(char *postfix, int out_size)
{
	int j, n;

	n = strlen(postfix);
	j = produced + n;
	if (j > out_size)
		return 0;

	memcpy(out_buffer, postfix, n);
	out_buffer += n;
	produced += n;
	++in_buffer;
	++consumed;

	return n;
}

int copy_postfix2(char *postfix1, char *postfix2, int out_size)
{
	int j, n, m;

	n = strlen(postfix1);
	m = strlen(postfix2);
	j = produced + n + m;
	if (j > out_size)
		return 0;

	memcpy(out_buffer, postfix1, n);
	out_buffer += n;
	memcpy(out_buffer, postfix2, m);
	out_buffer += m;
	produced += n + m;
	++in_buffer;
	++consumed;

	return n + m;
}

int is_nodisplay(unsigned char code)
{
	return(code < 0x20 || code >= 0x7F);
}

int room_for_key(unsigned char *buffer, int count)
{
	char row;

	if (buffer[0] >= 0xC0 && buffer[0] <= 0xCD &&
		buffer[2] > 0xF1 && buffer[3] && count > 4) {
		row = buffer[3] & 0x0F;
		return((row > 0 && row < 9) ? strlen((char *)pKey) : strlen((char *)psKey));
	}

	return(0);
}

void copy_key(unsigned char *buffer, unsigned char code)
{
	unsigned char row, col;

	row = code & 0x0F;
	if (row > 0 && row < 9) {
		col = (code >> 4) + 1;
		pKey[6] = row + '0';
		pKey[7] = col + '0';
		memcpy(buffer, pKey, strlen((char *)pKey));
	}
	else {
		code -= 8;
		row = code & 0x0F;
		col = (code >> 4) + 1;
		psKey[7] = row + '0';
		psKey[8] = col + '0';
		memcpy(buffer, psKey, strlen((char *)psKey));
	}
}

int seek_end(unsigned char *buffer, int count)
{
	unsigned char c;
	int end = 0;
	int i = 0;

	while (count && !end) {
		--count;
		c = buffer[i++];
		switch (seek_state) {
		case SEEK_BYTE1:
			if (c >= 0x1D && c <= 0x1F) {
				seek_state = SEEK_BYTE2_ALPHA;
			}
			else if ((c >= 0x90 && c <= 0xBF) ||
				(c >= 0xCE && c <= 0xCF)) {
				seek_state = SEEK_BYTE2_OF_2;
			}
			else if (c >= 0xC0 && c <= 0xCD) {
				seek_state = SEEK_BYTE2_GLOBAL;
			}
			else if (c >= 0xD0 && c <= 0xEF) {
				seek_state = SEEK_BYTE2_OF_3;
			}
			else if (c >= 0xF0 && c <= 0xFF) {
				alpha_count = c & 0x0F;
				if (alpha_count)
					seek_state = SEEK_BYTE_ALPHA;
			}
			break;

		case SEEK_BYTE2_OF_2:
			seek_state = SEEK_BYTE1;
			break;

		case SEEK_BYTE2_OF_3:
			seek_state = SEEK_BYTE3_OF_3;
			break;

		case SEEK_BYTE3_OF_3:
			seek_state = SEEK_BYTE1;
			break;

		case SEEK_BYTE2_GLOBAL:
			seek_state = SEEK_BYTE3_GLOBAL;
			break;

		case SEEK_BYTE3_GLOBAL:
			if (c < 0xF0) {
				end = i;
				seek_state = SEEK_BYTE1;
			}
			else {
				alpha_count = c & 0x0F;
				seek_state = alpha_count ? SEEK_BYTE4_GLOBAL : SEEK_BYTE1;
			}
			break;

		case SEEK_BYTE4_GLOBAL:
			--alpha_count;
			seek_state = alpha_count ? SEEK_BYTE_ALPHA : SEEK_BYTE1;
			break;

		case SEEK_BYTE2_ALPHA:
			if (c <= 0xF0)
				seek_state = SEEK_BYTE1;
			else {
				alpha_count = c & 0x0F;
				seek_state = SEEK_BYTE_ALPHA;
			}
			break;

		case SEEK_BYTE_ALPHA:
			--alpha_count;
			if (alpha_count == 0)
				seek_state = SEEK_BYTE1;
			break;
		}
	}

	return(end);
}
