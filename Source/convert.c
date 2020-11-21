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
aLong with HP41UC.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hp41uc.h"
#define DAT_ASCII_HEADER_SIZE   4


void convert(char *infile, FILE_DESC *pin, char *outfile, FILE_DESC *pout, char *name)
{
	/* use static on large arrays to avoid growing the stack */
	static char clone_dir_path[_MAX_PATH];
	static char clone_file_path[_MAX_PATH];
	int clone_input = HP41_FALSE;
	char pname[11];
	char p41name[11];
	Long inlength;
	Long outlength = 0;
	int files;
	Long dirblks, startblk;
	FILE *fin, *fout = NULL;
	FIND_FILE findfile;

	/* get input file(s) */
	if ((files = find_input_files(&findfile, dirpath, infile, pin->ext)) == HP41_OK) {
		goto convert_exit;
	}
	else if (files > 1 && pin->file_type == FILE_LIF) {
		printf("Error: found[ %d ] files searching[ %s ], but input type is LIF\n",
			files, dirpath);
		goto convert_exit;
	}

	/* get output path */
	if (outfile == NULL || *outfile == '\0') {
		clone_dir_path[0] = '\0';
		clone_input = HP41_TRUE;
	}
	else if (get_file_path(outpath, outfile, pout->ext) != HP41_OK) {
		goto convert_exit;
	}
	/* outpath is a directory? */
	else if (exists_as_directory(outpath) != HP41_ERROR) {
		strcpy(clone_dir_path, outpath);
		terminate_directory(clone_dir_path);
		clone_input = HP41_TRUE;
	}
	/* outpath is a file */
	else if (files > 1 && pout->file_type != FILE_LIF) {
		printf("Error: found[ %d ] files searching[ %s ], but output[ %s ] is not a LIF file\n",
			files, dirpath, outpath);
		goto convert_exit;
	}

	if (files > 1) {
		/* ignore NAME parameter, use input filename instead */
		name = NULL;

		/* cap input files to MAX_LIF_FILES */
		if (pout->file_type == FILE_LIF && files > MAX_LIF_FILES) {
			printf("Warning: found[ %d ] input files, but using only[ %d ]\n", files, MAX_LIF_FILES);
			files = MAX_LIF_FILES;
		}
	}

	do {
		/* set input path */
		getfullpath(inpath, dirpath, findfile.name);

		/* set input length */
		inlength = findfile.size;
		if (pin->file_type != FILE_LIF)
			inlength -= pin->header;

		/* clone output path from input file */
		if (clone_input) {
			file_splitpath(findfile.name, drive, dir, fname, ext);
			strcpy(clone_file_path, clone_dir_path);
			strcat(clone_file_path, fname);
			strcat(clone_file_path, pout->ext);
			file_fullpath(outpath, clone_file_path);
			outlength = 0;
			fout = NULL;
		}

		/* open input file */
		if ((fin = open_input_ex(inpath, inlength, infile, outpath))) {
			if (pin->file_type == FILE_LIF) {
				/* read LIF header */
				if ((dirblks = read_lif_hdr(fin, inpath, inlength))) {
					/* get program name */
					get_lif_name(pname, name, findfile.name);

					/* read LIF directory */
					if ((startblk = read_lif_dir(fin, inpath, &inlength, pname,
						dirblks))) {

						/* open output file */
						if ((fout = open_output(outpath))) {
							/* show banner */
							printf("%s[ %s ] => %s[ %s ]\n",
								pin->banner, inpath, pout->banner, outpath);

							/* write output file */
							fseek(fin, startblk * 256, SEEK_SET);
							copy_file(fout, outpath, pout->file_type, &outlength,
								pname, files, inlength,
								fin, inpath, pin->data_type);
						}
					}
				}
			}
			else {
				/* read program size */
				if (pin->file_type == FILE_BIN)
					inlength = read_bin_size(fin, inpath, inlength);
				else if (pin->file_type == FILE_DAT)
					inlength = read_dat_size(fin, inpath, inlength);
				else if (pin->file_type == FILE_P41) {
					inlength = read_p41_dir(fin, inpath, inlength, p41name);
					name = p41name;
				}

				if (inlength) {
					/* open output file */
					if (fout || (fout = open_output(outpath))) {
						/* show banner */
						printf("%s[ %s ] => %s[ %s ]\n",
							pin->banner, inpath, pout->banner, outpath);

						/* get program name */
						get_lif_name(pname, name, findfile.name);

						/* write output file */
						copy_file(fout, outpath, pout->file_type, &outlength,
							pout->file_type == FILE_RAW ? NULL : pname,
							files, inlength,
							fin, inpath, pin->data_type);
					}
				}
			}

			fclose(fin);
		}
		--files;
	} while (fout && files && findfile_next(&findfile) == HP41_OK);

convert_exit:
	findfile_close(&findfile);
	closefiles(NULL, fout, outlength, pout->header);
}

