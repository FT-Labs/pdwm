/* Please write your blocks same as below if you are not using configuration file.
*  All strings needed to be allocated on heap, therefore use the same syntax as below, or for dynamic changing
*  just use the configuration file.
*/
#define BLOCK_SIZE 7 /* Change this block size according to your bottom blocks */
blocks = malloc(BLOCK_SIZE * sizeof(Block));
statusbar = calloc(BLOCK_SIZE, sizeof(*statusbar));
					/*Command*/		/*Update Interval*/	/*Update Signal*/
blocks[0] = (Block){strdup("cat /tmp/recordingicon 2>/dev/null"),	0,	9};
blocks[1] = (Block){strdup("crypto"),	360,	6};
blocks[2] = (Block){strdup("weather"),	18000,	5};
blocks[3] = (Block){strdup("volume"),	0,	10};
blocks[4] = (Block){strdup("battery"),	5,	3};
blocks[5] = (Block){strdup("clock"),	60,	1};
blocks[6] = (Block){strdup("internet"),	5,	4};

