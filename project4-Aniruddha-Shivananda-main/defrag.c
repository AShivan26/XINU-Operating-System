#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#define INODESIZE sizeof(inode)
#define OUTPUT_FILE_NAME "disk_defrag"
#define N_DBLOCKS 10
#define N_IBLOCKS 4

typedef struct superblock
{
    int blocksize;    /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in blocks */
    int data_offset;  /* data region offset in blocks */
    int swap_offset;  /* swap region offset in blocks */
    int free_inode;   /* head of free inode list */
    int free_block;   /* head of free block list */
} superblock;

typedef struct inode
{
    int next_inode;         /* list for free inodes */
    int protect;            /* protection field */
    int nlink;              /* number of links to this file */
    int size;               /* number of bytes in file */
    int uid;                /* owner's user ID */
    int gid;                /* owner's group ID */
    int ctime;              /* creation time */
    int mtime;              /* modification time */
    int atime;              /* access time */
    int dblocks[N_DBLOCKS]; /* pointers to data blocks */
    int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
    int i2block;            /* pointer to doubly indirect block */
    int i3block;            /* pointer to triply indirect block */
} inode;

int i, j, k, t, d, s, n, l, m;

// Function prototypes
char *read_file(const char *filename, off_t *file_size);
int write_output_file(char *new_buffer, off_t file_size);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    off_t file_size;
    char *buffer = read_file(argv[1], &file_size);
    if (!buffer)
        return EXIT_FAILURE;

    char *new_buffer = (char *)malloc(file_size);
    if (!new_buffer)
    {
        perror("Memory allocation failed for new buffer");
        free(buffer);
        return EXIT_FAILURE;
    }

    memset(new_buffer, 0, file_size);

    memcpy(new_buffer, buffer, 512);

    superblock *superblock_ptr = (superblock *)&(buffer[512]);