void p41dump(char *infile)
{
	int files;
	FILE *fin;
	FIND_FILE p41file;

	/* get input file(s) */
	if ((files = find_input_files(&p41file, dirpath, infile, p41.ext)) == HP41_OK) {
		goto p41dump_exit;
	}

	do {
		/* set input path */
		getfullpath(inpath, dirpath, p41file.name);

		/* open input file */
		if ((fin = open_input_ex(inpath, p41file.size, infile, NULL))) {
			/* dump LIF directry */
			printf("LIF directory listing[ %s ]\n", inpath);
			dump_lif_dir(fin, inpath, 1, NULL);

			fclose(fin);
		}

		--files;
	} while (files && findfile_next(&p41file) == HP41_OK);

p41dump_exit:
	findfile_close(&p41file);
}

void lifdump(char *infile, char *name)
{
	Long dirblks;
	Long inlength;
	int files;
	FILE *fin;
	FIND_FILE liffile;

	/* get input file(s) */
	if ((files = find_input_files(&liffile, dirpath, infile, lif.ext)) == HP41_OK) {
		goto lifdump_exit;
	}

	do {
		/* set input path */
		getfullpath(inpath, dirpath, liffile.name);

		/* set input length */
		inlength = liffile.size;

		/* open input file */
		if ((fin = open_input_ex(inpath, inlength, infile, outpath))) {
			/* read LIF header */
			printf("LIF directory listing[ %s ]\n", inpath);
			if ((dirblks = read_lif_hdr(fin, inpath, inlength))) {
				/* dump LIF directry */
				dump_lif_dir(fin, inpath, dirblks * 8, name);
			}

			fclose(fin);
		}

		--files;
	} while (files && findfile_next(&liffile) == HP41_OK);

lifdump_exit:
	findfile_close(&liffile);
}

void copy_file(FILE *fout, char *outpath,
	FILE_TYPE outftype, Long *poutlength,
	char *pname, int files, Long inlength,
	FILE *fin, char *inpath, DATA_TYPE indtype)
{
	unsigned char *puc;
	unsigned char chksum;
	Long lsize, dirblk;
	/* copy_file() may be called multiple times for the same LIF output file */
	static Long startblk;
	static Long dirpos;

	/* write program blocks */
	if (outftype == FILE_RAW) {
		if (copy_blocks(fout, outpath, DATA_RAW, poutlength,
			&lsize, &chksum, NULL,
			fin, inpath, indtype, inlength)) {

			/* write cheksum block */
			if (!raw_checksum ||
				write_raw_checksum(fout, outpath, poutlength,
				buf1_256, 256, chksum)) {
				printf(" size[ %04X ]  ( %d bytes )\n",
					(unsigned short)lsize, lsize);
			}
		}
	}
	else if (outftype == FILE_BIN) {
		*poutlength += 2;
		fseek(fout, 2L, SEEK_SET);
		if (copy_blocks(fout, outpath, DATA_RAW, poutlength,
			&lsize, &chksum, NULL,
			fin, inpath, indtype, inlength)) {

			/* write cheksum block */
			puc = (unsigned char *)&lsize;
			chksum += puc[0] + puc[1];
			if (write_raw_checksum(fout, outpath, poutlength,
				buf1_256, 128, chksum)) {
				printf(" size[ %04X ]  ( %d bytes )\n",
					(unsigned short)lsize, lsize);
			}
		}

		/* over-write program size */
		fseek(fout, 0L, SEEK_SET);
		write_raw_size(fout, lsize);
	}
	else if (outftype == FILE_P41) {
		fseek(fout, 32L, SEEK_SET);
		if (copy_blocks(fout, outpath, DATA_RAW, poutlength,
			&lsize, &chksum, NULL,
			fin, inpath, indtype, inlength)) {

			/* write cheksum block */
			if (write_raw_checksum(fout, outpath, poutlength,
				buf1_256, 256, chksum)) {
				show_lif_info(pname, lsize);
			}
		}

		/* over-write directory */
		fseek(fout, 0L, SEEK_SET);
		*poutlength += write_lif_dir(fout, pname, 0, lsize);
	}
	else if (outftype == FILE_LIF) {
		/* write LIF header */
		if (*poutlength == 0) {
			*poutlength = write_lif_hdr(fout, &startblk, &dirpos, files);
		}

		/* write program blocks */
		dirblk = startblk;
		fseek(fout, dirblk * 256, SEEK_SET);
		if (copy_blocks(fout, outpath, DATA_RAW, poutlength,
			&lsize, &chksum, &startblk,
			fin, inpath, indtype, inlength)) {

			/* write cheksum block */
			++startblk;
			if (write_raw_checksum(fout, outpath, poutlength,
				buf1_256, 256, chksum)) {
				show_lif_info(pname, lsize);
			}
		}

		/* over-write empty directory */
		fseek(fout, dirpos, SEEK_SET);
		dirpos += write_lif_dir(fout, pname, dirblk, lsize);
	}
	else if (outftype == FILE_DAT) {
		fseek(fout, 4L, SEEK_SET);
		if (copy_blocks(fout, outpath, DATA_DAT, poutlength,
			&lsize, &chksum, NULL,
			fin, inpath, indtype, inlength)) {

			/* write cheksum */
			puc = (unsigned char *)&lsize;
			chksum += puc[0] + puc[1];
			if (write_dat_checksum(fout, outpath,
				poutlength, chksum)) {
				printf(" size[ %04X ]  ( %d bytes )\n",
					(unsigned short)lsize, lsize);
			}
		}

		/* over-write program size */
		fseek(fout, 0L, SEEK_SET);
		*poutlength += write_dat_size(fout, lsize);
	}
	else if (outftype == FILE_TXT) {
		copy_blocks(fout, outpath, DATA_TXT, poutlength,
			NULL, NULL, NULL,
			fin, inpath, indtype, inlength);
	}
}

