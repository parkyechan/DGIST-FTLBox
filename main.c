#include "ftl_box.h"

/*
 * Number of blocks: 151552
 * Number of pages per block: 128
 * Number of pages: 19398656 (=151552*128) (Phyiscal key space)
 * Page size: 4 Byte (integer)
 * Key range: 0 ~ 16777215 (Logical key space)
 */

/*
 * Argument to main: trace file (trace1.bin, trace2.bin, trace3.bin
 * trace: A large sequence of 1-byte type('r' or 'w'), 4-byte key, 4-byte value
 * You can input your arbitrary trace
 */

/*
 * Trace file format
 * File size: 1.3GB (total 3 files)
 * Total cycle: 11 times (sequential init + repeat 10 times)
 * No duplicate values (Not important)
 */

/*
 * trace1.bin: sequential key
 * cycle 1: write 0 ~ 16,777,215 keys sequentially
 * cycle 2: read/write(5:5) 0 ~ 16,777,215 keys sequentailly
 * repeat cycle 2 10 times
 */

/*
 * trace2.bin: random key
 * cycle 1: write 0 ~ 16,777,215 keys sequentially
 * cycle 2: read/write(5:5) 0 ~ 16,777,215 keys randomly
 * repeat cycle 2 10 times
 */

/*
 * trace3.bin: random & skewed key
 * cycle 1: write 0 ~ 16,777,215 keys sequentially
 * cycle 2: read/write(5:5) 0 ~ 16,777,215 keys randomly (20% of keys occupy 80%)
 * repeat cycle 2 10 times
 */

/*
 * Implement your own FTL (Page FTL or Anything else)
 * This main function starts FTLBox and reads traces
 */

int block_num = 0, page_num = 0;

typedef struct table {
	int32_t pbn;
	int32_t ppn;
	int32_t past_num;
} _table;

_table logical_table[NUMKEY];
int key_table[NUMKEY];
int OOB[19398656] = {};
int cycle = 0;
void page_write(int trace_key, int trace_value){


	if(key_table[trace_key] == -1){ // cycle 1 일 경우에
		if(OOB[block_num * 128 + page_num] == -1){
			logical_table[trace_key].pbn = block_num;
			logical_table[trace_key].ppn = page_num;
			printf("Cycle 1 %d %d\n", block_num, page_num);
			OOB[block_num * 128 + page_num] = 0;
			flash_page_write(logical_table[trace_key].pbn, logical_table[trace_key].ppn, trace_value);
		}
		key_table[trace_key] = 0;
	}else{
		if(OOB[block_num * 128 + page_num] == -1){
			OOB[logical_table[trace_key].pbn * 128 + logical_table[trace_key].ppn] = -1;
			OOB[block_num * 128 + page_num] = 0;
			logical_table[trace_key].pbn = block_num;
			logical_table[trace_key].ppn = page_num;
			flash_page_write(logical_table[trace_key].pbn, logical_table[trace_key].ppn, trace_value);
	  }
	}
	page_num++;
	if(page_num > 127){
		page_num = 0;
		block_num += 1;
	}
	if(block_num > 151551){
		block_num = 0;
		cycle++;
	}
}
int main(int argc, char *argv[])
{
	for(int i=0; i<NUMKEY; i++){
		key_table[i] = -1;
	}
	for(int i=0; i<19398656; i++){
			OOB[i] = -1;
	}
	FILE * fp;
	char *buf;
	int trace_key, trace_value;

	if (argc > 2 || argc <= 1) {
		puts("argument error");
		abort();
	}
	if (!fopen(argv[1], "rt")) {
		puts("trace open error");
		abort();
	}
	printf("Input Trace File : %s\n", (argv[1]));

	fp = fopen(argv[1], "rb");
	buf = (char*)malloc(sizeof(char) * 1);

	box_create();
	/* Read Trace file */
	while (1) {
		if (!fread((void*)buf, 1, 1, fp)) {
			if (feof(fp)) {
				puts("trace eof");
				break;
			}
			puts("fread error");
			abort();
		}

		if (*buf == 'w') {
			fread((void*)&trace_key, 4, 1, fp);
			fread((void*)&trace_value, 4, 1, fp);

			/* Trace check */
			// printf("type: %c, key: %d, value: %d\n", *buf, trace_key, trace_value);

			/* Implement your page_write function */
			page_write(trace_key, trace_value);
		}
		else if (*buf == 'r') {
			fread((void*)&trace_key, 4, 1, fp);

			/* Trace check */
			// printf("type: %c, key: %d, value: %d\n", *buf, trace_key, trace_value);

			/* Implement your page_read function */
			//page_read(trace_key, trace_value);
		}
		else {
			puts("trace error");
			abort();
		}
	}

	puts("ftl_box end");
	fclose(fp);
	box_destroy();

	return 0;
}
