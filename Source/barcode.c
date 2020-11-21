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
#include "barcode.h"

static char *bc_title = NULL;
static int ps_pages = 1;
static float ps_top = (float)PS_TOP_MARGIN;
static long ps_page_pos[PS_MAX_PAGES];
static char pcl_left_margin[] = "    ";	  /* 4 spaces */
static char pcl_row_margin[] = "      ";  /* 6 spaces */

void barcode_init(void)
{
	bc_title = NULL;
	ps_pages = 1;
	ps_top = (float)PS_TOP_MARGIN;
}

void barcode(char *infile, char *outfile, char *title)
{
	/* use static on large arrays to avoid growing the stack */
	static char clone_dir_path[_MAX_PATH];
	static char clone_file_path[_MAX_PATH];
	char bc_ext[_MAX_EXT];
	int clone_input = HP41_FALSE;
	int i, j, k;
	long blk, reg;
	long prog_pos;
	long inlength;
	long outlength = 0;
	int files;
	FILE *fin = NULL;
	FILE *fout = NULL;
	FIND_FILE rawfile;
	unsigned char *in_buffer, *out_buffer;
	int in_count, out_count = 0, out_size = 0;
	int start_line, end_line;
	int trail_count, lead_count;
	int previous_trail_count = 0;
	int previous_start_line = 0;
	int previous_end_line = 0;
	int checksum;
	int row = 1;
	DECODE_FLAG flag;

	/* init strings */
	bc_title = title;
	get_bc_ext(bc_ext);

	/* get input file(s) */
	if ((files = find_input_files(&rawfile, dirpath, infile, ".raw")) == HP41_OK) {
		goto barcode_exit;
	}

	/* multiple input files? */
	if (files > 1) {
		printf("Error: invalid input path, found[ %d ] files searching[ %s ]\n",
			files, dirpath);
		goto barcode_exit;
	}

	/* get output path */
	if (outfile == NULL || *outfile == '\0') {
		clone_dir_path[0] = '\0';
		clone_input = HP41_TRUE;
	}
	else if (get_file_path(outpath, outfile, bc_ext) != HP41_OK) {
		goto barcode_exit;
	}
	else if (exists_as_directory(outpath) != HP41_ERROR) {
		/* outpath is a directory */
		strcpy(clone_dir_path, outpath);
		terminate_directory(clone_dir_path);
		clone_input = HP41_TRUE;
	}

	/* set input path */
	getfullpath(inpath, dirpath, rawfile.name);

	/* set input length */
	inlength = rawfile.size;

	/* clone output path from input file */
	if (clone_input) {
		file_splitpath(rawfile.name, drive, dir, fname, ext);
		strcpy(clone_file_path, clone_dir_path);
		strcat(clone_file_path, fname);
		strcat(clone_file_path, bc_ext);
		file_fullpath(outpath, clone_file_path);
	}

	/* open input file */
	if ((fin = open_input_ex(inpath, inlength, infile, outpath))) {
		/* open output file */
		if ((fout = open_output(outpath))) {
			/* show banner */
			get_bc_type(ascii);
			printf("Barcode[ %s ]:\n", ascii);
			printf("in[ %s ] => out[ %s ]\n", inpath, outpath);

			if (bc_printer == PRINTER_POSTCRIPT) {
				/* postcript prolog */
				fprintf(fout, "%%!PS-Adobe-2.0\r\n");
				fprintf(fout, "%%%%Creator: \"HP41UC\", HP-41 Barcode Generator by Leo Duran.\r\n");
				fprintf(fout, "%%%%DocumentPaperSizes: letter\r\n");
				fprintf(fout, "%%%%Pages: (atend)\r\n");
				fprintf(fout, "%%%%PageOrder: Ascend\r\n");
				fprintf(fout, "%%%%EndComments\r\n\r\n");

				/* postcript variables */
				fprintf(fout, "%%%%BeginProlog\r\n");
				fprintf(fout, "/ps_punit %.2f def\r\n", PS_PEN_UNIT);
				fprintf(fout, "/ps_height %.2f def\r\n", PS_BAR_HEIGHT);
				fprintf(fout, "/ps_zero_bits %d def\r\n", PS_ZERO_BITS);
				fprintf(fout, "/ps_zero_pen %d def\r\n", PS_ZERO_PEN);
				fprintf(fout, "/ps_one_bits %d def\r\n", PS_ONE_BITS);
				fprintf(fout, "/ps_one_pen %d def\r\n", PS_ONE_PEN);

				/* postcript procedure: ps_putbar */
				fprintf(fout, "\r\n/ps_putbar {\r\n");
				fprintf(fout, "  dup ps_bits add ps_punit mul 2 div ps_center add\r\n");
				fprintf(fout, "  /ps_center exch def\r\n");
				fprintf(fout, "  /ps_bits exch def\r\n");
				fprintf(fout, "  ps_punit mul setlinewidth\r\n");
				fprintf(fout, "  ps_center ps_top moveto\r\n");
				fprintf(fout, "  0 ps_height rlineto stroke\r\n");
				fprintf(fout, "} bind def\r\n");

				/* postcript procedure: ps_putbyte */
				fprintf(fout, "\r\n/ps_putbyte {\r\n");
				fprintf(fout, "  /ps_byte exch def\r\n");
				fprintf(fout, "  /ps_mask 128 def\r\n");
				fprintf(fout, "  8 {\r\n");
				fprintf(fout, "    ps_byte ps_mask and 0 eq\r\n");
				fprintf(fout, "    { ps_zero_pen ps_zero_bits }\r\n");
				fprintf(fout, "    { ps_one_pen ps_one_bits }\r\n");
				fprintf(fout, "    ifelse ps_putbar\r\n");
				fprintf(fout, "    /ps_mask ps_mask 2 idiv def\r\n");
				fprintf(fout, "  } repeat\r\n");
				fprintf(fout, "} bind def\r\n");

				/* postcript procedure: ps_putrow */
				fprintf(fout, "\r\n/ps_putrow {\r\n");
				fprintf(fout, "  /ps_bytes exch def\r\n");
				fprintf(fout, "  /ps_center %.2f def\r\n", PS_CENTER);
				fprintf(fout, "  /ps_bits 0 def\r\n");
				fprintf(fout, "  ps_zero_pen ps_zero_bits ps_putbar\r\n");
				fprintf(fout, "  ps_zero_pen ps_zero_bits ps_putbar\r\n");
				fprintf(fout, "  ps_bytes {\r\n");
				fprintf(fout, "    ps_putbyte\r\n");
				fprintf(fout, "  } repeat\r\n");
				fprintf(fout, "  ps_one_pen ps_one_bits ps_putbar\r\n");
				fprintf(fout, "  ps_zero_pen ps_zero_bits ps_putbar\r\n");
				fprintf(fout, "} bind def\r\n");
				fprintf(fout, "%%%%EndProlog\r\n\r\n");

				/* first page */
				fprintf(fout, "gsave\r\n");
				fflush(fout);
				ps_page_pos[0] = get_filelength(_fileno(fout)) + 2;
				fprintf(fout, "%%%%Page:      \r\n");
			}
			else if (bc_printer == PRINTER_HP) {
				/* normal text pitch at 12cpi, 80cpl */
				fprintf(fout, "\x1B&k0S");

				/* line spacing at 8 lines/inch */
				fprintf(fout, "\x1B&l8D");

				/* horizontal print region at 640 dots( 96 dots/inch ) */
				fprintf(fout, "\x1B*r640S");
			}

			/* "Title" */
			if (bc_printer == PRINTER_POSTCRIPT) {
				if (bc_title && *bc_title != '\0') {
					fprintf(fout, "/Helvetica findfont 14 scalefont setfont\r\n");
					fprintf(fout, "%.2f %.2f moveto (%s) show\r\n",
						PS_LEFT_MARGIN, ps_top, bc_title);
				}

				/* skip a row */
				ps_top -= (float)PS_ROW_HEIGHT;
			}
			else if (bc_printer == PRINTER_HP) {
				/* title in bold */
				if (bc_title && *bc_title != '\0') {
					fprintf(fout, "%s\x1B(s1B%s\x1B(s0B",
						pcl_left_margin, bc_title);
				}
				fprintf(fout, "\r\n\r\n");
			}
			else { /* PRINTER_NONE */
				if (bc_title && *bc_title != '\0') {
					fprintf(fout, "%s\r\n", bc_title);
				}
				fprintf(fout, "\r\n");
			}

			/* "Registers Needed:" */
			if (bc_printer == PRINTER_POSTCRIPT) {
				fprintf(fout, "/Helvetica findfont 12 scalefont setfont\r\n");
				fprintf(fout, "%.2f %.2f moveto (", PS_LEFT_MARGIN, ps_top);

				fflush(fout);
				prog_pos = get_filelength(_fileno(fout));
				fprintf(fout, "Program Registers Needed:     ) show\r\n");

				/* set font for rows */
				fprintf(fout, "/Helvetica findfont 8 scalefont setfont\r\n");

				/* skip a row */
				ps_top -= (float)PS_ROW_HEIGHT;
			}
			else if (bc_printer == PRINTER_HP) {
				fflush(fout);
				prog_pos = get_filelength(_fileno(fout));
				fprintf(fout, "%sProgram Registers Needed:     \r\n\r\n",
					pcl_left_margin);

				/* compressed text pitch at 21.3cpi, 142cpl ( for rows ) */
				fprintf(fout, "\x1B&k2S");
			}
			else { /* PRINTER_NONE */
				fflush(fout);
				prog_pos = get_filelength(_fileno(fout));
				fprintf(fout, "Program Registers Needed:     \r\n\r\n");
			}

			/* program length in blocks */
			blk = inlength / 256;
			if (blk * 256 < inlength)
				++blk;

			/* write program blocks */
			checksum = 0;
			flag = DECODE_FLAG_NONE;
			for (i = 0, j = sizeof(buf1_256); i < blk && j && inlength; ++i) {
				if (inlength < sizeof(buf1_256))
					j = (int)inlength;
				if (fread(buf1_256, 1, j, fin) != (size_t)j) {
					j = 0;
					printf("Error reading from[ %s ]\n", inpath);
				}
				else {
					inlength -= j;
					in_count = j;
					in_buffer = buf1_256;
					do {
						if (flag == DECODE_FLAG_PENDING)
							out_size = 13 - out_count;
						else {
							out_buffer = &buf1_16[3];
							out_size = 13;
							out_count = 0;
						}
						out_count += decode_row(&out_buffer, out_size,
							&in_buffer, &in_count,
							&start_line, &end_line,
							&trail_count, &lead_count,
							&flag);

						if (flag != DECODE_FLAG_PENDING || inlength == 0) {
							/* update checksum */
							for (k = 0; k < out_count; ++k)
								checksum = checksum8(checksum, buf1_16[k + 3]);

							/* init 3-byte header */
							buf1_16[2] = 0;
							buf1_16[1] = ((row - 1) & 0x0F) + 0x10;
							checksum = checksum8(checksum, buf1_16[1]);
							buf1_16[0] = checksum;

							if (lead_count) {
								/* adjust current row */
								checksum = checksum8(checksum, previous_trail_count);
								buf1_16[2] = lead_count << 4;
								checksum = checksum8(checksum, buf1_16[2]);
								buf1_16[0] = checksum;

								/* adjust previous row */
								buf2_16[2] += previous_trail_count;
								buf2_16[0] = checksum8(buf2_16[0], previous_trail_count);

								/* print previous row (adjusted) */
								printbc(row - 1,
									previous_start_line,
									previous_end_line,
									buf2_16, 16, fout);
							}
							else if (previous_trail_count) {
								/* print previous row (non-adjusted) */
								printbc(row - 1,
									previous_start_line,
									previous_end_line,
									buf2_16, 16, fout);
							}

							if (trail_count) {
								/* save current row (wait for next row) */
								memcpy(buf2_16, buf1_16, 16);
								previous_trail_count = trail_count;
								previous_start_line = start_line;
								previous_end_line = end_line;
							}
							else {
								/* print current row */
								previous_trail_count = 0;
								printbc(row, start_line, end_line,
									buf1_16, out_count + 3, fout);
							}

							++row;
							outlength += out_count;
						}

						if (flag == DECODE_FLAG_END)
							inlength = 0;
					} while (j && flag == DECODE_FLAG_NONE);
				}
			}

			if (outlength) {
				/* form-feed on last page */
				if (bc_printer == PRINTER_POSTCRIPT) {
					fprintf(fout, "showpage\r\n");
					fprintf(fout, "%%%%PageTrailer\r\n");
					fprintf(fout, "grestore\r\n");
					fprintf(fout, "%%%%Trailer\r\n");
					fprintf(fout, "%%%%Pages: %d\r\n", ps_pages);
					fprintf(fout, "%%%%EOF\r\n");
				}
				else if (bc_printer == PRINTER_HP) {
					fprintf(fout, "\f");
				}

				/* how many 7-byte registers? */
				reg = outlength / 7;
				if (reg * 7 < outlength) {
					++reg;
				}

				/* go back and edit #registers */
				fseek(fout, prog_pos, SEEK_SET);
				if (bc_printer == PRINTER_POSTCRIPT) {
					fprintf(fout, "Program Registers Needed: %d", reg);
				}
				else if (bc_printer == PRINTER_HP) {
					fprintf(fout, "%sProgram Registers Needed: %d",
						pcl_left_margin, reg);
				}
				else { /* PRINTER_NONE */
					fprintf(fout, "Program Registers Needed: %d", reg);
				}

				/* go back and edit page# */
				if (bc_printer == PRINTER_POSTCRIPT) {
					for (i = 0; i < ps_pages; ++i) {
						fseek(fout, ps_page_pos[i], SEEK_SET);
						fprintf(fout, "Page: %d %d", i + 1, i + 1);
					}
				}

				printf(" Program Size[ %04X ]  ( %d bytes )\n",
					outlength, outlength);
				printf(" Program Registers Needed: %d\n", reg);
			}
		}
	}

barcode_exit:
	if (fout) {
		fflush(fout);
		if (outlength)
			outlength = get_filelength(_fileno(fout));
	}

	findfile_close(&rawfile);
	closefiles(fin, fout, outlength, 0);
}