int copy_blocks(FILE *fout, char *outpath,
	DATA_TYPE outdtype, Long *poutlength,
	Long *psize, unsigned char *pchksum, Long *pstartblk,
	FILE *fin, char *inpath,
	DATA_TYPE indtype, Long inlength)
{
	if (indtype == DATA_RAW) {
		return(copy_raw_blocks(fout, outpath, outdtype, poutlength,
			psize, pchksum, pstartblk,
			fin, inpath, inlength));
	}
	else if (indtype == DATA_DAT) {
		return(copy_dat_blocks(fout, outpath, outdtype, poutlength,
			psize, pchksum, pstartblk,
			fin, inpath, inlength));
	}
	else { /* DATA_TXT*/
		return(copy_txt_blocks(fout, outpath, outdtype, poutlength,
			psize, pchksum, pstartblk,
			fin, inpath, inlength));
	}
}

int copy_raw_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	Long *poutlength, Long *psize,
	unsigned char *pchksum, Long *pstartblk,
	FILE *fin, char *inpath, Long inlength)
{
	size_t blkcnt, incnt;
	size_t inblk, outblk;
	size_t  i, j, k;
	Long size;
	int pending, end;
	unsigned char *inbuf;

	/* program length in blocks */
	blkcnt = (size_t)inlength / 256;
	if (blkcnt * 256 < (size_t)inlength)
		++blkcnt;

	/* init checksum */
	if (pchksum) {
		*pchksum = 0;
	}

	/* init progranm size */
	size = inlength;

	/* copy blocks */
	inblk = sizeof(buf1_256);
	memset(buf1_256, 0, sizeof(buf1_256));
	if (outdtype == DATA_TXT)
		decompile_init();
	for (i = 0; i < blkcnt && inblk && inlength; ++i) {
		/* less than a full block? */
		if (inlength < sizeof(buf1_256))
			inblk = (size_t)inlength;

		/* read block */
		if (fread(buf1_256, 1, inblk, fin) != inblk) {
			inblk = 0;
			printf("Error reading from[ %s ]\n", inpath);
		}
		else {
			// We really want whole program, not just part to the first end /* check for END in block */
			// outblk = seek_end(buf1_256, inblk);
			outblk = inblk;
			if (outblk) {
				/* adjust program size */
				size += outblk - inlength;

				/* last input block */
				inlength = 0;
			}
			else {
				inlength -= inblk;
				outblk = inblk;
			}

			/* update start block */
			if (pstartblk) {
				if (outblk == sizeof(buf1_256))
					*pstartblk += 1;
			}

			/* update checksum */
			if (pchksum) {
				for (j = 0; j < outblk; ++j)
					*pchksum += buf1_256[j];
			}

			/* write block */
			if (outdtype == DATA_TXT) {
				/* decompile block */
				inbuf = buf1_256;
				incnt = outblk;
				do {
					j = decompile(buf_1024, sizeof(buf_1024),
						&inbuf, (int *)&incnt, &pending, &end);
					if (j) {
						k = fwrite(buf_1024, 1, j, fout);
						*poutlength += k;
						if (k != j) {
							inblk = 0;
							printf("Error writing to[ %s ]\n", outpath);
						}
					}
					if (end) {
						incnt = 0;
						inlength = 0;
					}
				} while (inblk && (incnt || (pending && !inlength)));
			}
			else if (outdtype == DATA_DAT) {
				j = hextoascii(buf_512, buf1_256, outblk);
				k = fwrite(buf_512, 1, j, fout);
				*poutlength += k;
				if (k != j) {
					inblk = 0;
					printf("Error writing to[ %s ]\n", outpath);
				}
			}
			else {  /* DATA_RAW */
				j = fwrite(buf1_256, 1, outblk, fout);
				*poutlength += j;
				if (j != outblk) {
					inblk = 0;
					printf("Error writing to[ %s ]\n", outpath);
				}
			}
		}
	}

	/* adjusted program size */
	if (psize) {
		*psize = size;
	}

	return(inblk && size);
}