#define BLOCK_SIZE superblock_ptr->blocksize
#define INODE_OFFSET superblock_ptr->inode_offset *BLOCK_SIZE
#define DATA_OFFSET superblock_ptr->data_offset *BLOCK_SIZE

    if (INODE_OFFSET >= DATA_OFFSET || INODE_OFFSET >= superblock_ptr->swap_offset || DATA_OFFSET >= superblock_ptr->swap_offset)
    {
        perror("Super block is corrupted.\n");
        free(buffer);
        free(new_buffer);
        return EXIT_FAILURE;
    }
    int total_inodes = (DATA_OFFSET - INODE_OFFSET) / INODESIZE;
    int current_data_index = 0;
    int current_data_address = 1024 + DATA_OFFSET;
    int inode_address = 1024 + INODE_OFFSET;

    int itr = total_inodes;
    for (itr = total_inodes; itr > 0; itr--)
    {
        inode *current_inode = (inode *)&buffer[inode_address];

        if (current_inode->nlink != -1)
        {
            inode temp_inode = *current_inode;
            memset(temp_inode.dblocks, -1, sizeof(temp_inode.dblocks));
            memset(temp_inode.iblocks, -1, sizeof(temp_inode.iblocks));
            temp_inode.i2block = -1;
            temp_inode.i3block = -1;

            // Map Direct Blocks to new buffer
            for (i = 0; i < N_DBLOCKS; i++)
            {
                if (current_inode->dblocks[i] != -1)
                {
                    int old_data_block_location = (current_inode->dblocks[i] * BLOCK_SIZE) + 1024 + DATA_OFFSET;
                    memcpy(new_buffer + current_data_address, buffer + old_data_block_location, BLOCK_SIZE);
                    temp_inode.dblocks[i] = current_data_index++;
                    current_data_address += BLOCK_SIZE;
                }
            }

            // Level 1 mapping for Indirect Blocks
            for (i = 0; i < N_IBLOCKS; i++)
            {
                if (current_inode->iblocks[i] != -1)
                {
                    int pointers_per_block = BLOCK_SIZE / sizeof(int);
                    int *new_indirect_pointer_array = (int *)(new_buffer + current_data_address);

                    for (j = 0; j < pointers_per_block; j++)
                    {
                        new_indirect_pointer_array[j] = -1;
                    }

                    temp_inode.iblocks[i] = current_data_index++;
                    current_data_address += BLOCK_SIZE;

                    int *old_indirect_pointer_array = (int *)(buffer + 1024 + DATA_OFFSET + (current_inode->iblocks[i] * BLOCK_SIZE));

                    for (j = 0; j < pointers_per_block; j++)
                    {
                        if (old_indirect_pointer_array[j] != -1)
                        {
                            memcpy(new_buffer + current_data_address, buffer + 1024 + DATA_OFFSET + (old_indirect_pointer_array[j] * BLOCK_SIZE), BLOCK_SIZE);
                            new_indirect_pointer_array[j] = current_data_index++;
                            current_data_address += BLOCK_SIZE;
                        }
                    }
                }
            }

            // Logic for level2  Double Indirect Blocks
            if (current_inode->i2block != -1)
            {
                int pointers_per_block = BLOCK_SIZE / sizeof(int);
                int *old_double_indirect_pointer_array = (int *)(buffer + 1024 + DATA_OFFSET + (current_inode->i2block * BLOCK_SIZE));
                int *new_double_indirect_pointer_array = (int *)(new_buffer + current_data_address);

                for (k = 0; k < pointers_per_block; k++)
                {
                    new_double_indirect_pointer_array[k] = -1;
                }

                temp_inode.i2block = current_data_index++;
                current_data_address += BLOCK_SIZE;

                for (m = 0; m < pointers_per_block; m++)
                {
                    if (old_double_indirect_pointer_array[m] != -1)
                    {
                        int *new_indirect_pointer_array_2nd_level = (int *)(new_buffer + current_data_address);

                        for (n = 0; n < pointers_per_block; n++)
                        {
                            new_indirect_pointer_array_2nd_level[n] = -1;
                        }

                        new_double_indirect_pointer_array[m] = current_data_index++;
                        current_data_address += BLOCK_SIZE;

                        int *old_indirect_pointer_array_2nd_level = (int *)(buffer + 1024 + DATA_OFFSET + old_double_indirect_pointer_array[m] * BLOCK_SIZE);

                        for (l = 0; l < pointers_per_block; l++)
                        {
                            if (old_indirect_pointer_array_2nd_level[l] != -1)
                            {
                                memcpy(new_buffer + current_data_address, buffer + 1024 + DATA_OFFSET + old_indirect_pointer_array_2nd_level[l] * BLOCK_SIZE, BLOCK_SIZE);
                                new_indirect_pointer_array_2nd_level[l] = current_data_index++;
                                current_data_address += BLOCK_SIZE;
                            }
                        }
                    }
                }
            }

            // Handle Triple Indirect Blocks
            if (current_inode->i3block != -1)
            {
                int pointers_per_block = BLOCK_SIZE / sizeof(int);
                int *old_triple_indirect_pointer_array = (int *)(buffer + 1024 + DATA_OFFSET + current_inode->i3block * BLOCK_SIZE);
                int *new_triple_indirect_pointer_array = (int *)(new_buffer + current_data_address);

                for (t = 0; t < pointers_per_block; t++)
                {
                    new_triple_indirect_pointer_array[t] = -1;
                }

                temp_inode.i3block = current_data_index++;
                current_data_address += BLOCK_SIZE;

                for (t = 0; t < pointers_per_block; t++)
                {
                    if (old_triple_indirect_pointer_array[t] != -1)
                    {
                        int *old_double_indirect_pointers_arr = (int *)(buffer + 1024 + DATA_OFFSET + old_triple_indirect_pointer_array[t] * BLOCK_SIZE);
                        int *new_double_indirect_pointers_arr = (int *)(new_buffer + current_data_address);

                        for (d = 0; d < pointers_per_block; d++)
                        {
                            new_double_indirect_pointers_arr[d] = -1;
                        }

                        new_triple_indirect_pointer_array[t] = current_data_index++;
                        current_data_address += BLOCK_SIZE;

                        for (d = 0; d < pointers_per_block; d++)
                        {
                            if (old_double_indirect_pointers_arr[d] != -1)
                            {
                                int *old_single_indirect_pointers_arr = (int *)(buffer + 1024 + DATA_OFFSET + old_double_indirect_pointers_arr[d] * BLOCK_SIZE);
                                int *new_single_indirect_pointers_arr = (int *)(new_buffer + current_data_address);

                                for (s = 0; s < pointers_per_block; s++)
                                {
                                    new_single_indirect_pointers_arr[s] = -1;
                                }

                                new_double_indirect_pointers_arr[d] = current_data_index++;
                                current_data_address += BLOCK_SIZE;

                                for (s = 0; s < pointers_per_block; s++)
                                {
                                    if (old_single_indirect_pointers_arr[s] != -1)
                                    {
                                        memcpy(new_buffer + current_data_address, buffer + 1024 + DATA_OFFSET + old_single_indirect_pointers_arr[s] * BLOCK_SIZE, BLOCK_SIZE);
                                        new_single_indirect_pointers_arr[s] = current_data_index++;
                                        current_data_address += BLOCK_SIZE;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            memcpy(new_buffer + inode_address, &temp_inode, INODESIZE);
        }
        else
        {
            memcpy(new_buffer + inode_address, buffer + inode_address, INODESIZE);
        }
        inode_address += INODESIZE;
    }

    // Calculate and write free blocks
    int free_blocks_count = superblock_ptr->swap_offset - (current_data_index + superblock_ptr->data_offset);
    int next_free_block = current_data_index + 1;

    for (i = 0; i < free_blocks_count; i++)
    {
        int free_block_location = current_data_address + (i * BLOCK_SIZE);

        if (i == free_blocks_count - 1)
        {
            int end_marker = -1;
            memcpy(new_buffer + free_block_location, &end_marker, sizeof(end_marker));
        }
        else
        {
            memcpy(new_buffer + free_block_location, &next_free_block, sizeof(next_free_block));
            next_free_block++;
        }

        memset(new_buffer + free_block_location + sizeof(next_free_block), 0, BLOCK_SIZE - sizeof(next_free_block));
    }

    memcpy(new_buffer, buffer, 1024);

    superblock *new_superblock_ptr = (superblock *)&(new_buffer[512]);
    new_superblock_ptr->free_block = current_data_index;

    int swap_start = BLOCK_SIZE * superblock_ptr->swap_offset + 1024;
    int swap_size = file_size - superblock_ptr->swap_offset + 1024;

    swap_size = (swap_size + swap_start < file_size) ? swap_size : file_size - swap_start;
    memcpy(new_buffer + swap_start, buffer + swap_start, swap_size);

    if (write_output_file(new_buffer, file_size) != 0)
    {
        free(buffer);
        free(new_buffer);
        return EXIT_FAILURE;
    }

    free(buffer);
    free(new_buffer);

    return 0;
}

char *read_file(const char *filename, off_t *file_size)
{
    FILE *in = fopen(filename, "r");
    if (in == NULL)
    {
        perror("Failed to open input file");
        return NULL;
    }

    fseek(in, 0, SEEK_END);
    *file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (*file_size == -1)
    {
        perror("File size is -1");
        fclose(in);
        return NULL;
    }

    char *buffer = (char *)malloc(*file_size);
    if (!buffer)
    {
        perror("Failed to allocate memory for buffer");
        fclose(in);
        return NULL;
    }

    if (fread(buffer, 1, *file_size, in) != *file_size)
    {
        fprintf(stderr, "Error: Incomplete read\n");
        free(buffer);
        fclose(in);
        return NULL;
    }

    fclose(in);
    return buffer;
}

int write_output_file(char *new_buffer, off_t file_size)
{
    FILE *out = fopen(OUTPUT_FILE_NAME, "w+");
    if (out == NULL)
    {
        perror("Error opening output file");
        return -1;
    }

    if (fwrite(new_buffer, 1, file_size, out) != file_size)
    {
        perror("Error writing to output file");
        fclose(out);
        return -1;
    }

    fclose(out);
    return 0;
}