void printbc(int row, int start_line, int end_line,
	unsigned char *buffer, int count,
	FILE *fout)
{
	int i, j;
	int bit, bytes;
	unsigned char c, *pc;
	RASTER8 one, zero, blank;
	static int rows_in_page = 1;

	if (bc_printer == PRINTER_POSTCRIPT) {
		/* need form-feed? */
		if (ps_top < PS_BOTTOM_MARGIN) {
			/* next page */
			fprintf(fout, "showpage\r\n");
			fprintf(fout, "%%%%PageTrailer\r\n\r\n");
			fflush(fout);
			ps_page_pos[ps_pages++] = get_filelength(_fileno(fout)) + 2;
			fprintf(fout, "%%%%Page:        \r\n");

			/* reset TOP margin */
			ps_top = (float)PS_TOP_MARGIN;

			/* "Title" */
			if (bc_title && *bc_title != '\0') {
				fprintf(fout, "/Helvetica findfont 14 scalefont setfont\r\n");
				fprintf(fout, "%.2f %.2f moveto (%s) show\r\n",
					PS_LEFT_MARGIN, ps_top, bc_title);
			}

			/* skip a row */
			ps_top -= (float)PS_ROW_HEIGHT;

			/* reset font for rows */
			fprintf(fout, "/Helvetica findfont 8 scalefont setfont\r\n");
		}

		/* print row number */
		if (end_line > start_line) {
			fprintf(fout, "%.2f %.2f moveto (Row %d (%d - %d)) show\r\n",
				PS_LEFT_MARGIN, ps_top,
				row, start_line, end_line);
		}
		else {
			fprintf(fout, "%.2f %.2f moveto (Row %d ( %d )) show\r\n",
				PS_LEFT_MARGIN, ps_top,
				row, start_line);
		}

		/* print row bars */
		ps_top -= (float)PS_START_ROW;
		fprintf(fout, "/ps_top %.2f def\r\n", ps_top);

		strcpy(ascii, " ");
		for (i = count - 1; i >= 0; --i) {
			c = buffer[i];
			sprintf(&ascii[strlen(ascii)], " 16#%02X", c);
		}
		sprintf(&ascii[strlen(ascii)], " %d ps_putrow\r\n", count);
		fwrite(ascii, 1, strlen(ascii), fout);

		/* update TOP margin */
		ps_top -= (float)PS_NEXT_ROW;
	}
	else if (bc_printer == PRINTER_HP) {
		/* new page? */
		if (rows_in_page == 20) {
			/* next page */
			rows_in_page = 0;
			fprintf(fout, "\f");

			/* "Title" */
			if (bc_title && *bc_title != '\0') {
				/* normal text pitch at 12cpi, 80cpl */
				fprintf(fout, "\x1B&k0S");

				/* title in bold */
				fprintf(fout, "%s\x1B(s1B%s\x1B(s0B",
					pcl_left_margin, bc_title);

				/* compressed text pitch at 21.3cpi, 142cpl ( for rows ) */
				fprintf(fout, "\x1B&k2S");
			}
			fprintf(fout, "\r\n\r\n");
		}

		if (end_line > start_line) {
			fprintf(fout, "%sROW %d (%d - %d)",
				pcl_row_margin, row, start_line, end_line);
		}
		else {
			fprintf(fout, "%sROW %d ( %d )",
				pcl_row_margin, row, start_line);
		}

		/* init loop variables */
		pc = buf2_256;
		bytes = bit = 0;

		/* init rater fonts */
		one.bitmap = 0x07;
		one.bits = 5;
		zero.bitmap = 0x01;
		zero.bits = 3;
		blank.bitmap = 0x00;
		blank.bits = 8;

		/* left margin */
		for (i = 1; (size_t)i < strlen(pcl_left_margin); ++i)
			render(&pc, blank, &bytes, &bit);

		/* start bars */
		render(&pc, zero, &bytes, &bit);
		render(&pc, zero, &bytes, &bit);

		/* row bytes */
		for (i = 0; i < count; ++i) {
			c = buffer[i];
			for (j = 0; j < 8; ++j) {
				render(&pc, c & 0x80 ? one : zero, &bytes, &bit);
				c <<= 1;
			}
		}

		/* end bars */
		render(&pc, one, &bytes, &bit);
		render(&pc, zero, &bytes, &bit);

		/* start raster graphics */
		fprintf(fout, "\x1B*r0A");

		/* move down 2 scan-lines to start raster */
		fprintf(fout, "\x1B*b0W\x1B*b0W");

		/* repeat raster to set bar height ( close to 0.5in ) */
		for (i = 0; i < 24; ++i) {
			/*printer raster line */
			fprintf(fout, "\x1B*b%dW", bytes);
			fwrite(buf2_256, 1, bytes, fout);
		}

		/* end raster graphics */
		fprintf(fout, "\x1B*rB\r\n");

		/* update ROW count */
		++rows_in_page;
	}
	else { /* PRINTER_NONE */
		if (end_line > start_line) {
			fprintf(fout, "Row %d (%d - %d)\r\n", row, start_line, end_line);
		}
		else {
			fprintf(fout, "Row %d ( %d )\r\n", row, start_line);
		}

		i = hextoascii((unsigned char *)ascii, buffer, count);
		ascii[i++] = 0x0D;
		ascii[i++] = 0x0A;
		fwrite(ascii, 1, i, fout);
	}
}