int copy_dat_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	Long *poutlength, Long *psize,
	unsigned char *pchksum, Long *pstartblk,
	FILE *fin, char *inpath, Long inlength)
{
	size_t blkcnt, incnt;
	size_t inblk, outblk;
	size_t remain;
	size_t i, j, k;
	Long size;
	Long seekpos;
	int pending, end;
	unsigned char *inbuf;
	unsigned char *datbuf;

	/* program length in blocks */
	blkcnt = (size_t)inlength / 512;
	if (blkcnt * 512 < (size_t)inlength)
		++blkcnt;

	/* init checksum */
	if (pchksum) {
		*pchksum = 0;
	}

	/* init program size */
	size = 0;
	seekpos = DAT_ASCII_HEADER_SIZE;

	/* copy blocks */
	inblk = sizeof(buf_512);
	memset(buf_512, 0, sizeof(buf_512));
	if (outdtype == DATA_TXT)
		decompile_init();
	for (i = 0; i < blkcnt && inblk && inlength; ++i) {
		/* less than a full block? */
		if (inlength < sizeof(buf_512))
			inblk = (size_t)inlength;

		remain = inblk;
		datbuf = buf_512;
		while (inblk && remain) {
			/* read block */
			if (fread(datbuf, 1, remain, fin) != remain) {
				inblk = 0;
				printf("Error reading from[ %s ]\n", inpath);
			}
			else {
				/* validate hex digits */
				if ((j = nonxdigit_buffer(datbuf, remain))) {
					seekpos += j;
					remain -= (j - 1);
					datbuf = &buf_512[inblk - remain];

					/* point past LF in the input file */
					fseek(fin, seekpos, SEEK_SET);
					printf(" Warning: Skipped Line-Feed (0x0A) character at offset[ %d ]\n", seekpos - 1);
				}
				else {
					seekpos += remain;
					remain = 0;
				}
			}
		}

		if (inblk) {
			/* convert ASCII to hex */
			j = asciitohex(buf1_256, buf_512, inblk);

			/* check for END in block */
			outblk = seek_end(buf1_256, j);
			if (outblk) {
				/* adjust program size */
				size += outblk - inblk / 2;

				/* last input block */
				inlength = 0;
			}
			else {
				inlength -= inblk;
				outblk = inblk / 2;
				size += inblk / 2;
			}

			/* update start block */
			if (pstartblk) {
				if (outblk == sizeof(buf1_256))
					*pstartblk += 1;
			}

			/* update checksum */
			if (pchksum) {
				for (j = 0; j < outblk; ++j)
					*pchksum += buf1_256[j];
			}

			/* write block */
			if (outdtype == DATA_TXT) {
				/* decompile block */
				inbuf = buf1_256;
				incnt = outblk;
				do {
					j = decompile(buf_1024, sizeof(buf_1024),
						&inbuf, (int *)&incnt, &pending, &end);
					if (j) {
						k = fwrite(buf_1024, 1, j, fout);
						*poutlength += k;
						if (k != j) {
							inblk = 0;
							printf("Error writing to[ %s ]\n", outpath);
						}
					}
					if (end) {
						incnt = 0;
						inlength = 0;
					}
				} while (inblk && (incnt || (pending && !inlength)));
			}
			else {  /* DATA_RAW */
				j = fwrite(buf1_256, 1, outblk, fout);
				*poutlength += j;
				if (j != outblk) {
					inblk = 0;
					printf("Error writing to[ %s ]\n", outpath);
				}
			}
		}
	}

	/* adjusted program size */
	if (psize) {
		*psize = size;
	}

	return(inblk && size);
}

int copy_txt_blocks(FILE *fout, char *outpath, DATA_TYPE outdtype,
	Long *poutlength, Long *psize,
	unsigned char *pchksum, Long *pstartblk,
	FILE *fin, char *inpath, Long inlength)
{
	Long blkcnt;
	size_t incnt;
	size_t inblk, outblk;
	size_t i, j, k;
	Long size;
	int pending;
	COMPILE_FLAG flag;
	int dist, max;
	unsigned char *inbuf;
	unsigned char *outbuf;

	/* program length in blocks */
	blkcnt = inlength / 1024;
	if (blkcnt * 1024 < inlength)
		++blkcnt;

	/* init checksum */
	if (pchksum)
		*pchksum = 0;

	/* init program size */
	size = 0;

	/* copy blocks */
	inblk = sizeof(buf_1024);
	memset(buf_1024, 0, sizeof(buf_1024));
	pending = dist = 0;
	flag = COMPILE_FLAG_RESTART;
	compile_init();
	for (i = 0, max = 0; i < (size_t)blkcnt && !max && inblk && inlength; ++i) {
		/* less than a full block? */
		if (inlength < sizeof(buf_1024)) {
			inblk = (size_t)inlength;
		} else {
			fprintf(stderr, "Code doesn't fit in buffer of %i bytes. Recompile code with bigger buffer.\n",(int)sizeof(buf_1024));
			exit(1);
		}

		/* read block */
		if (fread(buf_1024, 1, inblk, fin) != inblk) {
			inblk = 0;
			printf("Error reading from[ %s ]\n", inpath);
		}
		else {
			inlength -= inblk;
			inbuf = buf_1024;
			incnt = inblk;
			inbuf[inblk] = 0; // Terminating char
			do {
				/* compile block */
				outblk = compile(buf1_256, sizeof(buf1_256),
					&inbuf, (int *)&incnt, &flag, &pending, &dist);

				/* compiler error? */
				if (flag == COMPILE_FLAG_ERROR) {
					inblk = 0;
					size = 0;
				}
				else if (outblk) {
					/* update program size */
					size += outblk;

					/* limit output @ 64K */
					if (size > 65535 ||
						(size > 65532 && flag != COMPILE_FLAG_END)) {
						size -= outblk;
						max = 1;
					}

					if (!max) {
						/* update checksum */
						if (pchksum) {
							for (j = 0; j < outblk; ++j)
								*pchksum += buf1_256[j];
						}

						/* write block */
						if (outdtype == DATA_DAT) {
							j = hextoascii(buf_512, buf1_256, outblk);
							outbuf = buf_512;
						}
						else {  /* DATA_RAW */
							j = outblk;
							outbuf = buf1_256;
						}
						k = fwrite(outbuf, 1, j, fout);
						*poutlength += k;
						if (k != j) {
							inblk = 0;
							printf("Error writing to[ %s ]\n", outpath);
						}
					}
				}

				/* the .END.? */
				if (flag == COMPILE_FLAG_END || flag == COMPILE_FLAG_EOF) {
					incnt = 0;
					inlength = 0;
				}
				else if (outblk == 0 && inlength == 0) {
					incnt = 0;
				}
				/* force last flush with EOF character */
				else if (incnt == 0 && inlength == 0 && !pending) {
					incnt = 1;
					inbuf = buf_1024;
					*inbuf = 0x1A;
				}
			} while (inblk && !max && (incnt || (pending && !inlength)));
		}
	}

	/* need to append .END.? */
	if (size && inblk && flag != COMPILE_FLAG_END) {
		/* compile .END. */
		compile_end(buf1_256, dist);

		/* update checksum */
		if (pchksum) {
			for (i = 0; i < 3; ++i)
				*pchksum += buf1_256[i];
		}

		/* append .END. */
		if (outdtype == DATA_DAT) {
			j = hextoascii(buf_512, buf1_256, 3);
			outbuf = buf_512;
		}
		else {  /* DATA_RAW */
			j = 3;
			outbuf = buf1_256;
		}
		k = fwrite(outbuf, 1, j, fout);
		*poutlength += k;
		size += k;
		if (k != j) {
			inblk = 0;
			printf("Error writing to[ %s ]\n", outpath);
		}
	}

	/* update start block */
	if (pstartblk) {
		*pstartblk += size / 256;
	}

	/* program size */
	if (psize) {
		*psize = size;
	}

	return(inblk && size);
}

Long write_raw_checksum(FILE *fout, char *outpath, Long *poutlength,
	unsigned char *buffer, Long bufsize,
	unsigned char chksum)
{
	Long n, r;

	/* get remaining bytes in block */
	n = *poutlength;
	r = bufsize - (n - (n / bufsize * bufsize));

	/* copy checksum ( padded with NULL ) */
	memset(buffer, 0, (size_t)r);
	buffer[0] = chksum;

	/* write checksum block */
	n = fwrite(buffer, 1, (size_t)r, fout);
	*poutlength += n;
	if (n != r) {
		printf("Error writing to[ %s ]\n", outpath);
		return(0);
	}

	return(n);
}

Long write_dat_checksum(FILE *fout, char *outpath,
	Long *poutlength,
	unsigned char chksum)
{
	unsigned char buffer2[2];
	Long n;

	/* copy checksum */
	hextoascii(buffer2, &chksum, 1);

	/* write checksum bytes */
	n = fwrite(buffer2, 1, 2, fout);
	*poutlength += n;
	if (n != 2) {
		printf("Error writing to[ %s ]\n", outpath);
		return(0);
	}

	return(n);
}

Long write_raw_size(FILE *fout, Long size)
{
	unsigned char buffer2[2];
	unsigned char *puc;

	/* motorola format */
	puc = (unsigned char *)&size;
	buffer2[0] = puc[1];
	buffer2[1] = puc[0];
	return(fwrite(buffer2, 1, 2, fout));
}