int decode_row(unsigned char **pout_buffer, int out_size,
	unsigned char **pin_buffer, int *pin_count,
	int *pstart_line, int *pend_line,
	int *ptrail_count, int *plead_count,
	DECODE_FLAG *pflag)
{
	DECODE_FLAG flag;
	int consumed = 0;
	int produced = 0;
	unsigned char c, *inp, *outp;
	static int line = 0;
	static int numeric = HP41_FALSE;
	static int multi_byte = 0;
	static int trail_count = 0;
	static int lead_count;
	static int alpha_count;
	static DECODE_STATE state = DECODE_BYTE1;

	inp = *pin_buffer;
	outp = *pout_buffer;
	flag = *pflag;

	/* update start line */
	if (flag != DECODE_FLAG_PENDING) {
		lead_count = 0;
		if (line) {
			c = *inp;
			*pstart_line = line;
			if (state == DECODE_BYTE1 && (!numeric || c < 0x10 || c > 0x1C))
				*pstart_line += 1;
		}
		else {
			*pstart_line = 1;
		}
	}

	do {
		/* copy byte */
		c = *inp++;
		++consumed;
		*outp++ = c;
		++produced;

		/* update byte count */
		if (state != DECODE_BYTE1)
			++multi_byte;

		switch (state) {
		case DECODE_BYTE1:
			if (c >= 0x10 && c <= 0x1C) {
				if (numeric)
					++multi_byte;
				else {
					++line;
					numeric = HP41_TRUE;
					trail_count = 0;
					multi_byte = 1;
				}
			}
			else {
				numeric = HP41_FALSE;
				if (c != '\0')
					++line;

				if (c >= 0x1D && c <= 0x1F) {
					state = DECODE_BYTE2_ALPHA;
				}
				else if ((c >= 0x90 && c <= 0xBF) ||
					(c >= 0xCE && c <= 0xCF)) {
					state = DECODE_BYTE2_OF_2;
				}
				else if (c >= 0xC0 && c <= 0xCD) {
					state = DECODE_BYTE2_GLOBAL;
				}
				else if (c >= 0xD0 && c <= 0xEF) {
					state = DECODE_BYTE2_OF_3;
				}
				else if (c >= 0xF0 && c <= 0xFF) {
					alpha_count = c & 0x0F;
					if (alpha_count)
						state = DECODE_BYTE_ALPHA;
				}

				/* update byte count */
				if (trail_count) {
					lead_count = multi_byte - trail_count;
					trail_count = 0;
				}
				multi_byte = state == DECODE_BYTE1 ? 0 : 1;
			}
			break;

		case DECODE_BYTE2_OF_2:
			state = DECODE_BYTE1;
			break;

		case DECODE_BYTE2_OF_3:
			state = DECODE_BYTE3_OF_3;
			break;

		case DECODE_BYTE3_OF_3:
			state = DECODE_BYTE1;
			break;

		case DECODE_BYTE2_GLOBAL:
			state = DECODE_BYTE3_GLOBAL;
			break;

		case DECODE_BYTE3_GLOBAL:
			if (c < 0xF0) {
				flag = DECODE_FLAG_END;
				state = DECODE_BYTE1;
			}
			else {
				alpha_count = c & 0x0F;
				state = alpha_count ? DECODE_BYTE4_GLOBAL : DECODE_BYTE1;
			}
			break;

		case DECODE_BYTE4_GLOBAL:
			--alpha_count;
			state = alpha_count ? DECODE_BYTE_ALPHA : DECODE_BYTE1;
			break;

		case DECODE_BYTE2_ALPHA:
			if (c <= 0xF0)
				state = DECODE_BYTE1;
			else {
				alpha_count = c & 0x0F;
				state = DECODE_BYTE_ALPHA;
			}
			break;

		case DECODE_BYTE_ALPHA:
			--alpha_count;
			if (alpha_count == 0)
				state = DECODE_BYTE1;
			break;
		}

		/* init byte count */
		if (state == DECODE_BYTE1 && !numeric && multi_byte) {
			if (trail_count) {
				lead_count = multi_byte - trail_count;
				trail_count = 0;
			}
			multi_byte = 0;
		}

	} while (produced < out_size && consumed < *pin_count && flag != DECODE_FLAG_END);

	/* update flag */
	if (flag != DECODE_FLAG_END) {
		if (produced < out_size)
			flag = DECODE_FLAG_PENDING;
		else {
			trail_count = multi_byte;
			flag = DECODE_FLAG_NONE;
		}
	}
	*pflag = flag;

	/* update END line */
	*pend_line = line;
	*ptrail_count = trail_count;
	*plead_count = lead_count;

	*pout_buffer += produced;
	*pin_buffer += consumed;
	*pin_count -= consumed;

	return(produced);
}