Long write_dat_size(FILE *fout, Long size)
{
	unsigned char buffer4[4];
	unsigned char buffer2[2];
	unsigned char *puc;

	/* motorola format */
	puc = (unsigned char *)&size;
	buffer2[0] = puc[1];
	buffer2[1] = puc[0];
	hextoascii(buffer4, buffer2, 2);
	return(fwrite(buffer4, 1, 4, fout));
}

void dump_lif_dir(FILE *fin, char *inpath, Long count, char *name)
{
	unsigned char dir[sizeof(lifdir)];
	unsigned short *pus;
	char pname[11];
	Long entries;
	int i;

	/* get search name ( if any ) */
	get_lif_name(pname, name, NULL);

	entries = 0;
	while (count) {
		/* read 32-byte directory */
		--count;
		if (fread(dir, 1, sizeof(lifdir), fin) != sizeof(lifdir)) {
			printf("Error reading 32-byte directory from[ %s ]\n", inpath);
			count = 0;
		}
		else {
			/* get directory type */
			pus = (unsigned short *)&dir[10];

			/* empty directory? */
			if (*pus == 0xFFFF) {
				count = 0;
			}
			else if (*pname == '\0' || is_lif_name((char *)dir, name)) {
				/* list directory */
				++entries;
				if (entries == 1) {
					printf(" name        type  size  implementation\n");
					printf(" --------------------------------------\n");
				}
				printf(" ");
				for (i = 0; i < 10; ++i)
					printf("%c", dir[i]);
				printf("  ");
				for (i = 10; i < 12; ++i)
					printf("%02X", dir[i]);
				printf("  ");
				if (*pus == 0x80E0) {
					for (i = 28; i < 30; ++i)
						printf("%02X", dir[i]);
				}
				else
					printf("xxxx");
				printf("  ");
				for (i = 28; i < 32; ++i)
					printf("%02X", dir[i]);
				printf("\n");
			}
		}
	}

	if (entries == 0 && *pname != '\0') {
		printf("No match found for name[ %s ], in file [ %s ]\n", pname, inpath);
	}
	else if (entries) {
		printf("%d directory entries found.\n", entries);
	}
}

void show_lif_info(char *name, Long size)
{
	if (name)
		printf(" name[ %s ]\n", name);
	printf(" type[ E080 ]\n");
	printf(" size[ %04X ]  ( %d bytes )\n", (unsigned short)size, size);
	printf(" implementation[ %04X0020 ]\n", (unsigned short)size);
}

Long read_bin_size(FILE *fin, char *inpath, Long length)
{
	unsigned char buffer2[2];

	/* read 2-byte header */
	if (fread(buffer2, 1, 2, fin) != 2) {
		printf("Error reading 2-byte header from[ %s ]\n", inpath);
		return(0);
	}

	/* get program size */
	return(get_lif_size(buffer2, length));
}

Long read_dat_size(FILE *fin, char *inpath, Long length)
{
	unsigned char buffer4[4];
	unsigned char buffer2[2];
	int i;

	/* read 4-byte header */
	if (fread(buffer4, 1, 4, fin) != 4) {
		printf("Error reading 4-byte header from[ %s ]\n", inpath);
		return(0);
	}

	/* validate hex digits */
	if ((i = nonxdigit_buffer(buffer4, 4))) {
		printf("Error: invalid ASCII hex-digit in 4-byte header[ %02X ]\n",
			buffer4[i - 1]);
		return(0);
	}

	/* convert ASCII to hex */
	asciitohex(buffer2, buffer4, 4);


	/* get program size */
	return(get_lif_size(buffer2, length / 2) * 2);
}

Long read_p41_dir(FILE *fin, char *inpath, Long length, char *name)
{
	unsigned char dir[sizeof(lifdir)];
	unsigned short *pus;
	int i;

	/* init complete name with NULL */
	if (name) {
		memset(name, 0, 11);
	}

	/* read 32-byte directory */
	if (fread(dir, 1, sizeof(lifdir), fin) != sizeof(lifdir)) {
		printf("Error reading 32-byte directory from[ %s ]\n", inpath);
		return(0);
	}

	/* get program name */
	if (name) {
		i = 0;
		while (dir[i] != 0x20 && i < 10) {
			name[i] = dir[i];
			++i;
		}
	}

	/* check LIF type */
	if (*(pus = (unsigned short *)&dir[10]) != 0x80E0) {
		printf("Error: invalid file type in 32-byte header: expected[ E080 ], found[ %02X%02X ]\n",
			dir[10], dir[11]);
		return(0);
	}

	/* get program size */
	return(get_lif_size(&dir[28], length));
}

Long read_lif_dir(FILE *fin, char *inpath, Long *plength, char *name,
	Long dirblks)
{
	unsigned char *puc, *plif;
	unsigned short *pus;
	Long length, size;
	Long startblk;
	int i, match;

	match = 0;
	do {
		/* read 256-byte diretory block */
		if (fread(buf1_256, 1, 256, fin) != 256) {
			printf("Error reading directory block from[ %s ]\n", inpath);
			dirblks = 0;
		}
		else {
			plif = buf1_256;
			for (i = 0; i < 8 && dirblks && !match; ++i) {
				/* get file type */
				pus = (unsigned short *)&plif[10];

				/* empty directory? */
				if (*pus == 0xFFFF) {
					dirblks = 0;
				}
				/* HP-41 program type? */
				else if (*pus == 0x80E0) {
					/* search for matching name */
					if ((match = is_lif_name((char *)plif, name))) {
						/* get program start block (motorola format) */
						puc = (unsigned char *)&startblk;
						puc[0] = plif[15];
						puc[1] = plif[14];
						puc[2] = plif[13];
						puc[3] = plif[12];

						/* get program size (motorola format) */
						puc = (unsigned char *)&size;
						puc[3] = puc[2] = 0;
						puc[1] = plif[28];
						puc[0] = plif[29];

						if (size) {
							/* minimum file size */
							length = startblk * 256 + size;
							if (*plength < length) {
								printf("Error: input file too short: expected[ %d+ ], found[ %d ] bytes\n",
									length, *plength);
								size = 0;
							}
						}
						else {
							printf("Error: invalid program size[ 0000 ] for[ %s ]\n", name);
						}
					}
				}

				/* next 32-byte directory */
				plif += sizeof(lifdir);
			}

			/* next 256-byte directory block */
			if (dirblks)
				--dirblks;
		}
	} while (dirblks && !match);

	if (match) {
		if (size) {
			*plength = size;
			return(startblk);
		}
	}
	else {
		printf("No match found for: name[ %s ], file type[ E080 ]\n", name);
	}
	return(0);
}


Long read_lif_hdr(FILE *fin, char *inpath, Long size)
{
	unsigned char buffer4[4];
	unsigned char *puc;
	unsigned short *pus;
	Long *pul;
	Long dirblks = 0;
	int i;

	printf("sizeof(Long)=%i\n",(int)sizeof(Long));
	printf("sizeof(int)=%i\n",(int)sizeof(int));

	/* fill buffer with NULL for error checking */
	memset(buf2_256, 0, sizeof(buf2_256));

	if (fread(buf1_256, 1, 256, fin) != 256) {
		printf("Error reading LIF header from[ %s ]\n", inpath);
	}
	else if (size <= 768) {
		printf("Error: input file too short: expected[ 769+ ], found[ %d ]\n", size);
	}
	else if (*(pus = (unsigned short *)&buf1_256[0]) != 0x0080) {
		puc = (unsigned char *)pus;
		printf("Error: invalid LIF identifier at offset[ ");
		if (puc[0] != 0x80)
			printf("0 ]: expected[ 80 ], found[ %02X ]\n", puc[0]);
		else
			printf("1 ]: expected[ 00 ], found[ %02X ]\n", puc[1]);
	}
	else if (*(pul = (Long *)&buf1_256[8]) != 0x02000000) {
		puc = (unsigned char *)pul;
		printf("Error: invalid directory start block at offset[ ");
		if (puc[0] != 0x00)
			printf("8 ]: expected[ 00 ], found[ %02X ]\n", puc[0]);
		else if (puc[1] != 0x00)
			printf("9 ]: expected[ 00 ], found[ %02X ]\n", puc[1]);
		else if (puc[2] != 0x00)
			printf("10 ]: expected[ 00 ], found[ %02X ]\n", puc[2]);
		else
			printf("11 ]: expected[ 02 ], found[ %02X ]\n", puc[3]);
	}
	else if (*(pus = (unsigned short *)&buf1_256[12]) != 0x0010) {
		puc = (unsigned char *)pus;
		printf("Error: invalid machine identifier at offset[ ");
		if (puc[0] != 0x10)
			printf("12 ]: expected[ 10 ], found[ %02X ]\n", puc[0]);
		else
			printf("13 ]: expected[ 00 ], found[ %02X ]\n", puc[1]);
	}
	else if (*(pus = (unsigned short *)&buf1_256[14]) != 0x00) {
		puc = (unsigned char *)pus;
		printf("Error: invalid filler at offset[ ");
		if (puc[0] != 0x00)
			printf("14 ]: expected[ 00 ], found[ %02X ]\n", puc[0]);
		else if (puc[1] != 0x00)
			printf("15 ]: expected[ 00 ], found[ %02X ]\n", puc[1]);
	}
	else if (*(pus = (unsigned short *)&buf1_256[22]) != 0x00) {
		puc = (unsigned char *)pus;
		printf("Error: invalid filler at offset[ ");
		if (puc[0] != 0x00)
			printf("22 ]: expected[ 00 ], found[ %02X ]\n", puc[0]);
		else if (puc[1] != 0x00)
			printf("23 ]: expected[ 00 ], found[ %02X ]\n", puc[1]);
	}
	else if (memcmp(&buf1_256[42], buf2_256, 214) != 0x00) {
		for (i = 42; i < 256 && buf1_256[i] == 0; ++i);
		printf("Error: invalid filler at offset[ %d ]: expected[ 00 ], found[ %02X ]\n", i, buf1_256[i]);
	}
	else if ((dirblks = *(Long *)&buf1_256[16]) == 0x00) {
		printf("Error: LIF header has 0 directory blocks at offset[ 16..19 ]\n");
	}
	else if (fread(buf1_256, 1, 256, fin) != 256) {
		printf("Error reading LIF header from[ %s ]\n", inpath);
		dirblks = 0;
	}

	/* convert to intel format */
	puc = (unsigned char *)&dirblks;
	memcpy(buffer4, puc, 4);
	puc[0] = buffer4[3];
	puc[1] = buffer4[2];
	puc[2] = buffer4[1];
	puc[3] = buffer4[0];

	return(dirblks);
}