void render(unsigned char **pbuffer, RASTER8 font,
	int *pbytes, int *pbit)
{
	int i, bit, bytes;
	unsigned char dst_mask;
	unsigned char *dst_bitmap;
	unsigned char src_mask;
	unsigned char src_bitmap;

	bit = *pbit;
	bytes = *pbytes;

	dst_mask = 0x80;
	for (i = 0; i < bit; ++i)
		dst_mask >>= 1;
	dst_bitmap = *pbuffer;

	src_mask = 0x01;
	for (i = 1; i < font.bits; ++i)
		src_mask <<= 1;
	src_bitmap = font.bitmap;

	/* walk thru source bits */
	for (i = 0; i < font.bits; ++i) {
		if (bit == 0) {
			++bytes;
			*dst_bitmap = 0x00;
		}

		if (src_bitmap & src_mask)
			*dst_bitmap |= dst_mask;

		src_mask >>= 1;
		dst_mask >>= 1;
		if (++bit == 8) {
			bit = 0;
			++dst_bitmap;
			dst_mask = 0x80;
		}
	}

	*pbit = bit;
	*pbytes = bytes;
	*pbuffer = dst_bitmap;
}

int checksum8(int old_sum, int update)
{
	int new_sum;

	new_sum = old_sum + update;
	if (new_sum > 0xFF)
		new_sum -= 0xFF;

	return(new_sum);
}


void get_bc_ext(char *ext)
{
	if (bc_printer == PRINTER_HP)
		strcpy(ext, ".pcl");
	else if (bc_printer == PRINTER_POSTCRIPT)
		strcpy(ext, ".ps");
	else
		strcpy(ext, ".wnd");
}

void get_bc_type(char *type)
{
	if (bc_printer == PRINTER_HP)
		strcpy(type, "HP PCL");
	else if (bc_printer == PRINTER_POSTCRIPT)
		strcpy(type, "Postcript");
	else
		strcpy(type, "Hex Dump");
}