Long get_lif_size(unsigned char *buffer2, Long length)
{
	unsigned char *puc;
	Long size;

	/* get program size */
	puc = (unsigned char *)&size;
	size = 0L;
	puc[1] = buffer2[0];
	puc[0] = buffer2[1];
	if (size == 0 || size > length)
		size = length;

	return(size);
}

void get_lif_name(char *pname, char *name, char *filename)
{
	int i, j;

	j = 0;
	*pname = '\0';
	if (name && *name != '\0' && strlen(name) < 11) {
		for (i = 0; i < (int)strlen(name) && j == 0; ++i) {
			if (!isalnum(name[i]))
				++j;
		}
		if (j == 0)
			strcpy(pname, name);
	}
	if (filename) {
		if (*pname == '\0') {
			file_splitpath(filename, drive, dir, fname, ext);
			strcpy(pname, fname);
			_strupr(pname);
		}
		if (j) {
			printf("Warning: specified name[ %s ] ignored, using[ %s ] instead\n",
				name, pname);
		}
	}
}

int is_lif_name(char *lifname, char *name)
{
	int i, j;

	/* search for matching name */
	for (i = 0, j = 0; i < (int)strlen(name) && !j; ++i) {
		if (lifname[i] == 0x20 || name[i] != lifname[i])
			++j;
	}
	return(j == 0 && (i == 10 || lifname[i] == 0x20));
}

Long write_lif_hdr(FILE *fout, Long *pstartblk, Long *pdirpos, int files)
{
	Long length, startblk;
	int blk, i;

	/* init header ( padded with 0x00 ) */
	memset(buf1_256, 0, sizeof(buf1_256));
	memcpy(buf1_256, lifhdr, sizeof(lifhdr));

	/* directory length in blocks (motorola format): header[19..16]  */
	blk = files * 32 / 256;
	if (blk * 256 < files * 32)
		++blk;
	/* max of 255 blocks, at 8 files per block = 2040 files */
	buf1_256[19] = (unsigned char)blk;

	/* write header at block[0], and filler (0) at block[1] */
	length = fwrite(buf1_256, 1, sizeof(buf1_256), fout);
	memset(buf1_256, 0, sizeof(lifhdr));
	length += fwrite(buf1_256, 1, sizeof(buf1_256), fout);
	*pdirpos = length;

	/* write empty directory blocks ( filled with 0xFF ) */
	startblk = 2;
	memset(buf1_256, 0xFF, sizeof(buf1_256));
	for (i = 0; i < blk; ++i) {
		length += fwrite(buf1_256, 1, sizeof(buf1_256), fout);
		++startblk;
	}

	*pstartblk = startblk;
	return(length);
}

Long write_lif_dir(FILE *fout, char *name, Long startblk, Long size)
{
	unsigned char dir[sizeof(lifdir)];
	unsigned char *puc;
	Long blk;
	int i;

	/* init directory */
	memcpy(dir, lifdir, sizeof(lifdir));

	/* program name */
	for (i = 0; i < (int)strlen(name); ++i)
		dir[i] = name[i];

	/* start block ( motorola format ) */
	puc = (unsigned char *)&startblk;
	dir[12] = puc[3];
	dir[13] = puc[2];
	dir[14] = puc[1];
	dir[15] = puc[0];

	/* program length in blocks ( motorola format ) */
	blk = (size + 1) / 256;
	if (blk * 256 < (size + 1))
		++blk;
	puc = (unsigned char *)&blk;
	dir[18] = puc[1];
	dir[19] = puc[0];

	/* program size ( motorola format ) */
	puc = (unsigned char *)&size;
	dir[28] = puc[1];
	dir[29] = puc[0];

	/* write directory */
	return(fwrite(dir, 1, sizeof(lifdir), fout));
}

int nonxdigit_buffer(unsigned char *buffer, int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		if (!isxdigit(buffer[i]))
			return(i + 1);
	}

	return(0);
}

